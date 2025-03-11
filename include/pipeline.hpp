#pragma once
#include <fstream>
#include <fmt/base.h>
#include <glbinding/gl46core/gl.h>
using namespace gl46core;

struct Pipeline {
    void init(const char* vs_path, const char* fs_path, const char* tcs_path = nullptr, const char* tes_path = nullptr) {
        // Compile Vertex Shader
        GLuint vertex_shader = compile_shader(vs_path, GL_VERTEX_SHADER);

        // Compile Tessellation Control Shader (optional)
        GLuint tess_control_shader = 0;
        if (tcs_path) {
            tess_control_shader = compile_shader(tcs_path, GL_TESS_CONTROL_SHADER);
        }

        // Compile Tessellation Evaluation Shader (optional)
        GLuint tess_eval_shader = 0;
        if (tes_path) {
            tess_eval_shader = compile_shader(tes_path, GL_TESS_EVALUATION_SHADER);
        }

        // Compile Fragment Shader
        GLuint fragment_shader = compile_shader(fs_path, GL_FRAGMENT_SHADER);

        // Link Shader Program
        _shader_program = glCreateProgram();
        glAttachShader(_shader_program, vertex_shader);
        if (tcs_path) glAttachShader(_shader_program, tess_control_shader);
        if (tes_path) glAttachShader(_shader_program, tess_eval_shader);
        glAttachShader(_shader_program, fragment_shader);
        glLinkProgram(_shader_program);

        // Check Linking Status
        GLint success;
        glGetProgramiv(_shader_program, GL_LINK_STATUS, &success);
        if (!success) {
            std::vector<GLchar> info_log(512);
            glGetProgramInfoLog(_shader_program, info_log.size(), nullptr, info_log.data());
            fmt::print("Program Linking Error: {}", info_log.data());
        }

        // Clean Up
        glDeleteShader(vertex_shader);
        if (tcs_path) glDeleteShader(tess_control_shader);
        if (tes_path) glDeleteShader(tess_eval_shader);
        glDeleteShader(fragment_shader);
    }
    void create_framebuffer() {
        // create frame buffer for shadow mapping pipeline
        glCreateFramebuffers(1, &_framebuffer);
        // attach texture to frame buffer (only draw to depth, no color output -> GL_NONE)
        glNamedFramebufferReadBuffer(_framebuffer, GL_NONE);
        glNamedFramebufferDrawBuffer(_framebuffer, GL_NONE);
    }

    GLuint compile_shader(const char* path, GLenum shader_type) {
        // Read Shader Source
        std::ifstream shader_file(path, std::ios::binary);
        shader_file.seekg(0, std::ios::end);
        GLint size = shader_file.tellg();
        shader_file.seekg(0, std::ios::beg);
        std::vector<GLchar> source(size);
        shader_file.read(source.data(), size);

        // Compile Shader
        GLuint shader = glCreateShader(shader_type);
        const GLchar* source_ptr = source.data();
        glShaderSource(shader, 1, &source_ptr, &size);
        glCompileShader(shader);

        // Check Compilation Status
        GLint success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            std::vector<GLchar> info_log(512);
            glGetShaderInfoLog(shader, info_log.size(), nullptr, info_log.data());
            fmt::print("Shader Compilation Error ({}): {}", path, info_log.data());
        }

        return shader;
    }

    void bind(GLuint framebuffer = 0) {
        if (framebuffer) glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        else glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);
        glUseProgram(_shader_program);
    }

    GLuint _shader_program;
    GLuint _framebuffer = 0;
};
