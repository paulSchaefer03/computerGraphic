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
#include "entities/postProcess.hpp"

struct ImGuiManager {
    void init(Window& window) {
        ImGui::CreateContext();
        ImGui_ImplSDL3_InitForOpenGL(window._window_p, window._context);
        ImGui_ImplOpenGL3_Init();
    }
    
    void newFrame(std::vector<Model>& models, PostProcess& postProcess, int number_of_lights, std::vector<Light>& lights) {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
        renderUI(models, number_of_lights);
        lightingOptions(lights);
        postProcessingOptions(postProcess);
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

    void lightingOptions(std::vector<Light>& lights) {
        ImGui::Begin("Lighting Options");
        for (size_t i = 0; i < lights.size(); ++i) {
            ImGui::Text("Light %d", i);
            ImGui::SliderFloat(("Light " + std::to_string(i) + " ColorR").c_str(), &lights[i]._color.r, 0, 20);
            ImGui::SliderFloat(("Light " + std::to_string(i) + " ColorG").c_str(), &lights[i]._color.g, 0, 20);
            ImGui::SliderFloat(("Light " + std::to_string(i) + " ColorB").c_str(), &lights[i]._color.b, 0, 20);
            ImGui::SliderFloat(("Light " + std::to_string(i) + " Range").c_str(), &lights[i]._range, 0, 100);
        }
        ImGui::End();
    }

    void postProcessingOptions(PostProcess& postProcess) {
        ImGui::Begin("Post-Processing Options");
        ImGui::SliderFloat("Motion Blur Sample", &postProcess._motionBlurSamples, 4.0f, 128.0f);
        ImGui::SliderFloat("Motion Blur Strength", &postProcess._motionBlurStrength, 0.0001f, 0.05f);
        ImGui::SliderFloat("Exposure", &postProcess._exposure, 0.1f, 5.0f);
        ImGui::SliderFloat("Gamma", &postProcess._gamma, 0.1f, 5.0f);
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
