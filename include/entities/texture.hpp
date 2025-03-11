#pragma once
#include <fmt/base.h>
#include <glbinding/gl46core/gl.h>
using namespace gl46core;
#include <stb_image.h>
#include <tinyexr.h>

struct Texture {
    void init(const char* path, bool isHeightMap = false) {
        // load image
        int width, height, channel_count; // output for stbi_load_from_memory
        int desired_channels = isHeightMap ? 1 : 4;
        stbi_uc* image_p = stbi_load(path, &width, &height, &channel_count, desired_channels); // explicitly ask for 4 channels
        if (image_p == nullptr) fmt::println("Failed to load texture: {}", path);
        // create texture to store image in (texture is gpu buffer)
        GLenum internalFormat = isHeightMap ? GL_R16F : GL_RGBA8; // Use higher precision for height maps
        GLenum format = isHeightMap ? GL_RED : GL_RGBA; // Match format to texture type
        glCreateTextures(GL_TEXTURE_2D, 1, &_texture);
        glTextureStorage2D(_texture, 4, internalFormat, width, height);
        glTextureSubImage2D(_texture, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, image_p);
        // free image on cpu side
        stbi_image_free(image_p);
        // sampler parameters
        glTextureParameteri(_texture, GL_TEXTURE_WRAP_S, GL_REPEAT); // s is the u coordinate (width) or GL_CLAMP_TO_EDGE
        glTextureParameteri(_texture, GL_TEXTURE_WRAP_T, GL_REPEAT); // t is the v coordinate (height) or GL_CLAMP_TO_EDGE
        glTextureParameteri(_texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // interpolation mode when scaling image down
        glTextureParameteri(_texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // interpolation mode when scaling image up
        // generate mipmap textures
        glGenerateTextureMipmap(_texture);
    }

    std::vector<float> convertToFloat(const stbi_uc* image_p, int width, int height, int channels) {
        std::vector<float> floatData(width * height * channels);
        for (int i = 0; i < width * height * channels; ++i) {
            floatData[i] = image_p[i] / 255.0f; // Normalisierung auf [0, 1]
        }
        return floatData;
    }

    //More configurable texture loading
    void initPBRTexture(const char* path, GLenum internalFormat, GLenum format, GLenum wrapMode, int desired_channels, bool generateMipmaps) {
        int width, height, channel_count;
        stbi_uc* image_p = stbi_load(path, &width, &height, &channel_count, desired_channels);
        if (!image_p) {
            fmt::println("Failed to load texture: {}", path);
        }

        GLint maxTextureSize;
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
        if (width > maxTextureSize || height > maxTextureSize) {
            fmt::println("Texture too large for GPU (max: {}), resizing...", maxTextureSize);
            float scale = std::min((float)maxTextureSize / width, (float)maxTextureSize / height);
            width = (int)(width * scale);
            height = (int)(height * scale);
        }

        int mipmapLevels = generateMipmaps ? (int)std::floor(std::log2(std::max(width, height))) + 1 : 1;

        glCreateTextures(GL_TEXTURE_2D, 1, &_texture);
        try {
            glTextureStorage2D(_texture, mipmapLevels, internalFormat, width, height);
        } catch (...) {
            stbi_image_free(image_p);
            printf("Failed to create texture storage for texture: %s\n", path);
        }
        GLenum type = (internalFormat == GL_RGB16F || internalFormat == GL_RGBA16F) ? GL_FLOAT : GL_UNSIGNED_BYTE;
        if (type == GL_FLOAT) {
            std::vector<float> floatData = convertToFloat(image_p, width, height, channel_count);
            glTextureSubImage2D(_texture, 0, 0, 0, width, height, format, type, floatData.data());
        } else {
            glTextureSubImage2D(_texture, 0, 0, 0, width, height, format, type, image_p);
        }

        if (wrapMode != GL_REPEAT && wrapMode != GL_MIRRORED_REPEAT && wrapMode != GL_CLAMP_TO_EDGE && wrapMode != GL_CLAMP_TO_BORDER) {
            stbi_image_free(image_p);
            printf("Invalid wrap mode: %d\n", wrapMode);
        }

        glTextureParameteri(_texture, GL_TEXTURE_WRAP_S, wrapMode);
        glTextureParameteri(_texture, GL_TEXTURE_WRAP_T, wrapMode);
        glTextureParameteri(_texture, GL_TEXTURE_MIN_FILTER, generateMipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
        glTextureParameteri(_texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if (generateMipmaps) {
            glGenerateTextureMipmap(_texture);
        }

        stbi_image_free(image_p);
    }

    void initHDRTexture(const char* path) {
        // Dateiformat erkennen
        std::string filePath = path;
        bool isHDR = (filePath.substr(filePath.find_last_of(".") + 1) == "hdr");
        bool isEXR = (filePath.substr(filePath.find_last_of(".") + 1) == "exr");

        float* image_p = nullptr; // Speicher für Bilddaten
        int width = 0, height = 0, channel_count = 3; // Standard: RGB-Kanäle

        if (isHDR) {
            // HDR-Textur mit stb_image laden
            stbi_set_flip_vertically_on_load(true); // Wichtig für korrekte Ausrichtung
            image_p = stbi_loadf(path, &width, &height, &channel_count, 0);
            if (image_p == nullptr) {
                fmt::println("Failed to load HDR texture: {}", path);
                fmt::println("Reason: {}", stbi_failure_reason());
                return;
            }
        } else if (isEXR) {
            printf("Loading EXR texture: %s\n", path);
            // EXR-Textur mit TinyEXR laden
            const char* error = nullptr;
            int ret = LoadEXR(&image_p, &width, &height, path, &error);
            if (ret != TINYEXR_SUCCESS) {
                fmt::println("Failed to load EXR texture: {}", path);
                if (error) {
                    fmt::println("Reason: {}", error);
                    FreeEXRErrorMessage(error);
                }
                return;
            }
        } else {
            fmt::println("Unsupported texture format: {}", path);
            return;
        }

        fmt::println("Successfully loaded texture: {} ({}x{})", path, width, height);

        // OpenGL-Textur erstellen
        glCreateTextures(GL_TEXTURE_2D, 1, &_texture);
        glBindTexture(GL_TEXTURE_2D, _texture);

        // Speicher für HDR-Daten auf der GPU reservieren
        //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, image_p);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, image_p);
        // Sampler-Parameter setzen
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Speicher freigeben
        if (isHDR) {
            stbi_image_free(image_p);
        } else if (isEXR) {
            free(image_p);
        }

        fmt::println("HDR texture uploaded to GPU.");
    }


    void destroy() {
        glDeleteTextures(1, &_texture);
    }
    void bind() {
        glBindTextureUnit(0, _texture);
    }
    void bindPBR(GLuint program_id) {
        glActiveTexture(GL_TEXTURE8);
        glBindTexture(GL_TEXTURE_2D, _texture);
        glUniform1i(glGetUniformLocation(program_id, "tex_diffuse"), 8);
    }

    GLuint _texture = 0;
};