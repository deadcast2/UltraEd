#include <thread>
#include "Build.h"
#include "Debug.h"
#include "Scene.h"
#include "FileIO.h"
#include "Util.h"
#include "BoxCollider.h"
#include "SphereCollider.h"

namespace UltraEd
{
    Scene::Scene(HWND hWnd, Gui *gui) :
        m_version(APP_SCENE_VERSION),
        m_hWnd(hWnd),
        m_defaultMaterial(),
        m_fillMode(D3DFILLMODE::D3DFILL_SOLID),
        m_gizmo(),
        m_views(),
        m_actors(),
        m_grid(),
        m_selectedActorIds(),
        m_mouseSmoothX(0),
        m_mouseSmoothY(0),
        m_activeViewType(ViewType::Perspective),
        m_sceneName(),
        m_backgroundColorRGB({ 0, 0, 0 }),
        m_auditor(this),
        m_gui(gui),
        m_renderDevice(800, 600),
        m_path()
    {
        m_defaultMaterial.Diffuse.r = m_defaultMaterial.Ambient.r = 1.0f;
        m_defaultMaterial.Diffuse.g = m_defaultMaterial.Ambient.g = 1.0f;
        m_defaultMaterial.Diffuse.b = m_defaultMaterial.Ambient.b = 1.0f;
        m_defaultMaterial.Diffuse.a = m_defaultMaterial.Ambient.a = 1.0f;

        New();
    }

    Scene::~Scene()
    {
        Release();
    }

    void Scene::New()
    {
        SetTitle("Untitled");
        UnselectAll();
        Release();
        ResetViews();
        m_actors.clear();
        m_auditor.Reset();
        m_gizmo.SetSnapSize(0.5f);
        m_backgroundColorRGB = { 0, 0, 0 };
        m_path.clear();
        SetDirty(false);
    }

    bool Scene::SaveAs()
    {
        return HasPath() && SaveAs(m_path);
    }

    bool Scene::SaveAs(const std::filesystem::path &path)
    {
        if (FileIO::Save(this, path))
        {
            SetTitle(path.stem().string());
            SetDirty(false);
            m_path = path;

            return true;
        }

        return false;
    }

    void Scene::Load(const std::filesystem::path &path)
    {
        std::shared_ptr<nlohmann::json> root;

        if (FileIO::Load(root, path))
        {
            New();
            SetTitle(path.stem().string());
            Load(*root.get());
            m_path = path;
        }
    }

    void Scene::BuildROM(BuildFlag flag)
    {
        std::thread run([this, flag]() {
            if (Build::Start(this))
            {
                if (static_cast<int>(flag) &static_cast<int>(BuildFlag::Run))
                {
                    Build::Run();
                }
                else if (static_cast<int>(flag) &static_cast<int>(BuildFlag::Load))
                {
                    Build::Load();
                }
            }
            else
            {
                Debug::Instance().Error("The ROM build has failed. Make sure the build tools have been installed.");
            }
        });
        run.detach();
    }

    void Scene::AddModel(const boost::uuids::uuid &assetId)
    {
        std::shared_ptr<Model> model = NULL;
        model = std::make_shared<Model>(assetId);
        m_actors[model->GetId()] = model;
        model->SetName(std::string("Actor ").append(std::to_string(m_actors.size())));
        m_auditor.AddActor("Model", model->GetId());

        if (model != NULL)
        {
            SelectActorById(model->GetId());
        }
    }

    void Scene::AddCamera()
    {
        auto newCamera = std::make_shared<Camera>();
        m_actors[newCamera->GetId()] = newCamera;
        newCamera->SetName(std::string("Camera ").append(std::to_string(m_actors.size())));
        m_auditor.AddActor("Camera", newCamera->GetId());

        SelectActorById(newCamera->GetId());
    }

