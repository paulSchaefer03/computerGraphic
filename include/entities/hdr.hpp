#pragma once
#include <glbinding/gl46core/gl.h>
using namespace gl46core;
#include <glm/glm.hpp>
#include "texture.hpp"
#include "cube.hpp"

struct HDR {

    void init(const char* path) {
        //HDR Texture laden
        _hdr.initHDRTexture(path);
        //Setup framebuffer
        glGenFramebuffers(1, &_captureFBO);
        glGenRenderbuffers(1, &_captureRBO);

        glBindFramebuffer(GL_FRAMEBUFFER, _captureFBO);
        glBindRenderbuffer(GL_RENDERBUFFER, _captureRBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, _captureResolution, _captureResolution);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _captureRBO);

        // Create environment cubemap
        glGenTextures(1, &_envCubeMap);
        glBindTexture(GL_TEXTURE_CUBE_MAP, _envCubeMap);
        for (unsigned int i = 0; i < 6; ++i) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F,
                        _captureResolution, _captureResolution, 0, GL_RGB, GL_FLOAT, nullptr);
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Projection and view matrices
        _captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
        _captureViews = {
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))
        };

    }

    void bindAndCreateEnvMap(GLuint program_id){
        glUniform1i(glGetUniformLocation(program_id, "equirectangularMap"), 0);
        glUniformMatrix4fv(glGetUniformLocation(program_id, "projection"), 1, GL_FALSE, &_captureProjection[0][0]);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, _hdr._texture);

        glViewport(0, 0, _captureResolution, _captureResolution);
        glBindFramebuffer(GL_FRAMEBUFFER, _captureFBO);
        for (unsigned int i = 0; i < 6; ++i) {
            glUniformMatrix4fv(glGetUniformLocation(program_id, "view"), 1, GL_FALSE, &_captureViews[i][0][0]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, _envCubeMap, 0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            Cube cube(1.0);
            renderCube(cube);
        }
        printf("Environment Map created\n");
    }
    //Irradiance Map erstellen wichtig zuerst bindAndCreateEnvMap aufrufen
    void bindAndCreateIrradianceMap(GLuint program_id){
        glGenTextures(1, &_irradianceMap);
        glBindTexture(GL_TEXTURE_CUBE_MAP, _irradianceMap);
        for (unsigned int i = 0; i < 6; ++i) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F,
                        _irradianceResolution, _irradianceResolution, 0, GL_RGB, GL_FLOAT, nullptr);
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glUniform1i(glGetUniformLocation(program_id, "environmentMap"), 0);
        glUniformMatrix4fv(glGetUniformLocation(program_id, "projection"), 1, GL_FALSE, &_captureProjection[0][0]);

        glBindTexture(GL_TEXTURE_CUBE_MAP, _envCubeMap);
        glViewport(0, 0, _irradianceResolution, _irradianceResolution);
        glBindFramebuffer(GL_FRAMEBUFFER, _captureFBO);
        for (unsigned int i = 0; i < 6; ++i) {
            glUniformMatrix4fv(glGetUniformLocation(program_id, "view"), 1, GL_FALSE, &_captureViews[i][0][0]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, _irradianceMap, 0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            Cube cube(1.0);
            renderCube(cube);
        }
        printf("Irradiance Map created\n");
        glBindFramebuffer(GL_FRAMEBUFFER, 0); 

        // Clean up
        glDeleteFramebuffers(1, &_captureFBO);
        glDeleteRenderbuffers(1, &_captureRBO);
    }

    void bindIrrandianceMap(GLuint program_id){
        glActiveTexture(GL_TEXTURE5); // Textur-Einheit 5 reservieren
        glBindTexture(GL_TEXTURE_CUBE_MAP, _irradianceMap); // Die Irradiance Map binden
        glUniform1i(glGetUniformLocation(program_id, "irradianceMap"), 5);  
    }

    void destroy() {
        glDeleteTextures(1, &_envCubeMap);
        glDeleteTextures(1, &_irradianceMap);
        glDeleteTextures(1, &_prefilter_env_map);
        glDeleteTextures(1, &_brdf_lut_texture);
        glDeleteTextures(1, &_corrected_brdf_lut_texture);
        glDeleteFramebuffers(1, &_captureFBO);
        glDeleteRenderbuffers(1, &_captureRBO);
    }

    // Pre-Filtered Environment Map (Cube Map mit Mipmaps)
    void createPrefilterEnvMap(GLuint program_id) {
        glGenFramebuffers(1, &_captureFBO);
        glGenRenderbuffers(1, &_captureRBO);
        glGenTextures(1, &_prefilter_env_map);
        glBindTexture(GL_TEXTURE_CUBE_MAP, _prefilter_env_map);
    
        const unsigned int baseResolution = 128;
        const unsigned int maxMipLevels = 5;
    
        for (unsigned int i = 0; i < 6; ++i) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, baseResolution, baseResolution, 0, GL_RGB, GL_FLOAT, nullptr);
        }
    
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    
        glUseProgram(program_id);
        glUniformMatrix4fv(glGetUniformLocation(program_id, "projection"), 1, GL_FALSE, &_captureProjection[0][0]);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, _envCubeMap);
    
        glBindFramebuffer(GL_FRAMEBUFFER, _captureFBO);
        Cube cube(1.0f);
    
        for (unsigned int mip = 0; mip < maxMipLevels; ++mip) {
            unsigned int mipWidth = baseResolution * std::pow(0.5, mip);
            unsigned int mipHeight = baseResolution * std::pow(0.5, mip);
            glBindRenderbuffer(GL_RENDERBUFFER, _captureRBO);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
            glViewport(0, 0, mipWidth, mipHeight);
    
            float roughness = (float)mip / (float)(maxMipLevels - 1);
            //Nicht zwangsweise nötig für die prefilterEnvMap
            //glUniform1f(glGetUniformLocation(program_id, "roughness"), roughness);
    
            for (unsigned int i = 0; i < 6; ++i) {
                glUniformMatrix4fv(glGetUniformLocation(program_id, "view"), 1, GL_FALSE, &_captureViews[i][0][0]);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, _prefilter_env_map, mip);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                renderCube(cube);
            }
        }
    
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        printf("Pre-Filtered Environment Map created\n");

    }
    //BRDF LUT 2D Texture
    void ceateBRDFLUT() {
        glGenTextures(1, &_brdf_lut_texture);
        glBindTexture(GL_TEXTURE_2D, _brdf_lut_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 1024, 1024, 0, GL_RG, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
        glBindFramebuffer(GL_FRAMEBUFFER, _captureFBO);
        glBindRenderbuffer(GL_RENDERBUFFER, _captureRBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 1024, 1024);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _brdf_lut_texture, 0);

        glViewport(0, 0, 1024, 1024);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        Cube cube(1.0f);
        renderCube(cube);

        glBindFramebuffer(GL_FRAMEBUFFER, 0); 
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            printf("Framebuffer-Status ist nicht vollständig: 0x%x\n", status);
        }

        // BRDF-LUT-Daten abrufen (und Textur verkleinern, weil sie unabhänge der Auflösung immer einen Schwarzen Rand hat (kein Plan warum))
        int width = 1024; // oder die tatsächliche Breite der BRDF-LUT
        int height = 1024; // oder die tatsächliche Höhe der BRDF-LUT
   
        glGenTextures(1, &_corrected_brdf_lut_texture);
        glBindTexture(GL_TEXTURE_2D, _corrected_brdf_lut_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
 
        // Alte Textur auslesen
        int srcWidth = 1024, srcHeight = 1024;
        std::vector<float> oldData(srcWidth * srcHeight * 2);
        glBindTexture(GL_TEXTURE_2D, _brdf_lut_texture);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RG, GL_FLOAT, oldData.data());

        // Pixel extrahieren(die Korrekt sind)
        int dstWidth = 512, dstHeight = 512;
        std::vector<float> newData(dstWidth * dstHeight * 2);

        for (int y = 0; y < dstHeight; y++) {
            for (int x = 0; x < dstWidth; x++) {
                int srcX = x + 256; // Offset
                int srcY = y + 256;
                int srcIndex = (srcY * srcWidth + srcX) * 2;
                int dstIndex = (y * dstWidth + x) * 2;

                newData[dstIndex] = oldData[srcIndex];
                newData[dstIndex + 1] = oldData[srcIndex + 1];
            }
        }

        // Neue Pixel in die neue Textur hochladen
        glBindTexture(GL_TEXTURE_2D, _corrected_brdf_lut_texture);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, dstWidth, dstHeight, GL_RG, GL_FLOAT, newData.data());
      
        // Alte Textur löschen
        glDeleteTextures(1, &_brdf_lut_texture);

        // Neue Textur prüfen
        checkPreComputeMap(_corrected_brdf_lut_texture, "BRDF LUT", 512, 512, 2);
    }

    void bindPrefilterEnvMap(GLuint program_id){
        glActiveTexture(GL_TEXTURE6); // Textur-Einheit 6 reservieren
        glBindTexture(GL_TEXTURE_CUBE_MAP, _prefilter_env_map); // Die Pre-Filtered Environment Map binden
        glUniform1i(glGetUniformLocation(program_id, "prefilterMap"), 6);  
    }

    void bindBRDFLUT(GLuint program_id){
        glActiveTexture(GL_TEXTURE7); // Textur-Einheit 7 reservieren
        glBindTexture(GL_TEXTURE_2D, _corrected_brdf_lut_texture); // Die BRDF LUT binden
        glUniform1i(glGetUniformLocation(program_id, "brdfLUT"), 7);  
    }

    void bindEnvCubeMap(GLuint program_id){
        glActiveTexture(GL_TEXTURE0); // Textur-Einheit 0 reservieren
        glBindTexture(GL_TEXTURE_CUBE_MAP, _envCubeMap); // Die Environment Map binden
        glUniform1i(glGetUniformLocation(program_id, "skybox"), 0);  
    }

    //Optional but good to have since the maps are precomputed and there where serveral bugs in the past
    void checkPreComputeMap(GLuint textureID, const char* name, int width, int height, int channels) {

        // Textur auslesen
        std::vector<float> testData(width * height * channels);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glGetTexImage(GL_TEXTURE_2D, 0, (channels == 4 ? GL_RGBA : (channels == 3 ? GL_RGB : GL_RG)), GL_FLOAT, testData.data());

        int counterBlack = 0;
        int counterNotBlack = 0;
        // Prüfen und Werte in der Konsole ausgeben
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
            bool isBlack = true;
            for (int c = 0; c < channels; c++) {
                int index = (y * width + x) * channels + c;
                if (testData[index] != 0.0f) {
                isBlack = false;
                break;
                }
            }
            if (isBlack) {
                counterBlack++;
            } else {
                counterNotBlack++;
            }
            }
        }

        printf("Schwarze Pixel in %s: %d\n", name, counterBlack);
        printf("Nicht-schwarze Pixel in %s: %d\n", name, counterNotBlack);
    
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        printf("%s created.\n", name);
        
    }

    Texture _hdr;
    GLuint _envCubeMap;
    GLuint _irradianceMap;
    glm::mat4 _captureProjection;
    std::array<glm::mat4, 6> _captureViews;
    const unsigned int _captureResolution = 512;
    const unsigned int _irradianceResolution = 32;
    GLuint _captureFBO, _captureRBO;

    //Pre-Filtered Environment Map and BRDF LUT
    GLuint _prefilter_env_map;
    GLuint _brdf_lut_texture;
    GLuint _corrected_brdf_lut_texture;
};