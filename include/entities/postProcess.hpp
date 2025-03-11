#pragma once
#include <glbinding/gl46core/gl.h>
using namespace gl46core;
#include <glm/glm.hpp>
#include <fmt/core.h>

struct PostProcess {
    void init(GLuint window_width, GLuint window_height) {
        glGenFramebuffers(1, &_postProcessFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, _postProcessFBO);
    
        glGenTextures(2, _postProcessTextures);
        for (unsigned int i = 0; i < 2; i++) {
            glBindTexture(GL_TEXTURE_2D, _postProcessTextures[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, window_width, window_height, 0, GL_RGBA, GL_FLOAT, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, _postProcessTextures[i], 0);
        }
    
        // Tiefenpuffer für normales Rendering
        GLuint rbo;
        glGenRenderbuffers(1, &rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, window_width, window_height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);
    
        // Wir haben zwei Render-Targets
        GLenum attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
        glDrawBuffers(2, attachments);
    
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            fmt::print("PostProcessing Framebuffer ist nicht vollständig!\n");
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void initPingPongBuffer(GLuint window_width, GLuint window_height) {
        glGenFramebuffers(2, _pingpongFBO);
        glGenTextures(2, _pingpongColorBuffers);
        for (unsigned int i = 0; i < 2; i++) {
            glBindFramebuffer(GL_FRAMEBUFFER, _pingpongFBO[i]);
            glBindTexture(GL_TEXTURE_2D, _pingpongColorBuffers[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, window_width, window_height, 0, GL_RGBA, GL_FLOAT, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _pingpongColorBuffers[i], 0);
        }
    }
    

    void initMotionVectorBuffer(GLuint window_width, GLuint window_height) {
        glGenFramebuffers(1, &_motionVectorFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, _motionVectorFBO);
    
        // Farbpuffer für das normale Bild
        glGenTextures(1, &_motionVectorTexture);
        glBindTexture(GL_TEXTURE_2D, _motionVectorTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, window_width, window_height, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _motionVectorTexture, 0);
    
        // Zweiter Puffer für Motion Vectors
        glGenTextures(1, &_motionVectorTexture);
        glBindTexture(GL_TEXTURE_2D, _motionVectorTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, window_width, window_height, 0, GL_RG, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, _motionVectorTexture, 0);
    
        // Definiere, welche Attachments genutzt werden
        GLenum attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
        glDrawBuffers(2, attachments);
    
        // Überprüfen, ob der Framebuffer vollständig ist
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            fmt::print("Motion Vector Framebuffer nicht vollständig!\n");
        }
    
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void bind(GLuint program_id){

        // Textur mit dem gerenderten Bild binden
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, _postProcessTextures[0]);
        glUniform1i(glGetUniformLocation(program_id, "screenTexture"), 0);
        // Textur mit Motion Vectors binden
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, _motionVectorTexture);
        glUniform1i(glGetUniformLocation(program_id, "motionVectorTexture"), 1);
        // Textur mit dem finalen Bloom-Effekt binden
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, _finalBloomTexture);
        glUniform1i(glGetUniformLocation(program_id, "bloomBlur"), 2);

        glUniform1f(glGetUniformLocation(program_id, "motionBlurStrength"), _motionBlurStrength);
        glUniform1f(glGetUniformLocation(program_id, "motionBlurSamples"), _motionBlurSamples);
        glUniform1f(glGetUniformLocation(program_id, "exposure"), _exposure);
        glUniform1f(glGetUniformLocation(program_id, "gamma"), _gamma);

    }

    void destroy() {

    }

    // Post Processing
    GLuint _depthTexture;
    GLuint _postProcessFBO;
    GLuint _postProcessTextures[2];
    GLuint _pingpongFBO[2], _pingpongColorBuffers[2];
    GLuint _finalBloomTexture;
    GLuint _postProcessRBO;
    GLuint _motionVectorFBO;
    GLuint _motionVectorTexture;
    float _aberrationStrength = 0.002;
    float _motionBlurStrength = 0.00125;
    float _motionBlurSamples = 8;
    float _exposure = 1.0;
    float _gamma = 1.0;

};