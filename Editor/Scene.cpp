#include <thread>
#include "Build.h"
#include "Debug.h"
#include "Scene.h"
#include "FileIO.h"
#include "Dialog.h"
#include "Util.h"
#include "BoxCollider.h"
#include "SphereCollider.h"
#include "PubSub.h"

namespace UltraEd
{
    Scene::Scene() :
        m_defaultMaterial(),
        m_fillMode(D3DFILLMODE::D3DFILL_SOLID),
        m_gizmo(),
        m_views(),
        m_device(0),
        m_d3d9(0),
        m_d3dpp(),
        m_actors(),
        m_grid(),
        m_selectedActorIds(),
        m_mouseSmoothX(0),
        m_mouseSmoothY(0),
        m_activeViewType(ViewType::Perspective),
        m_sceneName(),
        m_backgroundColorRGB(),
        m_auditor(this),
        m_gui()
    {
        m_defaultMaterial.Diffuse.r = m_defaultMaterial.Ambient.r = 1.0f;
        m_defaultMaterial.Diffuse.g = m_defaultMaterial.Ambient.g = 1.0f;
        m_defaultMaterial.Diffuse.b = m_defaultMaterial.Ambient.b = 1.0f;
        m_defaultMaterial.Diffuse.a = m_defaultMaterial.Ambient.a = 1.0f;

        PubSub::Subscribe({ "Resize", [&](void *data) {
            auto rect = static_cast<std::tuple<int, int> *>(data);
            if (rect) Resize(std::get<0>(*rect), std::get<1>(*rect));
        } });
    }

    Scene::~Scene()
    {
        ReleaseResources(ModelRelease::AllResources);
        if (m_device) m_device->Release();
        if (m_d3d9) m_d3d9->Release();
    }

    bool Scene::Create(HWND hWnd)
    {
        if (hWnd == NULL) return false;
        if ((m_d3d9 = Direct3DCreate9(D3D_SDK_VERSION)) == NULL) return false;

        D3DDISPLAYMODE d3ddm;
        if (FAILED(m_d3d9->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3ddm))) return false;

        m_d3dpp.Windowed = TRUE;
        m_d3dpp.SwapEffect = D3DSWAPEFFECT_COPY;
        m_d3dpp.BackBufferFormat = d3ddm.Format;
        m_d3dpp.EnableAutoDepthStencil = TRUE;
        m_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

