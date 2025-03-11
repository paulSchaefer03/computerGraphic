#pragma once
#include <cstdint>
#include <fmt/base.h>
#include <map>
#include "texture.hpp" // Include the header file where Texture is defined

struct Material {
    
    void bind(GLuint program_id, bool PBR) {

        glUniform1f(17, _texture_contribution);
        glUniform1f(18, _specular);
        glUniform1f(19, _specular_shininess);

        if (_albedo_map != 0) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, _albedo_map);
            glUniform1i(glGetUniformLocation(program_id, "albedoMap"), 0);
        }
        if (_normal_map != 0) {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, _normal_map);
            glUniform1i(glGetUniformLocation(program_id, "normalMap"), 1);
        }
        if (_metallic_map != 0) {
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, _metallic_map);
            glUniform1i(glGetUniformLocation(program_id, "metallicMap"), 2);
        }
        if (_roughness_map != 0) {
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, _roughness_map);
            glUniform1i(glGetUniformLocation(program_id, "roughnessMap"), 3);
        }
        if (_ao_map != 0) {
            glActiveTexture(GL_TEXTURE4);
            glBindTexture(GL_TEXTURE_2D, _ao_map);
            glUniform1i(glGetUniformLocation(program_id, "aoMap"), 4);
        }
        if (_height_map != 0) {
            glActiveTexture(GL_TEXTURE9);
            glBindTexture(GL_TEXTURE_2D, _height_map);
            glUniform1i(glGetUniformLocation(program_id, "heightMap"), 9);
        }
        
        glUniform1ui(glGetUniformLocation(program_id, "materialFlags"), _materialFlags);
        glUniform1f(glGetUniformLocation(program_id, "heightScale"), _heightScale);

    }

    void useCustomMetallic(GLuint program_id, float metallic) {
        _metallic = metallic;
        _materialFlags |= MaterialFlags::CUSTOM_METALLIC;
        glUniform1f(glGetUniformLocation(program_id, "customMetallicValue"), _metallic);
    }

    void useCustomRoughness(GLuint program_id, float roughness) {
        _roughness = roughness;
        _materialFlags |= MaterialFlags::CUSTOM_ROUGHNESS;
        glUniform1f(glGetUniformLocation(program_id, "customRoughnessValue"), _roughness);
    }


    enum MaterialFlags {
        HAS_ALBEDO_MAP    = 1 << 0, // 0001
        HAS_NORMAL_MAP    = 1 << 1, // 0010
        HAS_METALLIC_MAP  = 1 << 2, // 0100
        HAS_ROUGHNESS_MAP = 1 << 3, // 1000
        HAS_AO_MAP        = 1 << 4, // 10000
        NO_PBR_TEXTURES   = 1 << 5, // 100000
        CUSTOM_METALLIC   = 1 << 6, // 1000000
        CUSTOM_ROUGHNESS  = 1 << 7,  // 10000000
        HAS_HEIGHT_MAP    = 1 << 8  // 100000000
    };

    float _texture_contribution = 0;
    float _specular = 1;
    float _specular_shininess = 32;

    float _metallic = 0.5;
    float _roughness = 0.5;
    float _heightScale = 0.05;
    // PBR-Texturen
    uint32_t _materialFlags = 0; //Default: 0 d.h. keine PBR-Texturen
    GLuint _albedo_map = 0;
    GLuint _normal_map = 0;
    GLuint _metallic_map = 0;
    GLuint _roughness_map = 0;
    GLuint _ao_map = 0;
    GLuint _height_map = 0;
    std::vector<std::map<std::string, Texture>> _texturesPBR;

};