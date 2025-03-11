#pragma once
#include <array>
#include <glbinding/gl46core/gl.h>
using namespace gl46core;
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_audio.h>
#include <glm/glm.hpp>
#include <fmt/base.h>
#include "time.hpp"
#include "window.hpp"
#include "input.hpp"
#include "pipeline.hpp"
#include "entities/camera.hpp"
#include "entities/model.hpp"
#include "entities/light.hpp"
#include "entities/hdr.hpp"
#include "imGui.hpp"
//----
#include "entities/cube.hpp"
#include "entities/postProcess.hpp"

struct Engine {
    void init() {
        Time::init();
        _window.init(_window_width, _window_height, "OpenGL Renderer", 4);
        _camera.set_perspective(_window_width, _window_height, 70);
        _imGuiManager.init(_window);

        // create pipeline for textured objects
/*      _pipeline.init("../assets/shaders/default.vert", "../assets/shaders/default.frag");*/
        _pipeline_shadows.init("../assets/shaders/shadows.vert", "../assets/shaders/shadows.frag");
        _pipeline_shadows.create_framebuffer();
        _pipeline_pbr.init("../assets/shaders/pbr.vert", "../assets/shaders/pbr.frag"); 
        _pipeline_skybox.init("../assets/shaders/skybox.vert", "../assets/shaders/skybox.frag");
        _pipeline_equirectangular.init("../assets/shaders/equirectangular_to_cubemap.vert", "../assets/shaders/equirectangular_to_cubemap.frag");
        _pipeline_irradiance.init("../assets/shaders/irradiance_convolution.vert", "../assets/shaders/irradiance_convolution.frag");
        _pipeline_prefilter_env.init("../assets/shaders/prefilter_env.vert", "../assets/shaders/prefilter_env.frag");
        _pipeline_brdf_lut.init("../assets/shaders/brdf_lut.vert", "../assets/shaders/brdf_lut.frag");
        _pipeline_tessellation.init("../assets/shaders/tessellation.vert", "../assets/shaders/tessellation.frag", "../assets/shaders/tessellation.tcs", "../assets/shaders/tessellation.tes");
        _pipeline_motion_blur.init("../assets/shaders/motion_blur.vert", "../assets/shaders/motion_blur.frag");
        _pipeline_post_process.init("../assets/shaders/sRGBcorrect.vert", "../assets/shaders/sRGBcorrect.frag"); // Post-Processing Shader laden
        _pipeline_motion_blur_vectors.init("../assets/shaders/motion_vector.vert", "../assets/shaders/motion_vector.frag");
        
        // create light and its shadow map
        _lights[0].init({+1.0, +3.0, -0.5}, {.992, .984, .827}, 100);
        _lights[1].init({+3.0, +1.5, +4.0}, {.992, .984, .827}, 100);
        _lights[2].init({-5.0, +1.6, +2.9}, {.992, .984, .827}, 100);
        // create HDR environment map
        _hdr.init("../assets/textures/HDR/Old-Train-Nuernberg-4K.hdr");
        //other hdr textures
        //_hdr.init("../assets/textures/HDR/HDR_silver_and_gold_nebulae.hdr");
        //_hdr.init("../assets/textures/HDR/Citychurch-Saint-Johannes-And-Saint-Martin-Schwabach-4K.hdr");  
        //_hdr.init("../assets/textures/HDR/814-hdri-skies-com.hdr");
        // create renderable models
        //Post Processing
        _postProcess.init(_window_width, _window_height);


        // create renderable models
        _models.emplace_back().initPBR(Mesh::cube, 1.0
         ,
            "../assets/textures/PBR/cgaxis_tree_roots_38_82_8K/tree_roots_38_82_diffuse.jpg",
            "../assets/textures/PBR/cgaxis_tree_roots_38_82_8K/tree_roots_38_82_normal.jpg", 
            "../assets/textures/PBR/cgaxis_tree_roots_38_82_8K/tree_roots_38_82_metallic.jpg",
            "../assets/textures/PBR/cgaxis_tree_roots_38_82_8K/tree_roots_38_82_roughness.jpg", 
            "../assets/textures/PBR/cgaxis_tree_roots_38_82_8K/tree_roots_38_82_ao.jpg",
            "../assets/textures/PBR/cgaxis_tree_roots_38_82_8K/tree_roots_38_82_height.jpg"//, 96, 96 
        );
        _models.back()._transform._position = glm::vec3(-6, 0, 1.5);
        _models.back()._transform._rotation = glm::vec3(1.5707963, 0, 0);//90 degrees in radians
        _models.emplace_back().initPBR(Mesh::eSphere, 1.0
         ,
            "../assets/textures/PBR/TerrazzoSlab028(1)/TerrazzoSlab028_COL_8K_METALNESS.png",
            "../assets/textures/PBR/TerrazzoSlab028(1)/TerrazzoSlab028_NRM_8K_METALNESS.png", 
            "../assets/textures/PBR/TerrazzoSlab028(1)/TerrazzoSlab028_METALNESS_8K_METALNESS.png",
            "../assets/textures/PBR/TerrazzoSlab028(1)/TerrazzoSlab028_ROUGHNESS_8K_METALNESS.png", 
            "../assets/textures/PBR/TerrazzoSlab028(1)/TerrazzoSlab028_AO_8K_METALNESS.png"
        );
        _models.back()._transform._position = glm::vec3(-4, 0, 1.5);
        _models.back()._transform._rotation = glm::vec3(1.5707963, 0, 0);//90 degrees in radians
        _models.emplace_back().initPBR(Mesh::eSphere, 1.0
         ,
            "../assets/textures/PBR/subtle-black-granite-bl/subtle-black-granite_albedo.png",
            "../assets/textures/PBR/subtle-black-granite-bl/subtle-black-granite_normal-ogl.png", 
            "../assets/textures/PBR/subtle-black-granite-bl/subtle-black-granite_metallic.png",
            "../assets/textures/PBR/subtle-black-granite-bl/subtle-black-granite_roughness.png", 
            "../assets/textures/PBR/subtle-black-granite-bl/subtle-black-granite_ao.png",
            "../assets/textures/PBR/subtle-black-granite-bl/subtle-black-granite_height.png"
            //"../assets/textures/PBR/cgaxis_violet_crystal_43_04_8K/violet_crystal_43_04_height.jpg"
        );
        _models.back()._transform._position = glm::vec3(-2, 0, 1.5);
        _models.back()._transform._rotation = glm::vec3(1.5707963, 0, 0);//90 degrees in radians
        _models.emplace_back().initPBR(Mesh::cube, 1,
             "../assets/textures/PBR/rounded-metal-cubes-bl/rounded-metal-cubes_albedo.png",
            "../assets/textures/PBR/rounded-metal-cubes-bl/rounded-metal-cubes_normal-ogl.png", 
            "../assets/textures/PBR/rounded-metal-cubes-bl/rounded-metal-cubes_metallic.png",
            "../assets/textures/PBR/rounded-metal-cubes-bl/rounded-metal-cubes_roughness.png", 
            "../assets/textures/PBR/rounded-metal-cubes-bl/rounded-metal-cubes_ao.png",
            "../assets/textures/PBR/rounded-metal-cubes-bl/rounded-metal-cubes_height.png" 
        );
        _models.back()._transform._position = glm::vec3(0, 0, 1.5);
        _models.emplace_back().initPBR(Mesh::eSphere, 0.5, 
            "../assets/textures/PBR/cgaxis_violet_crystal_43_04_8K/violet_crystal_43_04_diffuse.jpg",
            "../assets/textures/PBR/cgaxis_violet_crystal_43_04_8K/violet_crystal_43_04_normal.jpg", 
            "../assets/textures/PBR/cgaxis_violet_crystal_43_04_8K/violet_crystal_43_04_metallic.jpg",
            "../assets/textures/PBR/cgaxis_violet_crystal_43_04_8K/violet_crystal_43_04_roughness.jpg", 
            "../assets/textures/PBR/cgaxis_violet_crystal_43_04_8K/violet_crystal_43_04_ao.jpg",
            "../assets/textures/PBR/cgaxis_violet_crystal_43_04_8K/violet_crystal_43_04_height.jpg"
            );
        _models.back()._transform._position = glm::vec3(2, 0, 1.5);
        _models.back()._transform._rotation = glm::vec3(1.5707963, 0, 0);//90 degrees in radians
        _models.emplace_back().initPBR(Mesh::eSphere, 0.5, 
            "../assets/textures/PBR/mahogany_herringbone_wood_parquet_49_31_8K/mahogany_herringbone_wood_parquet_49_31_diffuse.jpg",
            "../assets/textures/PBR/mahogany_herringbone_wood_parquet_49_31_8K/mahogany_herringbone_wood_parquet_49_31_normal_opengl.jpg",
            "../assets/textures/PBR/mahogany_herringbone_wood_parquet_49_31_8K/mahogany_herringbone_wood_parquet_49_31_metallic.jpg",
            "../assets/textures/PBR/mahogany_herringbone_wood_parquet_49_31_8K/mahogany_herringbone_wood_parquet_49_31_roughness.jpg",
            "../assets/textures/PBR/mahogany_herringbone_wood_parquet_49_31_8K/mahogany_herringbone_wood_parquet_49_31_ao.jpg"
            );
        
        _models.back()._transform._position = glm::vec3(4, 0, 1.5);
        _models.back()._transform._rotation = glm::vec3(1.5707963, 0, 0);//90 degrees in radians 
        _models.emplace_back().initPBR(Mesh::cube, 1.0, 
            "../assets/textures/PBR/cgaxis_violet_crystal_43_04_8K/violet_crystal_43_04_diffuse.jpg",
            "../assets/textures/PBR/cgaxis_violet_crystal_43_04_8K/violet_crystal_43_04_normal.jpg", 
            "../assets/textures/PBR/cgaxis_violet_crystal_43_04_8K/violet_crystal_43_04_metallic.jpg",
            "../assets/textures/PBR/cgaxis_violet_crystal_43_04_8K/violet_crystal_43_04_roughness.jpg", 
            "../assets/textures/PBR/cgaxis_violet_crystal_43_04_8K/violet_crystal_43_04_ao.jpg",
            "../assets/textures/PBR/cgaxis_violet_crystal_43_04_8K/violet_crystal_43_04_height.jpg"
            );
        _models.back()._transform._position = glm::vec3(-2, 0, -0.5);
        _models.back()._transform._rotation = glm::vec3(1.5707963, 0, 0);//90 degrees in radians

        _models.emplace_back().initPBR(Mesh::eSphere, 0.5, 
            "../assets/textures/PBR/gold-scuffed-bl/gold-scuffed_basecolor-boosted.png",
            "../assets/textures/PBR/gold-scuffed-bl/gold-scuffed_normal.png", 
            "../assets/textures/PBR/gold-scuffed-bl/gold-scuffed_metallic.png",
            "../assets/textures/PBR/gold-scuffed-bl/gold-scuffed_roughness.png"
            );
        _models.back()._transform._position = glm::vec3(-2, 0, 3.5);
        _models.back()._transform._rotation = glm::vec3(1.5707963, 0, 0);//90 degrees in radians*/

        // create spheres to represent the lights
        for (auto& light: _lights) {
            _models.emplace_back().init(Mesh::eSphere, 0.5);
            _models.back()._transform._position = light._position;
        } 
        // audio stuff
        {
            SDL_InitSubSystem(SDL_INIT_AUDIO);
            // load .wav file from disk
            SDL_LoadWAV("../assets/audio/doom.wav", &audio_file.spec, &audio_file.buffer, &audio_file.buffer_size);
            // create an audio stream for default audio device
            audio_stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr, nullptr, nullptr);
            if (audio_stream == nullptr) fmt::println("{}", SDL_GetError());
            // get the format of the device (sample rate and such)
            SDL_AudioSpec device_format;
            SDL_GetAudioDeviceFormat(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &device_format, nullptr);
            // set up the audio stream to convert from our .wav file sample rate to the device's sample rate
            if(!SDL_SetAudioStreamFormat(audio_stream, &audio_file.spec, &device_format)) fmt::println("{}", SDL_GetError());
            // load .wav into the audio stream and play
            if(!SDL_PutAudioStreamData(audio_stream, audio_file.buffer, audio_file.buffer_size)) fmt::println("{}", SDL_GetError());
            if(!SDL_ResumeAudioStreamDevice(audio_stream)) fmt::println("{}", SDL_GetError());
        }
        _pipeline_equirectangular.bind();
        _hdr.bindAndCreateEnvMap(_pipeline_equirectangular._shader_program);
        _pipeline_irradiance.bind();
        _hdr.bindAndCreateIrradianceMap(_pipeline_irradiance._shader_program);
        _pipeline_prefilter_env.bind();
        _hdr.createPrefilterEnvMap(_pipeline_prefilter_env._shader_program);
        _pipeline_brdf_lut.bind();
        _hdr.ceateBRDFLUT();

