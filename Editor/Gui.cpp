#include "Gui.h"
#include "PubSub.h"

namespace UltraEd
{
    Gui::Gui(HWND hWnd, IDirect3DDevice9 *device)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
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
            ImGui::EndMainMenuBar();
        }

        ImGui::EndFrame();
    }

    void Gui::RenderFrame()
    {
        ImGui::Render();
        ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
    }
}