    void Scene::AddCollider(ColliderType type)
    {
        if (m_selectedActorIds.empty())
        {
            Debug::Instance().Warning("An object must be selected first.");
        }

        const auto groupId = Util::NewUuid();

        for (const auto &selectedActorId : m_selectedActorIds)
        {
            m_auditor.ChangeActor("Add Collider", selectedActorId, groupId);

            if (type == ColliderType::Box)
            {
                m_actors[selectedActorId]->SetCollider(new BoxCollider(m_actors[selectedActorId]->GetVertices()));
            }
            else
            {
                m_actors[selectedActorId]->SetCollider(new SphereCollider(m_actors[selectedActorId]->GetVertices()));
            }
        }
    }

    void Scene::DeleteCollider()
    {
        if (m_selectedActorIds.empty())
        {
            Debug::Instance().Warning("An object must be selected first.");
        }

        const auto groupId = Util::NewUuid();

        for (const auto &selectedActorId : m_selectedActorIds)
        {
            m_auditor.ChangeActor("Delete Collider", selectedActorId, groupId);

            m_actors[selectedActorId]->SetCollider(NULL);
        }
    }

    void Scene::AddTexture(const boost::uuids::uuid &assetId)
    {
        if (m_selectedActorIds.empty())
        {
            Debug::Instance().Warning("An actor must be selected first.");
            return;
        }

        const auto groupId = Util::NewUuid();

        for (const auto &selectedActorId : m_selectedActorIds)
        {
            if (m_actors[selectedActorId]->GetType() != ActorType::Model) continue;

            m_auditor.ChangeActor("Add Texture", selectedActorId, groupId);

            if (!dynamic_cast<Model *>(m_actors[selectedActorId].get())
                ->SetTexture(m_renderDevice.GetDevice(), assetId))
            {
                Debug::Instance().Warning("Texture could not be loaded.");
            }
        }
    }

    void Scene::DeleteTexture()
    {
        if (m_selectedActorIds.empty())
        {
            Debug::Instance().Warning("An actor must be selected first.");
            return;
        }

        const auto groupId = Util::NewUuid();

        for (const auto &selectedActorId : m_selectedActorIds)
        {
            if (m_actors[selectedActorId]->GetType() != ActorType::Model) continue;

            m_auditor.ChangeActor("Delete Texture", selectedActorId, groupId);

            reinterpret_cast<Model *>(m_actors[selectedActorId].get())->GetTexture()->Delete();
        }
    }

    bool Scene::Pick(ImVec2 mousePoint, bool ignoreGizmo, Actor **selectedActor)
    {
        D3DXVECTOR3 orig, dir;
        ScreenRaycast(mousePoint, &orig, &dir);
        const bool gizmoSelected = m_gizmo.Select(orig, dir);
        float closestDist = FLT_MAX;

        if (!ignoreGizmo && gizmoSelected && !m_selectedActorIds.empty())
            return false;

        // Check all actors to see which poly might have been picked.
        for (const auto &actor : m_actors)
        {
            // Only choose the closest actors to the view.
            float pickDist = 0;
            if (actor.second->Pick(orig, dir, &pickDist) && pickDist < closestDist)
            {
                closestDist = pickDist;

                SelectActorById(actor.first, !m_gui->IO().KeyShift);

                if (selectedActor != NULL)
                    *selectedActor = actor.second.get();
            }
        }

        if (closestDist != FLT_MAX)
            return true;

        UnselectAll();

        return false;
    }

    void Scene::Resize(UINT width, UINT height)
    {
        const auto params = m_renderDevice.GetParameters();
        if (m_renderDevice.IsLost() || params->BackBufferWidth != width || params->BackBufferHeight != height)
        {
            m_renderDevice.Resize(width, height);
            UpdateViewMatrix();
        }
    }

    void Scene::Refresh(const std::vector<boost::uuids::uuid> &changedAssetIds)
    {
        for (const auto &changedAssetId : changedAssetIds)
        {
            for (const auto &actor : m_actors)
            {
                const auto model = std::static_pointer_cast<Model>(actor.second);
                if (actor.second->GetType() == ActorType::Model && model != nullptr)
                {
                    if (model->GetModelId() == changedAssetId)
                        model->SetMesh(changedAssetId);

                    if (model->GetTexture()->GetId() == changedAssetId)
                        model->SetTexture(m_renderDevice.GetDevice(), changedAssetId);
                }
            }
        }
    }

