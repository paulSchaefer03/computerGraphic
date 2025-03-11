#pragma once
#include <glbinding/gl46core/gl.h>
using namespace gl46core;
#include <glm/glm.hpp>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>
#include "time.hpp"
#include "window.hpp"
#include "entities/model.hpp"

struct ImGuiManager {
    void init(Window& window) {
        ImGui::CreateContext();
        ImGui_ImplSDL3_InitForOpenGL(window._window_p, window._context);
        ImGui_ImplOpenGL3_Init();
    }
    
    void newFrame(std::vector<Model>& models, int number_of_lights) {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
        renderUI(models, number_of_lights);
        ImGui::Render();
    }
      
    void render() {
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void processEvent(SDL_Event* event_p) {
        ImGui_ImplSDL3_ProcessEvent(event_p);
    }
    
    void shutdown() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();
    }

    void renderAberationUI(float& aberrationStrength) {
        ImGui::Begin("Aberration window");
        ImGui::SliderFloat("Aberration Strength", &aberrationStrength, 0.0f, 1.0f);
        ImGui::End();
    }
    
    void renderUI(std::vector<Model>& models, int number_of_lights) {
        Time::update();
        ImGui::Begin("FPS window");
        ImGui::Text("%.1f fps, %.2f ms", ImGui::GetIO().Framerate, Time::get_delta() * 1000); 
        ImGui::End();
        
        for (size_t i = 0; i < models.size() - number_of_lights; ++i) {
            ImGui::Begin(("Model " + std::to_string(i) + " Controls").c_str());
            ImGui::SliderFloat("pos x", &models[i]._transform._position.x, -15, +15);
            ImGui::SliderFloat("pos y", &models[i]._transform._position.y, -15, +15);
            ImGui::SliderFloat("pos z", &models[i]._transform._position.z, -15, +15);
            ImGui::Checkbox("Use Custom Metallic", &models[i]._useCustomMetallic);
            if (models[i]._useCustomMetallic) {
                ImGui::SliderFloat("Custom Metallic", &models[i]._metallic, 0.0f, 1.0f);
            }

            ImGui::Checkbox("Use Custom Roughness", &models[i]._useCustomRoughness);
            if (models[i]._useCustomRoughness) {
                ImGui::SliderFloat("Custom Roughness", &models[i]._roughness, 0.0f, 1.0f);
            }
            ImGui::SliderFloat("heightScale", &models[i]._heightScale, 0.0f, 0.5f);
            ImGui::End();
        }
    }
};