        _postProcess.initMotionVectorBuffer(_window_width, _window_height);

    }
    void upload_shadow_textures(GLuint shader_program) {
        std::vector<GLint> texture_units;
        
        for (size_t i = 0; i < _lights.size(); i++) {
            texture_units.push_back(_lights[i]._shadow_texture);
        }
    
        glUniform1iv(glGetUniformLocation(shader_program, "tex_shadows"), texture_units.size(), texture_units.data());
    }
    void destroy() {
        // destroy audio stuff
        SDL_DestroyAudioStream(audio_stream);
        SDL_free(audio_file.buffer);
        // free HDR resources
        _hdr.destroy();
        // free OpenGL resources
        for (auto& light: _lights) light.destroy();
        for (auto& model: _models) model.destroy();
        //_pipeline.destroy();
        _window.destroy();
        
        // shut down imgui backend
        _imGuiManager.shutdown();
    }
    auto execute_event(SDL_Event* event_p) -> SDL_AppResult {
        // pass event over to imgui as well
        _imGuiManager.processEvent(event_p);
        // let input system process event
        Input::register_event(*event_p);
        switch (event_p->type) {
            case SDL_EventType::SDL_EVENT_QUIT: return SDL_AppResult::SDL_APP_SUCCESS;
            default: break;
        }
        return SDL_AppResult::SDL_APP_CONTINUE;   
    }
    void execute_input(){
        float speed = 0.1f;
        if(Input::Keys::down('W')) _camera.translate(0,0,-speed);
        if(Input::Keys::down('S')) _camera.translate(0,0,speed);
        if(Input::Keys::down('A')) _camera.translate(-speed,0,0);
        if(Input::Keys::down('D')) _camera.translate(speed,0,0);
        if(Input::Keys::down(SDLK_LSHIFT)) _camera.translate(0,-speed,0);
        if(Input::Keys::down(SDLK_SPACE)) _camera.translate(0,speed,0);

        if (_mouse_captured) {
            float rotationSpeed = 0.002f;
            _camera._rotation.x -= rotationSpeed * Mouse::delta().second;
            _camera._rotation.y -= rotationSpeed * Mouse::delta().first;
        }

        if(_mouse_captured && Keys::pressed(SDLK_ESCAPE)){
            _mouse_captured = false;
            SDL_SetWindowRelativeMouseMode(_window._window_p, _mouse_captured);
        }

        if(!_mouse_captured && Mouse::pressed(SDL_BUTTON_LEFT)){
            _mouse_captured = true;
            SDL_SetWindowRelativeMouseMode(_window._window_p, _mouse_captured);
        }
        if(Keys::down(SDLK_X)) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

}
    
    void execute_frame() {
        static std::vector<Model> models_without_lights(_models.begin(), _models.end() - _lights.size());
        // handle all the inputs such as camera movement
        //ImGUI for all Models without lights
        _imGuiManager.newFrame(_models, _lights.size());
        execute_input();

        //Save the old view projection matrix for motion blur
        _camera.update();
        // update the rotation of the models
        for(auto& models_without_lights: _models) {
            models_without_lights._transform._rotation += Time::get_delta();
        }
        
        // draw shadows
        if (_shadows_dirty) {
            glBindFramebuffer(GL_FRAMEBUFFER, _pipeline_shadows._framebuffer);
            // do this for each light
            for (auto& light: _lights) {
                _pipeline_shadows.bind();
                glViewport(0, 0, light._shadow_width, light._shadow_height);
                //glDisable(GL_SCISSOR_TEST);
                // render into each cubemap face
                for (int face = 0; face < 6; face++) {
                    // bind the target shadow map and clear it
                    //glClearDepth(0.5f);
                    light.bind_write(_pipeline_shadows._framebuffer, face);
                    glClearDepth(1.0f);
                    glClear(GL_DEPTH_BUFFER_BIT);
                    // draw the stuff
                 for (size_t i = 0; i < _models.size(); i++) {
                    _models[i].draw(true, false, _pipeline_shadows._shader_program); // PBR = false
                }
            }
                _shadows_dirty = false;
            } 
        }
        
        //Motion-Vector-Pass aktivieren
        glBindFramebuffer(GL_FRAMEBUFFER, _postProcess._motionVectorFBO);
        glViewport(0, 0, _window_width, _window_height);
        glClear(GL_COLOR_BUFFER_BIT);

        //Rendering mit Motion Vectors
        _pipeline_motion_blur_vectors.bind(_postProcess._motionVectorFBO);
        _camera.bind();
        _camera.bindPrevViewProjectionMatrix(_pipeline_motion_blur_vectors._shader_program);
        GLenum drawBuffers[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
        glDrawBuffers(2, drawBuffers); // 2 Attachments (Color + Motion)
        for (size_t i = 0; i < _models.size(); i++) {
            _models[i].draw(true, true, _pipeline_motion_blur_vectors._shader_program);                               
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glViewport(0, 0, _window_width, _window_height);
        glClearColor(0.1, 0.1, 0.1, 0.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
        //Init Post Processing (render the szene to a texture)
        glBindFramebuffer(GL_FRAMEBUFFER, _postProcess._postProcessFBO);
        glViewport(0, 0, _window_width, _window_height);
        //glClearColor(1.0, 0.0, 1.0, 1.0);  // Pink zum Debuggen
        glClearDepth(1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        //glDepthFunc(GL_ALWAYS);  // Erzwinge, dass alle Objekte zuerst sichtbar sind(testzwecke)
        glDepthMask(GL_TRUE);
        glClear(GL_DEPTH_BUFFER_BIT);

       {//PBR
        // clear screen before drawing
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glDepthMask(GL_TRUE);
        glClear(GL_DEPTH_BUFFER_BIT);
        
        _pipeline_pbr.bind(_postProcess._postProcessFBO);
        _hdr.bindIrrandianceMap(_pipeline_pbr._shader_program);
        _hdr.bindPrefilterEnvMap(_pipeline_pbr._shader_program);
        _hdr.bindBRDFLUT(_pipeline_pbr._shader_program);

        // Binde Lichter und Kamera
        for (int i = 0; i < _lights.size(); i++) {
            
            _lights[i].bind(i * 3); // Lichter binden
            if(_shadows_dirty) {
                glActiveTexture(GL_TEXTURE0 + (10 + i));  // Reserviere Textur-Einheit für Schatten-Textur
                glBindTexture(GL_TEXTURE_CUBE_MAP, _lights[i]._shadow_texture);
                glUniform1i(glGetUniformLocation(_pipeline_pbr._shader_program, ("tex_shadows[" + std::to_string(i) + "]").c_str()), 10 + i);
            }

        }
        
        _camera.bind();
        // Modelle mit PBR rendern
         for (size_t i = 0; i < _models.size(); i++) {
            if(i != 3 && i != 0 && i != 6) {
                _models[i].draw(true, true, _pipeline_pbr._shader_program, true, true, false);                
            }                 
        }
    }

    {//Tessellation
        _pipeline_tessellation.bind(_postProcess._postProcessFBO);
        _hdr.bindIrrandianceMap(_pipeline_tessellation._shader_program);
        _hdr.bindPrefilterEnvMap(_pipeline_tessellation._shader_program);
        _hdr.bindBRDFLUT(_pipeline_tessellation._shader_program);

        // Binde Lichter und Kamera
        for (int i = 0; i < _lights.size(); i++) {
            _lights[i].bind_read(i + 11, i * 3); // Lichter binden
            if(_shadows_dirty) {
                glActiveTexture(GL_TEXTURE0 + (10 + i));  // Reserviere Textur-Einheit für Schatten-Textur
                glBindTexture(GL_TEXTURE_CUBE_MAP, _lights[i]._shadow_texture);
                glUniform1i(glGetUniformLocation(_pipeline_tessellation._shader_program, ("tex_shadows[" + std::to_string(i) + "]").c_str()), 10 + i);
            }
        }
        _camera.bind();
        for (size_t i = 0; i < _models.size(); i++) {
            if(i == 3 || i == 0 || i == 6) {
                _models[i].draw(true, true, _pipeline_tessellation._shader_program, true, true, true);
            }
        } 

    }

    {
        glDepthFunc(GL_LEQUAL); // Wichtig: Skybox sollte im Hintergrund gerendert werden
        glDepthMask(GL_FALSE);
        glDisable(GL_BLEND); // Kein Blending für die Skybox vollständig opak
        _pipeline_skybox.bind(_postProcess._postProcessFBO);
        _camera.bind_skybox();
        // Cubemap binden
        _hdr.bindEnvCubeMap(_pipeline_skybox._shader_program);
        // Skybox rendern
        Cube skyboxCube(1.0f); // 1x1 Würfel
        renderCube(skyboxCube); // Nutzt den Skybox-Würfel direkt
        glEnable(GL_BLEND); // Blending wieder aktivieren
        glDepthMask(GL_TRUE); // Tiefe wieder beschreiben
        glDepthFunc(GL_LESS); // Standard-Tiefe zurücksetzen
        
   }  

        
        // Post-Processing
        // Auf Standard-Framebuffer zurückwechseln
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, _window_width, _window_height);
        //glClear(GL_COLOR_BUFFER_BIT);
        // Post-Processing Shader aktivieren
/*         _pipeline_post_process.bind();

        // Textur mit dem gerenderten Bild binden
        _postProcess.bind(_pipeline_post_process._shader_program);   */

        _pipeline_motion_blur.bind();
        // Textur mit dem gerenderten Bild binden und Motion Vectors
        _postProcess.bind(_pipeline_motion_blur._shader_program);
        // Fullscreen-Quad rendern
        renderFullscreenQuad();
        
        
        //ImGui
        //No gamma correction for the UI(otherwise imGui will be double corrected) 
        glDisable(GL_FRAMEBUFFER_SRGB);
        _imGuiManager.render();
        // present drawn image to screen
        SDL_GL_SwapWindow(_window._window_p);
        //input clear
        Input::flush();
        glEnable(GL_FRAMEBUFFER_SRGB); 
    }

    GLuint _window_width = 1920;
    GLuint _window_height = 1080;
    Window _window;
    Camera _camera;
    HDR _hdr;
    Pipeline _pipeline;
    Pipeline _pipeline_shadows;
    Pipeline _pipeline_pbr; 
    Pipeline _pipeline_equirectangular;
    Pipeline _pipeline_irradiance;
    Pipeline _pipeline_skybox;
    Pipeline _pipeline_prefilter_env;
    Pipeline _pipeline_brdf_lut;
    Pipeline _pipeline_tessellation;
    Pipeline _pipeline_post_process;
    Pipeline _pipeline_motion_blur_vectors;
    Pipeline _pipeline_motion_blur;

    std::array<Light, 3> _lights;
    std::vector<Model> _models;
    // other
    bool _shadows_dirty = false;
    bool _mouse_captured = false;
    // audio
    struct AudioFile {
        void init() {}
        void destroy() {}
        SDL_AudioSpec spec;
        Uint8* buffer;
        Uint32 buffer_size;
    };
    AudioFile audio_file;
    SDL_AudioStream* audio_stream;
    ImGuiManager _imGuiManager;
    //Post Processing
    PostProcess _postProcess;


};