    void Scene::UpdateViewMatrix()
    {
        D3DXMATRIX viewMat;
        const float aspect = static_cast<float>(m_renderDevice.GetParameters()->BackBufferWidth) /
            static_cast<float>(m_renderDevice.GetParameters()->BackBufferHeight);

        if (GetActiveView()->GetType() == ViewType::Perspective)
        {
            const float fov = D3DX_PI / 2.0f;
            D3DXMatrixPerspectiveFovLH(&viewMat, fov, aspect, 0.1f, 1000.0f);
        }
        else
        {
            const float zoom = GetActiveView()->GetZoom();
            D3DXMatrixOrthoLH(&viewMat, zoom * aspect, zoom, -1000.0f, 1000.0f);
        }

        m_renderDevice.GetDevice()->SetTransform(D3DTS_PROJECTION, &viewMat);
    }

    void Scene::UpdateInput(const ImVec2 &mousePos)
    {
        View *view = GetActiveView();
        static bool prevGizmo = false;
        static auto groupId = Util::NewUuid();

        const float deltaTime = m_gui->IO().DeltaTime;
        const float smoothingModifier = 20.0f;
        const float mouseSpeedModifier = 0.55f;

        Actor *selectedActor = NULL;
        if (m_gui->IO().MouseReleased[1] &&
            m_gui->IO().MouseDownDurationPrev[1] < 0.2f &&
            Pick(mousePos, true, &selectedActor))
        {
            m_gui->OpenContextMenu(selectedActor);
        }

        if (m_gui->IO().MouseClicked[0]) Pick(mousePos);
        if (m_gui->IO().KeyCtrl && ImGui::IsKeyPressed('A', false)) SelectAll();
        if (m_gui->IO().KeyCtrl && ImGui::IsKeyPressed('D', false)) Duplicate();
        if (m_gui->IO().KeyCtrl && ImGui::IsKeyPressed('Z', false)) m_auditor.Undo();
        if (m_gui->IO().KeyCtrl && ImGui::IsKeyPressed('Y', false)) m_auditor.Redo();
        if (ImGui::IsKeyPressed(VK_DELETE, false)) Delete();
        if (ImGui::IsKeyPressed('F', false)) FocusSelected();
        if (ImGui::IsKeyPressed('H', false)) ResetViews();
        if (ImGui::IsKeyPressed(0x31, false)) m_gizmo.SetModifier(GizmoModifierState::Translate);
        if (ImGui::IsKeyPressed(0x32, false)) m_gizmo.SetModifier(GizmoModifierState::Rotate);
        if (ImGui::IsKeyPressed(0x33, false)) m_gizmo.SetModifier(GizmoModifierState::Scale);

        if (m_gui->IO().MouseDown[0] && !m_selectedActorIds.empty())
        {
            D3DXVECTOR3 rayOrigin, rayDir;
            ScreenRaycast(mousePos, &rayOrigin, &rayDir);
            if (prevGizmo || (prevGizmo = m_gizmo.Select(rayOrigin, rayDir)))
            {
                const auto lastSelectedActorId = m_selectedActorIds.back();
                for (const auto &selectedActorId : m_selectedActorIds)
                {
                    auto action = m_auditor.PotentialChangeActor(m_gizmo.GetModifierName(), selectedActorId, groupId);

                    if (m_gizmo.Update(GetActiveView(), rayOrigin, rayDir, m_actors[selectedActorId].get(),
                        m_actors[lastSelectedActorId].get()))
                    {
                        action();
                    }
                }
            }
        }
        else if (m_gui->IO().MouseDown[1] && m_activeViewType == ViewType::Perspective)
        {
            if (ImGui::IsKeyDown('W')) view->Walk(5.0f * deltaTime);
            if (ImGui::IsKeyDown('S')) view->Walk(-5.0f * deltaTime);
            if (ImGui::IsKeyDown('A')) view->Strafe(-5.0f * deltaTime);
            if (ImGui::IsKeyDown('D')) view->Strafe(5.0f * deltaTime);

            m_mouseSmoothX = Util::Lerp(deltaTime * smoothingModifier, m_mouseSmoothX, m_gui->IO().MouseDelta.x);
            m_mouseSmoothY = Util::Lerp(deltaTime * smoothingModifier, m_mouseSmoothY, m_gui->IO().MouseDelta.y);

            view->Yaw(m_mouseSmoothX * mouseSpeedModifier * deltaTime);
            view->Pitch(m_mouseSmoothY * mouseSpeedModifier * deltaTime);
            WrapCursor();
        }
        else if (m_gui->IO().MouseDown[2])
        {
            m_mouseSmoothX = Util::Lerp(deltaTime * smoothingModifier, m_mouseSmoothX, -m_gui->IO().MouseDelta.x);
            m_mouseSmoothY = Util::Lerp(deltaTime * smoothingModifier, m_mouseSmoothY, m_gui->IO().MouseDelta.y);

            view->Strafe(m_mouseSmoothX * deltaTime);
            view->Fly(m_mouseSmoothY * deltaTime);
            WrapCursor();
        }
        else if (m_gui->IO().MouseWheel != 0)
        {
            view->SingleStep(m_gui->IO().MouseWheel * 150);
            if (view->GetType() != ViewType::Perspective)
                UpdateViewMatrix();
        }
        else
        {
            // Reset smoothing values for new mouse view movement.
            m_mouseSmoothX = m_mouseSmoothY = 0;
            prevGizmo = false;
            groupId = Util::NewUuid();
            m_gizmo.Reset();
        }
    }

