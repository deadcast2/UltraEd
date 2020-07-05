#include "Gui.h"
#include "PubSub.h"
#include "View.h"

namespace UltraEd
{
    Gui::Gui(HWND hWnd, IDirect3DDevice9 *device)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();
        ImGui_ImplWin32_Init(hWnd);
        ImGui_ImplDX9_Init(device);
    }

    Gui::~Gui()
    {
        ImGui_ImplDX9_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }

    void Gui::PrepareFrame()
    {
        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Exit"))
                {
                    PubSub::Publish("Exit");
                }

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("View"))
            {
                if (ImGui::MenuItem("Perspective"))
                {
                    ViewType type = ViewType::Perspective;
                    PubSub::Publish("ViewChange", static_cast<void*>(&type));
                }

                if (ImGui::MenuItem("Top"))
                {
                    ViewType type = ViewType::Top;
                    PubSub::Publish("ViewChange", static_cast<void *>(&type));
                }

                if (ImGui::MenuItem("Left"))
                {
                    ViewType type = ViewType::Left;
                    PubSub::Publish("ViewChange", static_cast<void *>(&type));
                }

                if (ImGui::MenuItem("Front"))
                {
                    ViewType type = ViewType::Front;
                    PubSub::Publish("ViewChange", static_cast<void *>(&type));
                }

                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }

        ImGui::EndFrame();
    }

    void Gui::RenderFrame()
    {
        ImGui::Render();
        ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
    }

    bool Gui::WantsMouse()
    {
        return ImGui::GetIO().WantCaptureMouse;
    }

    void Gui::RebuildWith(function<void()> inner)
    {
        ImGui_ImplDX9_InvalidateDeviceObjects();

        if (inner) inner();

        ImGui_ImplDX9_CreateDeviceObjects();
    }
}
