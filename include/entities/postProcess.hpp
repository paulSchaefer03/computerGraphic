#pragma once
#include <glbinding/gl46core/gl.h>
using namespace gl46core;
#include <glm/glm.hpp>
#include <fmt/core.h>

struct PostProcess {
    void init(GLuint window_width, GLuint window_height) {
        // Framebuffer für Post-Processing erstellen
        glGenFramebuffers(1, &_postProcessFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, _postProcessFBO);

        // Farb-Textur für den Framebuffer
        glGenTextures(1, &_postProcessTexture);
        glBindTexture(GL_TEXTURE_2D, _postProcessTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, window_width, window_height, 0, GL_RGB, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _postProcessTexture, 0);

        glBindFramebuffer(GL_FRAMEBUFFER, _postProcessFBO);
        glGenTextures(1, &_depthTexture);
        glBindTexture(GL_TEXTURE_2D, _depthTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, window_width, window_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _depthTexture, 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
            fmt::print("Post-Processing Framebuffer nicht vollständig!");
        }
        GLint attachedDepth;
        glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &attachedDepth);
        fmt::print("Depth-Attachment: {}\n", attachedDepth == 0 ? "None" : "Depth-Texture");    

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
        glBindTexture(GL_TEXTURE_2D, _postProcessTexture);
        glUniform1i(glGetUniformLocation(program_id, "screenTexture"), 0);
        glUniform1f(glGetUniformLocation(program_id, "aberrationStrength"), _aberrationStrength);
        // Textur mit Motion Vectors binden
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, _motionVectorTexture);
        glUniform1i(glGetUniformLocation(program_id, "motionVectorTexture"), 1);

    }

    void destroy() {

    }

    // Post Processing
    GLuint _depthTexture;
    GLuint _postProcessFBO;
    GLuint _postProcessTexture;
    GLuint _postProcessRBO;
    GLuint _motionVectorFBO;
    GLuint _motionVectorTexture;
    float _aberrationStrength = 0.002;

};