    void Scene::Render(LPDIRECT3DDEVICE9 target, LPDIRECT3DTEXTURE9 *texture)
    {
        CheckChanges();

        ID3DXMatrixStack *stack;
        if (!SUCCEEDED(D3DXCreateMatrixStack(0, &stack))) return;
        stack->LoadMatrix(&GetActiveView()->GetViewMatrix());

        auto device = m_renderDevice.GetDevice();
        device->SetTransform(D3DTS_WORLD, stack->GetTop());
        device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
            D3DCOLOR_XRGB(m_backgroundColorRGB[0], m_backgroundColorRGB[1], m_backgroundColorRGB[2]), 1.0f, 0);

        if (SUCCEEDED(device->BeginScene()))
        {
            m_grid.Render(device);

            // Render all actors with selected fill mode.
            device->SetMaterial(&m_defaultMaterial);
            device->SetRenderState(D3DRS_ZENABLE, TRUE);
            device->SetRenderState(D3DRS_FILLMODE, m_fillMode);
            device->SetRenderState(D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_COLOR1);

            for (const auto &actor : m_actors)
            {
                // Highlight any selected actors.
                device->SetRenderState(D3DRS_AMBIENT, IsActorSelected(actor.first) ? 0x0000ff00 : 0xffffffff);
                actor.second->Render(device, stack);
            }

            if (!m_selectedActorIds.empty())
            {
                // Draw the gizmo on "top" of all objects in scene.
                device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
                device->SetRenderState(D3DRS_ZENABLE, FALSE);
                m_gizmo.Render(device, stack, GetActiveView());
            }

            device->EndScene();

            auto deviceParameters = m_renderDevice.GetParameters();
            Util::CopyBackBuffer(deviceParameters->BackBufferWidth, deviceParameters->BackBufferHeight,
                m_renderDevice.GetDevice(), target, texture);
        }