        if (FAILED(m_d3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
            D3DCREATE_SOFTWARE_VERTEXPROCESSING, &m_d3dpp, &m_device))) return false;

        m_gui = std::make_unique<Gui>(this, hWnd, m_device);

        OnNew();

        return true;
    }

    void Scene::OnNew(bool confirm)
    {
        if (confirm && !Confirm()) return;

        SetTitle("Untitled");
        UnselectAll();
        ReleaseResources(ModelRelease::AllResources);
        m_actors.clear();
        m_auditor.Reset();
        ResetViews();
        m_backgroundColorRGB[0] = m_backgroundColorRGB[1] = m_backgroundColorRGB[2] = 0;
        m_gizmo.SetSnapSize(0.5f);
        SetDirty(false);
    }

    bool Scene::OnSave()
    {
        std::string savedName;
        if (FileIO::Save(this, savedName))
        {
            SetTitle(savedName);
            SetDirty(false);
            return true;
        }

        return false;
    }

    void Scene::OnLoad()
    {
        if (!Confirm()) return;

        cJSON *root = NULL;
        std::string loadedName;
        if (FileIO::Load(&root, loadedName))
        {
            OnNew(false);
            SetTitle(loadedName);
            Load(root);
            cJSON_Delete(root);
        }
    }

    void Scene::OnBuildROM(BuildFlag flag)
    {
        std::thread run([this, flag]() {
            if (Build::Start(this))
            {
                if (static_cast<int>(flag) & static_cast<int>(BuildFlag::Run))
                {
                    Build::Run();
                }
                else if (static_cast<int>(flag) & static_cast<int>(BuildFlag::Load))
                {
                    Build::Load(GetWndHandle());
                }
            }
            else
            {
                Debug::Error("The ROM build has failed. Make sure the build tools have been installed.");
            }
        });
        run.detach();
    }

    void Scene::OnAddModel(ModelPreset preset)
    {
        std::shared_ptr<Model> model = NULL;

        switch (preset)
        {
            case ModelPreset::Custom:
            {
                std::string file;
                if (Dialog::Open("Add Model",
                    "3D Studio (*.3ds)\0*.3ds\0Blender (*.blend)\0*.blend\0Autodesk (*.fbx)\0*.fbx\0"
                    "Collada (*.dae)\0*.dae\0DirectX (*.x)\0*.x\0Stl (*.stl)\0*.stl\0"
                    "VRML (*.wrl)\0*.wrl\0Wavefront (*.obj)\0*.obj", file))
                {
                    model = std::make_shared<Model>(file.c_str());
                    m_actors[model->GetId()] = model;
                    model->SetName(std::string("Actor ").append(std::to_string(m_actors.size())));
                    m_auditor.AddActor("Model", model->GetId());
                }
                break;
            }
            case ModelPreset::Pumpkin:
            {
                model = std::make_shared<Model>("presets/pumpkin.fbx");
                m_actors[model->GetId()] = model;
                model->SetName(std::string("Pumpkin ").append(std::to_string(m_actors.size())));
                model->SetTexture(m_device, "presets/pumpkin.png");
                m_auditor.AddActor("Pumpkin", model->GetId());
                break;
            }
        }

        if (model != NULL)
        {
            SelectActorById(model->GetId());
        }
    }

    void Scene::OnAddCamera()
    {
        auto newCamera = std::make_shared<Camera>();
        m_actors[newCamera->GetId()] = newCamera;
        newCamera->SetName(std::string("Camera ").append(std::to_string(m_actors.size())));
        m_auditor.AddActor("Camera", newCamera->GetId());

        SelectActorById(newCamera->GetId());
    }

    void Scene::OnAddCollider(ColliderType type)
    {
        if (m_selectedActorIds.empty())
        {
            Debug::Warning("An object must be selected first.");
        }

        GUID groupId = Util::NewGuid();

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

    void Scene::OnDeleteCollider()
    {
        if (m_selectedActorIds.empty())
        {
            Debug::Warning("An object must be selected first.");
        }

        GUID groupId = Util::NewGuid();

        for (const auto &selectedActorId : m_selectedActorIds)
        {
            m_auditor.ChangeActor("Delete Collider", selectedActorId, groupId);

            m_actors[selectedActorId]->SetCollider(NULL);
        }
    }

    void Scene::OnAddTexture()
    {
        std::string file;

        if (m_selectedActorIds.empty())
        {
            Debug::Warning("An actor must be selected first.");
            return;
        }

        if (Dialog::Open("Select a texture",
            "PNG (*.png)\0*.png\0JPEG (*.jpg)\0*.jpg\0BMP (*.bmp)\0*.bmp\0TGA (*.tga)\0*.tga", file))
        {
            GUID groupId = Util::NewGuid();

            for (const auto &selectedActorId : m_selectedActorIds)
            {
                if (m_actors[selectedActorId]->GetType() != ActorType::Model) continue;

                m_auditor.ChangeActor("Add Texture", selectedActorId, groupId);

                if (!dynamic_cast<Model *>(m_actors[selectedActorId].get())->SetTexture(m_device, file.c_str()))
                {
                    Debug::Warning("Texture could not be loaded.");
                }
            }
        }
    }

    void Scene::OnDeleteTexture()
    {
        if (m_selectedActorIds.empty())
        {
            Debug::Warning("An actor must be selected first.");
            return;
        }

        GUID groupId = Util::NewGuid();

        for (const auto &selectedActorId : m_selectedActorIds)
        {
            if (m_actors[selectedActorId]->GetType() != ActorType::Model) continue;

            m_auditor.ChangeActor("Delete Texture", selectedActorId, groupId);

            dynamic_cast<Model *>(m_actors[selectedActorId].get())->DeleteTexture();
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

    void Scene::CheckInput()
    {
        View *view = GetActiveView();
        static bool prevGizmo = false;
        static GUID groupId = Util::NewGuid();

        const float deltaTime = m_gui->IO().DeltaTime;
        const float smoothingModifier = 20.0f;
        const float mouseSpeedModifier = 0.55f;

        if (m_gui->IO().WantCaptureMouse || m_gui->IO().WantCaptureKeyboard)
            return;

        Actor *selectedActor = NULL;
        if (m_gui->IO().MouseReleased[1] &&
            m_gui->IO().MouseDownDurationPrev[1] < 0.2f &&
            Pick(m_gui->IO().MousePos, true, &selectedActor))
        {
            PubSub::Publish("ContextMenu", selectedActor);
        }

        if (m_gui->IO().MouseClicked[0]) Pick(m_gui->IO().MousePos);
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
            ScreenRaycast(m_gui->IO().MousePos, &rayOrigin, &rayDir);
            if (prevGizmo || (prevGizmo = m_gizmo.Select(rayOrigin, rayDir)))
            {
                GUID lastSelectedActorId = m_selectedActorIds.back();
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
            groupId = Util::NewGuid();
            m_gizmo.Reset();
        }
    }

    void Scene::Resize(int width, int height)
    {
        if (m_device && m_gui)
        {
            m_d3dpp.BackBufferWidth = width;
            m_d3dpp.BackBufferHeight = height;

            m_gui->RebuildWith([&]() {
                ReleaseResources(ModelRelease::VertexBufferOnly);
                m_device->Reset(&m_d3dpp);
                UpdateViewMatrix();
            });
        }
    }

    void Scene::UpdateViewMatrix()
    {
        D3DXMATRIX viewMat;
        const float aspect = static_cast<float>(m_d3dpp.BackBufferWidth) / static_cast<float>(m_d3dpp.BackBufferHeight);

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

        m_device->SetTransform(D3DTS_PROJECTION, &viewMat);
    }

    void Scene::Render()
    {
        CheckInput();
        CheckChanges();

        if (!m_device || !m_gui) return;

        m_gui->PrepareFrame();

        ID3DXMatrixStack *stack;
        if (!SUCCEEDED(D3DXCreateMatrixStack(0, &stack))) return;
        stack->LoadMatrix(&GetActiveView()->GetViewMatrix());

        m_device->SetTransform(D3DTS_WORLD, stack->GetTop());
        m_device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
            D3DCOLOR_XRGB(m_backgroundColorRGB[0], m_backgroundColorRGB[1], m_backgroundColorRGB[2]), 1.0f, 0);

        if (SUCCEEDED(m_device->BeginScene()))
        {
            m_grid.Render(m_device);

            // Render all actors with selected fill mode.
            m_device->SetMaterial(&m_defaultMaterial);
            m_device->SetRenderState(D3DRS_ZENABLE, TRUE);
            m_device->SetRenderState(D3DRS_FILLMODE, m_fillMode);
            m_device->SetRenderState(D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_COLOR1);

            for (const auto &actor : m_actors)
            {
                // Highlight any selected actors.
                m_device->SetRenderState(D3DRS_AMBIENT, IsActorSelected(actor.first) ? 0x0000ff00 : 0xffffffff);
                actor.second->Render(m_device, stack);
            }

            if (!m_selectedActorIds.empty())
            {
                // Draw the gizmo on "top" of all objects in scene.
                m_device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
                m_device->SetRenderState(D3DRS_ZENABLE, FALSE);
                m_gizmo.Render(m_device, stack, GetActiveView());
            }

            // Render the GUI on top of scene.
            m_device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
            m_gui->RenderFrame();

            m_device->EndScene();
            m_device->Present(NULL, NULL, NULL, NULL);
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
        View *view = GetActiveView();

        D3DVIEWPORT9 viewport;
        m_device->GetViewport(&viewport);

        D3DXMATRIX matProj;
        m_device->GetTransform(D3DTS_PROJECTION, &matProj);

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
        if (!m_selectedActorIds.empty())
        {
            return m_gizmo.ToggleSpace(m_actors[m_selectedActorIds.back()].get());
        }
        return false;
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

    void Scene::ReleaseResources(ModelRelease type)
    {
        m_grid.Release();
        m_gizmo.Release();

        for (const auto &actor : m_actors)
        {
            if (auto model = dynamic_cast<Model *>(actor.second.get()))
            {
                model->Release(type);
            }
            else
            {
                actor.second->Release();
            }
        }
    }

    void Scene::Delete()
    {
        // Copy selected ids since loop modifies the master selected actor id vector.
        GUID groupId = Util::NewGuid();
        auto selectedActorIds = m_selectedActorIds;
        for (const auto &selectedActorId : selectedActorIds)
        {
            m_auditor.DeleteActor("Actor", selectedActorId, groupId);
            Delete(m_actors[selectedActorId]);
            SetDirty(true);
        }
    }

    void Scene::Delete(std::shared_ptr<Actor> actor)
    {
        if (auto model = dynamic_cast<Model *>(actor.get()))
        {
            model->Release(ModelRelease::AllResources);
        }
        else
        {
            actor->Release();
        }

        // Unselect actor if selected.
        auto it = find(m_selectedActorIds.begin(), m_selectedActorIds.end(), actor->GetId());
        if (it != m_selectedActorIds.end()) m_selectedActorIds.erase(it);

        m_actors.erase(actor->GetId());
    }

    void Scene::Duplicate()
    {
        GUID groupId = Util::NewGuid();

        for (const auto &selectedActorId : m_selectedActorIds)
        {
            switch (m_actors[selectedActorId]->GetType())
            {
                case ActorType::Model:
                {
                    auto model = std::make_shared<Model>(*dynamic_cast<Model *>(m_actors[selectedActorId].get()));
                    std::string texturePath = model->GetResources()["textureDataPath"];
                    model->SetTexture(m_device, texturePath.c_str());
                    m_actors[model->GetId()] = model;
                    m_auditor.AddActor("Model", model->GetId(), groupId);
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

    HWND Scene::GetWndHandle()
    {
        D3DDEVICE_CREATION_PARAMETERS params;
        if (m_device && SUCCEEDED(m_device->GetCreationParameters(&params)))
        {
            return params.hFocusWindow;
        }
        return NULL;
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

    std::shared_ptr<Actor> Scene::GetActor(GUID id)
    {
        if (m_actors.find(id) != m_actors.end())
            return m_actors[id];
        return NULL;
    }

    bool Scene::IsActorSelected(GUID id)
    {
        auto it = std::find(m_selectedActorIds.begin(), m_selectedActorIds.end(), id);
        return it != m_selectedActorIds.end();
    }

    void Scene::SelectActorById(GUID id, bool clearAll)
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
        SetWindowText(GetWndHandle(), title.c_str());
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

    bool Scene::Confirm()
    {
        if (IsDirty())
        {
            int choice = MessageBox(NULL, "Would you like to save your changes?", "Are you sure?", MB_YESNOCANCEL | MB_ICONQUESTION);
            switch (choice)
            {
                case IDCANCEL:
                    return false;
                case IDYES:
                    if (!OnSave()) return false;
            }
        }
        return true;
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

    cJSON *Scene::Save()
    {
        char buffer[128];
        cJSON *scene = cJSON_CreateObject();

        cJSON *viewArray = cJSON_CreateArray();
        cJSON_AddItemToObject(scene, "views", viewArray);
        for (int i = 0; i < 4; i++)
        {
            cJSON_AddItemToArray(viewArray, m_views[i].Save());
        }

        sprintf(buffer, "%i", (int)GetActiveView()->GetType());
        cJSON_AddStringToObject(scene, "active_view", buffer);

        cJSON *actorArray = cJSON_CreateArray();
        cJSON_AddItemToObject(scene, "actors", actorArray);
        for (const auto &actor : m_actors)
        {
            cJSON_AddItemToArray(actorArray, actor.second->Save());
        }

        PartialSave(scene);

        return scene;
    }

    cJSON *Scene::PartialSave(cJSON *root)
    {
        char buffer[128];
        cJSON *scene = NULL;

        if (root == NULL)
            scene = cJSON_CreateObject();
        else
            scene = root;

        sprintf(buffer, "%i %i %i", m_backgroundColorRGB[0], m_backgroundColorRGB[1],
            m_backgroundColorRGB[2]);
        cJSON_AddStringToObject(scene, "background_color", buffer);

        sprintf(buffer, "%f", m_gizmo.GetSnapSize());
        cJSON_AddStringToObject(scene, "gizmo_snap_size", buffer);

        return scene;
    }

    bool Scene::Load(cJSON *root)
    {
        // Restore editor views.
        int count = 0;
        cJSON *views = cJSON_GetObjectItem(root, "views");
        cJSON *viewItem = NULL;
        cJSON_ArrayForEach(viewItem, views)
        {
            m_views[count++].Load(viewItem);
        }

        // Set the active view.
        ViewType viewType;
        cJSON *activeView = cJSON_GetObjectItem(root, "active_view");
        sscanf(activeView->valuestring, "%i", &viewType);
        SetViewType(viewType);

        // Restore saved actors.
        cJSON *actors = cJSON_GetObjectItem(root, "actors");
        cJSON *actor = NULL;
        cJSON_ArrayForEach(actor, actors)
        {
            RestoreActor(actor);
        }

        PartialLoad(root);

        return true;
    }

    bool Scene::PartialLoad(cJSON *root)
    {
        // Set the background color.
        cJSON *backgroundColor = cJSON_GetObjectItem(root, "background_color");
        sscanf(backgroundColor->valuestring, "%i %i %i", &m_backgroundColorRGB[0],
            &m_backgroundColorRGB[1], &m_backgroundColorRGB[2]);

        // Set gizmo snap size.
        float snapSize;
        cJSON *gizmoSnapSize = cJSON_GetObjectItem(root, "gizmo_snap_size");
        sscanf(gizmoSnapSize->valuestring, "%f", &snapSize);
        m_gizmo.SetSnapSize(snapSize);

        return true;
    }

    void Scene::RestoreActor(cJSON *item)
    {
        // Avoid creation of new actor objects when restoring.
        auto existingActor = GetActor(Actor::GetId(item));
        switch (Actor::GetType(item))
        {
            case ActorType::Model:
            {
                auto model = existingActor ? std::static_pointer_cast<Model>(existingActor) : std::make_shared<Model>();
                model->Load(item, m_device);
                if (!existingActor) m_actors[model->GetId()] = model;
                break;
            }
            case ActorType::Camera:
            {
                auto camera = existingActor ? std::static_pointer_cast<Camera>(existingActor) : std::make_shared<Camera>();
                camera->Load(item);
                if (!existingActor) m_actors[camera->GetId()] = camera;
                break;
            }
        }
    }
}