        stack->Release();
    }

    void Scene::CheckChanges()
    {
        SetDirty(this->IsDirty());

        for (const auto &actor : m_actors)
        {
            if (actor.second->IsDirty())
            {
                SetDirty(true);
                break;
            }
        }
    }

    void Scene::ScreenRaycast(ImVec2 screenPoint, D3DXVECTOR3 *origin, D3DXVECTOR3 *dir)
    {
        auto device = m_renderDevice.GetDevice();
        View *view = GetActiveView();

        D3DVIEWPORT9 viewport;
        device->GetViewport(&viewport);

        D3DXMATRIX matProj;
        device->GetTransform(D3DTS_PROJECTION, &matProj);

        D3DXMATRIX matWorld;
        D3DXMatrixIdentity(&matWorld);

        D3DXVECTOR3 v1;
        D3DXVECTOR3 start = D3DXVECTOR3(screenPoint.x, screenPoint.y, 0.0f);
        D3DXVec3Unproject(&v1, &start, &viewport, &matProj, &view->GetViewMatrix(), &matWorld);

        D3DXVECTOR3 v2;
        D3DXVECTOR3 end = D3DXVECTOR3(screenPoint.x, screenPoint.y, 1.0f);
        D3DXVec3Unproject(&v2, &end, &viewport, &matProj, &view->GetViewMatrix(), &matWorld);

        *origin = v1;
        D3DXVec3Normalize(dir, &(v2 - v1));
    }

    View *Scene::GetActiveView()
    {
        return &m_views[static_cast<int>(m_activeViewType)];
    }

    void Scene::SetViewType(ViewType type)
    {
        m_activeViewType = type;
        UpdateViewMatrix();
    }

    void Scene::ResetViews()
    {
        for (int i = 0; i < 4; i++)
        {
            m_views[i].Reset();
            switch (static_cast<ViewType>(i))
            {
                case ViewType::Perspective:
                    m_views[i].Fly(2);
                    m_views[i].Walk(-5);
                    m_views[i].SetViewType(ViewType::Perspective);
                    break;
                case ViewType::Top:
                    m_views[i].Fly(12);
                    m_views[i].Pitch(D3DX_PI / 2);
                    m_views[i].SetViewType(ViewType::Top);
                    break;
                case ViewType::Left:
                    m_views[i].Yaw(D3DX_PI / 2);
                    m_views[i].Walk(-12);
                    m_views[i].SetViewType(ViewType::Left);
                    break;
                case ViewType::Front:
                    m_views[i].Yaw(D3DX_PI);
                    m_views[i].Walk(-12);
                    m_views[i].SetViewType(ViewType::Front);
                    break;
            }
        }
    }

    std::string Scene::GetStats()
    {
        size_t vertCount = 0;
        for (const auto &actor : m_actors)
        {
            if (actor.second->GetType() == ActorType::Model)
            {
                vertCount += actor.second->GetVertices().size();
            }
        }
        return std::string("Actors:").append(std::to_string(m_actors.size()))
            .append(" | Tris:").append(std::to_string(vertCount / 3));
    }

    bool Scene::ToggleMovementSpace()
    {
        auto toggled = m_gizmo.ToggleSpace();
        if (!m_selectedActorIds.empty())
        {
            m_gizmo.Update(m_actors[m_selectedActorIds.back()].get());
        }
        return toggled;
    }

    bool Scene::ToggleFillMode()
    {
        if (m_fillMode == D3DFILL_SOLID)
        {
            m_fillMode = D3DFILL_WIREFRAME;
            return false;
        }

        m_fillMode = D3DFILL_SOLID;
        return true;
    }

    void Scene::Release()
    {
        m_grid.Release();
        m_gizmo.Release();

        for (const auto &actor : m_actors)
        {
            actor.second->Release();
        }
    }

    void Scene::Delete()
    {
        // Copy selected ids since loop modifies the master selected actor id vector.
        const auto groupId = Util::NewUuid();
        auto selectedActorIds = m_selectedActorIds;

        for (const auto &selectedActorId : selectedActorIds)
        {
            m_auditor.DeleteActor("Actor", selectedActorId, groupId);
            Delete(m_actors[selectedActorId]);
        }
    }

    void Scene::Delete(std::shared_ptr<Actor> actor)
    {
        actor->Release();

        // Unselect actor if selected.
        auto it = find(m_selectedActorIds.begin(), m_selectedActorIds.end(), actor->GetId());
        if (it != m_selectedActorIds.end()) m_selectedActorIds.erase(it);

        m_actors.erase(actor->GetId());
        SetDirty(true);
    }

    void Scene::Duplicate()
    {
        const auto groupId = Util::NewUuid();

        for (const auto &selectedActorId : m_selectedActorIds)
        {
            switch (m_actors[selectedActorId]->GetType())
            {
                case ActorType::Model:
                {
                    const auto selectedModel = dynamic_cast<Model *>(m_actors[selectedActorId].get());
                    auto model = std::make_shared<Model>(*selectedModel);
                    m_actors[model->GetId()] = model;
                    m_auditor.AddActor("Model", model->GetId(), groupId);

                    // Give duplicate a fresh copy of texture.
                    if (selectedModel->GetTexture()->IsLoaded())
                        model->SetTexture(m_renderDevice.GetDevice(), selectedModel->GetTexture()->GetId());

                    break;
                }
                case ActorType::Camera:
                {
                    auto camera = std::make_shared<Camera>(*dynamic_cast<Camera *>(m_actors[selectedActorId].get()));
                    m_actors[camera->GetId()] = camera;
                    m_auditor.AddActor("Camera", camera->GetId(), groupId);
                    break;
                }
            }
        }
    }

    void Scene::FocusSelected()
    {
        if (m_selectedActorIds.size() > 0)
        {
            auto selectedActor = m_actors[m_selectedActorIds[0]];
            GetActiveView()->SetPosition(selectedActor->GetPosition() + (GetActiveView()->GetForward() * -2.5f));
        }
    }

    void Scene::SetScript(std::string script)
    {
        if (!m_selectedActorIds.empty())
        {
            m_auditor.ChangeActor("Script Change", m_selectedActorIds[0]);
            m_actors[m_selectedActorIds[0]]->SetScript(script);
        }
    }

    std::string Scene::GetScript()
    {
        if (!m_selectedActorIds.empty())
        {
            return m_actors[m_selectedActorIds[0]]->GetScript();
        }
        return std::string("");
    }

    void Scene::SetBackgroundColor(COLORREF color)
    {
        if (GetBackgroundColor() != color)
        {
            m_auditor.ChangeScene("Background Color");
            Dirty([&] {
                m_backgroundColorRGB[0] = GetRValue(color);
                m_backgroundColorRGB[1] = GetGValue(color);
                m_backgroundColorRGB[2] = GetBValue(color);
            }, &m_backgroundColorRGB);
        }
    }

    void Scene::SetGizmoSnapSize(float size)
    {
        float prevSnapSize = m_gizmo.GetSnapSize();
        if (prevSnapSize != size)
        {
            m_auditor.ChangeScene("Gizmo Snap Size");
            Dirty([&] {
                prevSnapSize = size;
                m_gizmo.SetSnapSize(size);
            }, &prevSnapSize);
        }
    }

    COLORREF Scene::GetBackgroundColor()
    {
        return RGB(m_backgroundColorRGB[0], m_backgroundColorRGB[1], m_backgroundColorRGB[2]);
    }

    std::vector<Actor *> Scene::GetActors(bool selectedOnly)
    {
        std::vector<Actor *> actors;
        if (selectedOnly)
        {
            for (const auto &actorId : m_selectedActorIds)
            {
                actors.push_back(GetActor(actorId).get());
            }
        }
        else
        {
            for (const auto &actor : m_actors)
            {
                actors.push_back(actor.second.get());
            }
        }
        return actors;
    }

    std::shared_ptr<Actor> Scene::GetActor(const boost::uuids::uuid &id)
    {
        if (m_actors.find(id) != m_actors.end())
            return m_actors[id];
        return NULL;
    }

    bool Scene::IsActorSelected(const boost::uuids::uuid &id)
    {
        auto it = std::find(m_selectedActorIds.begin(), m_selectedActorIds.end(), id);
        return it != m_selectedActorIds.end();
    }

    void Scene::SelectActorById(const boost::uuids::uuid &id, bool clearAll)
    {
        if (clearAll) UnselectAll();

        if (IsActorSelected(id))
        {
            // Unselect actor when already selected and clicked on again.
            auto it = std::find(m_selectedActorIds.begin(), m_selectedActorIds.end(), id);
            m_selectedActorIds.erase(it);

            // Select previous selected actor if any available.
            if (!m_selectedActorIds.empty())
                m_gizmo.Update(m_actors[m_selectedActorIds.back()].get());
        }
        else
        {
            // Add to selection and move gizmo to its location.
            m_selectedActorIds.push_back(id);
            m_gizmo.Update(m_actors[id].get());
        }
    }

    void Scene::SelectAll()
    {
        UnselectAll();
        for (const auto &actor : m_actors)
        {
            SelectActorById(actor.first, false);
        }
    }

    void Scene::UnselectAll()
    {
        m_selectedActorIds.clear();
    }

    void Scene::SetTitle(std::string title, bool store)
    {
        if (store) m_sceneName = title;
        title.append(" - ").append(APP_NAME);
        SetWindowText(m_hWnd, title.c_str());
    }

    void Scene::SetDirty(bool value)
    {
        Savable::SetDirty(value);

        std::string newSceneName(m_sceneName);
        if (value)
        {
            newSceneName.append("*");
        }
        SetTitle(newSceneName, false);
    }

    void Scene::WrapCursor()
    {
        const int screenX = GetSystemMetrics(SM_CXVIRTUALSCREEN) - 1;
        const int screenY = GetSystemMetrics(SM_CYVIRTUALSCREEN) - 1;

        POINT mousePoint;
        GetCursorPos(&mousePoint);

        if (mousePoint.x >= screenX)
            SetCursorPos(1, mousePoint.y);
        else if (mousePoint.x < 1)
            SetCursorPos(screenX - 1, mousePoint.y);
        else if (mousePoint.y >= screenY)
            SetCursorPos(mousePoint.x, 1);
        else if (mousePoint.y < 1)
            SetCursorPos(mousePoint.x, screenY - 1);
    }

    bool Scene::HasPath()
    {
        return !m_path.empty();
    }

    nlohmann::json Scene::Save()
    {
        json scene = {
            { "version", m_version },
            { "active_view", GetActiveView()->GetType() }
        };

        for (int i = 0; i < 4; i++)
        {
            scene["views"].push_back(m_views[i].Save());
        }

        for (const auto &actor : m_actors)
        {
            scene["actors"].push_back(actor.second->Save());
        }

        auto partial = PartialSave();
        scene.update(partial);

        return scene;
    }

    nlohmann::json Scene::PartialSave()
    {
        return {
            { "background_color", m_backgroundColorRGB },
            { "gizmo_snap_size", m_gizmo.GetSnapSize() }
        };
    }

    void Scene::Load(const nlohmann::json &root)
    {
        m_version = root["version"];

        int count = 0;
        for (const auto &view : root["views"])
        {
            m_views[count++].Load(view);
        }

        SetViewType(root["active_view"]);

        for (const auto &actor : root["actors"])
        {
            RestoreActor(actor);
        }

        PartialLoad(root);
    }

    void Scene::PartialLoad(const nlohmann::json &root)
    {
        m_backgroundColorRGB = root["background_color"];
        m_gizmo.SetSnapSize(root["gizmo_snap_size"]);
    }

    void Scene::RestoreActor(const nlohmann::json &actor, bool markSceneDirty)
    {
        // Avoid creation of new actor objects when restoring.
        auto existingActor = GetActor(actor["id"]);

        switch (actor["type"].get<ActorType>())
        {
            case ActorType::Model:
            {
                auto model = existingActor ? std::static_pointer_cast<Model>(existingActor) : std::make_shared<Model>();
                model->Load(actor, m_renderDevice.GetDevice());
                if (!existingActor) m_actors[model->GetId()] = model;
                break;
            }
            case ActorType::Camera:
            {
                auto camera = existingActor ? std::static_pointer_cast<Camera>(existingActor) : std::make_shared<Camera>();
                camera->Load(actor);
                if (!existingActor) m_actors[camera->GetId()] = camera;
                break;
            }
        }

        SetDirty(markSceneDirty);
    }
}
