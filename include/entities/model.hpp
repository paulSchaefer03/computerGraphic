#pragma once
#include <fmt/base.h>
#include <filesystem>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/material.h>
#include "transform.hpp"
#include "material.hpp"
#include "texture.hpp"
#include "mesh.hpp"

struct Model {
    void init(Mesh::Primitive primitive, const float size, const uint32_t eSphereStacks = 32, const uint32_t eSphereSlices = 32) {
        _meshes.emplace_back();
        switch (primitive) {
            case Mesh::eCube: _meshes.front().init(); break;
            case Mesh::eSphere: _meshes.front().initESphere(eSphereStacks, eSphereSlices); break;
            case Mesh::cubeSphere: _meshes.front().initCubeSphere(size, 7); break;
            case Mesh::icoSphere: _meshes.front().initIcoSphere(size, 5); break;
            case Mesh::cube: _meshes.front().initCube(size); break;
            case Mesh::octahedron: _meshes.front().initOctahedron(size); break; 
        }
        _materials.emplace_back()._texture_contribution = 0.0;
    }
    void init(Mesh::Primitive primitive, const float size, const char* texture_path, const uint32_t eSphereStacks = 32, const uint32_t eSphereSlices = 32) {
        _meshes.emplace_back();
        switch (primitive) {
            case Mesh::eCube: _meshes.front().init(); break;
            case Mesh::eSphere: _meshes.front().initESphere(eSphereStacks, eSphereSlices); break;
            case Mesh::cubeSphere: _meshes.front().initCubeSphere(size, 7); break;
            case Mesh::icoSphere: _meshes.front().initIcoSphere(size, 5); break;
            case Mesh::cube: _meshes.front().initCube(size); break;
            case Mesh::octahedron: _meshes.front().initOctahedron(size); break; 
        }
        _textures.emplace_back().init(texture_path);
        _materials.emplace_back()._texture_contribution = 1.0;
    }
    void initPBR(Mesh::Primitive primitive, const float size, const char* albedo_path = nullptr, const char* normal_path = nullptr,
        const char* metallic_path = nullptr, const char* roughness_path = nullptr, const char* ao_path = nullptr, const char* height_path = nullptr, 
        const uint32_t eSphereStacks = 32, const uint32_t eSphereSlices = 32) {
        _meshes.emplace_back();
        switch (primitive) {
            case Mesh::eCube: _meshes.front().init(); break;
            case Mesh::eSphere: _meshes.front().initESphere(eSphereStacks, eSphereSlices); break;
            case Mesh::cubeSphere: _meshes.front().initCubeSphere(size, 7); break;
            case Mesh::icoSphere: _meshes.front().initIcoSphere(size, 5); break;
            case Mesh::cube: _meshes.front().initCube(size); break;
            case Mesh::octahedron: _meshes.front().initOctahedron(size); break; 
        }
        Material pbr_material;
        Texture albedo, normal, metallic, roughness, ao, height;

        if(ao_path != nullptr)       {ao.init(ao_path);}
        else                         {ao._texture = 0;}
        if(albedo_path != nullptr)   {albedo.initPBRTexture(albedo_path, GL_SRGB8_ALPHA8, GL_RGBA, GL_REPEAT, 4, true);}
        else                         {albedo._texture = 0;}
        if(normal_path != nullptr)   {printf("Test"); normal.initPBRTexture(normal_path, GL_RGB8, GL_RGB, GL_REPEAT, 3, true);}
        else                         {normal._texture = 0;}
        if(metallic_path != nullptr) {metallic.init(metallic_path);}
        else                         {metallic._texture = 0;}
        if(roughness_path != nullptr){roughness.init(roughness_path);}
        else                         {roughness._texture = 0;}
        if(height_path != nullptr)   {height.init(height_path, true);}
        else                         {height._texture = 0;}

        pbr_material._albedo_map = albedo._texture;
        pbr_material._normal_map = normal._texture;
        pbr_material._metallic_map = metallic._texture;
        pbr_material._roughness_map = roughness._texture;
        pbr_material._ao_map = ao._texture;
        pbr_material._height_map = height._texture;

        _materials.push_back(pbr_material);
        if(ao_path != nullptr){
            _materials.back()._materialFlags |= Material::HAS_AO_MAP;
        }
        if(albedo_path != nullptr){
            _materials.back()._materialFlags |= Material::HAS_ALBEDO_MAP;
        }
        if(normal_path != nullptr){
            _materials.back()._materialFlags |= Material::HAS_NORMAL_MAP;
        }
        if(metallic_path != nullptr){
            _materials.back()._materialFlags |= Material::HAS_METALLIC_MAP;
        }
        if(roughness_path != nullptr){
            _materials.back()._materialFlags |= Material::HAS_ROUGHNESS_MAP;
        }
        if(height_path != nullptr){
            _materials.back()._materialFlags |= Material::HAS_HEIGHT_MAP;
        }
    }

    void init(std::string model_path) {
        Assimp::Importer importer;

        // flags that allow some automatic post processing of model
        unsigned int flags = 0; // https://assimp.sourceforge.net/lib_html/postprocess_8h.html
        flags |= aiProcess_Triangulate; // triangulate all faces if not already triangulated
        flags |= aiProcess_GenNormals; // generate normals if they dont exist
        flags |= aiProcess_FlipUVs; // OpenGL prefers flipped y axis
        flags |= aiProcess_PreTransformVertices; // simplifies model load

        // load the entire "scene" (may be multiple meshes, hence scene)
        const aiScene* scene_p = importer.ReadFile(model_path, flags);
        if (scene_p == nullptr) {
            fmt::println("{}", importer.GetErrorString());
            return;
        }
        fmt::println("Loading model: {}", model_path);

        // figure out path to the model root for stuff like .obj, which puts its assets into sub-folders
        size_t separator_index = model_path.find_last_of('/');
        std::string model_root = model_path.substr(0, separator_index + 1);
        fmt::println("Model root: {}", model_root);

        // create materials and textures
        _materials.resize(scene_p->mNumMaterials);
        _textures.resize(scene_p->mNumMaterials);
        for (uint32_t i = 0; i < scene_p->mNumMaterials; i++) {
            aiMaterial* material_p = scene_p->mMaterials[i];
            Material& material = _materials[i];

            // load basic material properties
            material_p->Get(AI_MATKEY_SHININESS, material._specular);
            material_p->Get(AI_MATKEY_SHININESS_STRENGTH, material._specular_shininess);

            // load lighting colors
            aiColor3D color;
            material_p->Get(AI_MATKEY_COLOR_AMBIENT, color);
            material_p->Get(AI_MATKEY_COLOR_DIFFUSE, color);
            material_p->Get(AI_MATKEY_COLOR_SPECULAR, color);

            // see if this should use a diffuse texture
            if (material_p->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
                material._texture_contribution = 1.0;
                // load the texture
                aiString texture_path;
                material_p->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), texture_path);
                std::string full_texture_path = model_root;
                full_texture_path.append(texture_path.C_Str());
                _textures[i].init(full_texture_path.c_str());
            }
        }
        
        // create meshes
        _meshes.resize(scene_p->mNumMeshes);
        for (uint32_t i = 0; i < scene_p->mNumMeshes; i++) {
            aiMesh* mesh_p = scene_p->mMeshes[i];
            _meshes[i].init(mesh_p);
        }
    }

    void initPBR(std::string model_path) {
        Assimp::Importer importer;

        // flags that allow some automatic post processing of model
        unsigned int flags = 0; // https://assimp.sourceforge.net/lib_html/postprocess_8h.html
        flags |= aiProcess_Triangulate; // triangulate all faces if not already triangulated
        flags |= aiProcess_GenNormals; // generate normals if they dont exist
        flags |= aiProcess_FlipUVs; // OpenGL prefers flipped y axis
        flags |= aiProcess_PreTransformVertices; // simplifies model load

        // load the entire "scene" (may be multiple meshes, hence scene)
        const aiScene* scene_p = importer.ReadFile(model_path, flags);
        if (scene_p == nullptr) {
            fmt::println("{}", importer.GetErrorString());
            return;
        }
        fmt::println("Loading model: {}", model_path);

        // figure out path to the model root for stuff like .obj, which puts its assets into sub-folders
        size_t separator_index = model_path.find_last_of('/');
        std::string model_root = model_path.substr(0, separator_index + 1);
        fmt::println("Model root: {}", model_root);

        // create materials and textures
        _materials.resize(scene_p->mNumMaterials);
        _textures.resize(scene_p->mNumMaterials);
        for (uint32_t i = 0; i < scene_p->mNumMaterials; i++) {
            aiMaterial* material_p = scene_p->mMaterials[i];
            Material& material = _materials[i];

            // load basic material properties
            material_p->Get(AI_MATKEY_SHININESS, material._specular);
            material_p->Get(AI_MATKEY_SHININESS_STRENGTH, material._specular_shininess);

            // load lighting colors
            aiColor3D color;
            material_p->Get(AI_MATKEY_COLOR_AMBIENT, color);
            material_p->Get(AI_MATKEY_COLOR_DIFFUSE, color);
            material_p->Get(AI_MATKEY_COLOR_SPECULAR, color);

            // see if this should use a diffuse texture
            if (material_p->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
                material._texture_contribution = 1.0;
                material._materialFlags |= Material::HAS_ALBEDO_MAP;
                material._materialFlags |= Material::NO_PBR_TEXTURES;
                // load the texture
                aiString texture_path;
                material_p->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), texture_path);
                std::string full_texture_path = model_root;
                full_texture_path.append(texture_path.C_Str());
                _textures[i].init(full_texture_path.c_str());
            }
        }
        
        // create meshes
        _meshes.resize(scene_p->mNumMeshes);
        for (uint32_t i = 0; i < scene_p->mNumMeshes; i++) {
            aiMesh* mesh_p = scene_p->mMeshes[i];
            _meshes[i].init(mesh_p);
        }
    }

    void initPBROLD(std::string model_path) {
        Assimp::Importer importer;

        // flags that allow some automatic post processing of model
        unsigned int flags = 0; // https://assimp.sourceforge.net/lib_html/postprocess_8h.html
        flags |= aiProcess_Triangulate; // triangulate all faces if not already triangulated
        flags |= aiProcess_GenNormals; // generate normals if they dont exist
        flags |= aiProcess_FlipUVs; // OpenGL prefers flipped y axis
        flags |= aiProcess_PreTransformVertices; // simplifies model load

        // load the entire "scene" (may be multiple meshes, hence scene)
        const aiScene* scene_p = importer.ReadFile(model_path, flags);
        if (scene_p == nullptr) {
            fmt::println("{}", importer.GetErrorString());
            return;
        }
        fmt::println("Loading model: {}", model_path);

        // figure out path to the model root for stuff like .obj, which puts its assets into sub-folders
        size_t separator_index = model_path.find_last_of('/');
        std::string model_root = model_path.substr(0, separator_index + 1);
        fmt::println("Model root: {}", model_root);

        // create materials and textures
        _materials.resize(scene_p->mNumMaterials);
        _textures.resize(scene_p->mNumMaterials);
        for (uint32_t i = 0; i < scene_p->mNumMaterials; i++) {
            _materials[i]._texturesPBR.resize(5);
            //mehr wie fünf kann ich zur Zeit nicht laden, AO auch net
        }
        for (uint32_t i = 0; i < scene_p->mNumMaterials; i++) {
            aiMaterial* material_p = scene_p->mMaterials[i];
            Material& material = _materials[i];
            printf("Material %d:\n", i);
            int textureCount = 0;
            //PBRE Materialien falls vorhanden
            std::map<std::string, uint32_t> textureTypes = {
                {"albedo", Material::HAS_ALBEDO_MAP},
                {"metallic", Material::HAS_METALLIC_MAP},
                {"normal", Material::HAS_NORMAL_MAP},
                {"ao", Material::HAS_AO_MAP},
                {"roughness", Material::HAS_ROUGHNESS_MAP}
            };
            
            std::map<std::string, aiTextureType> textureTypeMap = {
                {"albedo", aiTextureType_BASE_COLOR},
                {"metallic", aiTextureType_METALNESS},
                {"normal", aiTextureType_NORMALS},
                {"ao", aiTextureType_AMBIENT_OCCLUSION},
                {"roughness", aiTextureType_DIFFUSE_ROUGHNESS}
            };

            // Load Diffuse (Albedo) texture
            aiString texture_path;
            if (material_p->Get(AI_MATKEY_TEXTURE_DIFFUSE(0), texture_path) == AI_SUCCESS) {
                std::string full_texture_path = model_root + texture_path.C_Str();
                _materials[i]._texturesPBR[0]["albedo"].init(full_texture_path.c_str());
                _materials[i]._materialFlags |= Material::HAS_ALBEDO_MAP;
                _materials[i]._albedo_map = _materials[i]._texturesPBR[0]["albedo"]._texture;
                textureCount++;
                printf("Albedo texture loaded: %s\n", full_texture_path.c_str());
            }

            // Load Specular (Gloss) texture
            if (material_p->Get(AI_MATKEY_TEXTURE_SPECULAR(0), texture_path) == AI_SUCCESS) {
                std::string full_texture_path = model_root + texture_path.C_Str();
                _materials[i]._texturesPBR[0]["roughness"].init(full_texture_path.c_str()); // Assuming Gloss = Roughness
                _materials[i]._materialFlags |= Material::HAS_ROUGHNESS_MAP;
                _materials[i]._roughness_map = _materials[i]._texturesPBR[0]["roughness"]._texture;
                textureCount++;
                printf("Gloss texture loaded: %s\n", full_texture_path.c_str());
            }

            // Load Normal map
            if (material_p->Get(AI_MATKEY_TEXTURE_HEIGHT(0), texture_path) == AI_SUCCESS) {
                std::string full_texture_path = model_root + texture_path.C_Str();
                _materials[i]._texturesPBR[0]["normal"].init(full_texture_path.c_str());
                _materials[i]._materialFlags |= Material::HAS_NORMAL_MAP;
                _materials[i]._normal_map = _materials[i]._texturesPBR[0]["normal"]._texture;
                textureCount++;
                printf("Normal texture loaded: %s\n", full_texture_path.c_str());
            }


            if (textureCount > 2 && textureCount < 5) {
                std::string mtl_path = model_path.substr(0, model_path.find_last_of('.')) + ".mtl";
                std::ifstream mtlFile(mtl_path);
                if (!mtlFile.is_open()) {
                    fmt::println("Failed to open .mtl file: {}", model_path + ".mtl");
                    return;
                }

                std::string line;
                while (std::getline(mtlFile, line)) {
                    std::istringstream iss(line);
                    std::string key, value;
                    iss >> key;

                    if (key == "map_Metallic") {
                        iss >> value; // Pfad zur Metallic-Map
                        std::string full_texture_path = model_root + value;
                        if (std::filesystem::exists(full_texture_path)) {
                            _materials[i]._texturesPBR[0]["metallic"].init(full_texture_path.c_str());
                            _materials[i]._materialFlags |= Material::HAS_METALLIC_MAP;
                            _materials[i]._metallic_map = _materials[i]._texturesPBR[0]["metallic"]._texture;
                            fmt::println("Metallic texture loaded: {}", full_texture_path);
                        } else {
                            fmt::println("Warning: Metallic texture not found at {}", full_texture_path);
                        }
                    }
                    else if(key == "map_Ao") { 
                        iss >> value; // Pfad zur AO-Map
                        std::string full_texture_path = model_root + value;
                        if (std::filesystem::exists(full_texture_path)) {
                            _materials[i]._texturesPBR[0]["ao"].init(full_texture_path.c_str());
                            _materials[i]._materialFlags |= Material::HAS_AO_MAP;
                            _materials[i]._ao_map = _materials[i]._texturesPBR[0]["ao"]._texture;
                            fmt::println("AO texture loaded: {}", full_texture_path);
                        } else {
                            fmt::println("Warning: AO texture not found at {}", full_texture_path);
                        }
                    }
                }
            }

            
            if(textureCount < 3){
                _materials[i]._materialFlags |= Material::NO_PBR_TEXTURES;
                _materials[i]._texture_contribution = 1.0;
            }
            
            // load basic material properties
            material_p->Get(AI_MATKEY_SHININESS, material._specular);
            material_p->Get(AI_MATKEY_SHININESS_STRENGTH, material._specular_shininess);

            // load lighting colors
            aiColor3D color;
            material_p->Get(AI_MATKEY_COLOR_AMBIENT, color);
            material_p->Get(AI_MATKEY_COLOR_DIFFUSE, color);
            material_p->Get(AI_MATKEY_COLOR_SPECULAR, color);

        }
        
        // create meshes
        _meshes.resize(scene_p->mNumMeshes);
        for (uint32_t i = 0; i < scene_p->mNumMeshes; i++) {
            aiMesh* mesh_p = scene_p->mMeshes[i];
            _meshes[i].init(mesh_p);
        }
    }


    void destroy() {
        for (auto texture: _textures) {
            texture.destroy();
        }
        for (auto mesh: _meshes) {
            mesh.destroy();
        }
    }
    
//Bei PBR Shader immer mit pbr = true aufrufen, auch wenn nicht PBR texturen oder Materialien verwendet werden
void draw(bool color = true, bool pbr = false, GLuint program_id = 0, bool customMetallic = false, bool customRoughness = false, bool tessellation = false) {
    _transform.bind(); // Binde Transformationsmatrix
    for (uint32_t i = 0; i < _meshes.size(); i++) {
        uint32_t material_index = _meshes[i]._material_index;
        // Texturen binden nur, wenn "color" aktiv ist
        if (pbr && material_index < _textures.size()) {
            _textures[material_index].bindPBR(program_id);// PBR Texturen binden
        }
        else if (!pbr && material_index < _textures.size()) {
            _textures[material_index].bind(); // Standard Texturen binden
        }

        // Materialien binden basierend auf "pbr"-Flag
        if (pbr && program_id != 0) {
            _materials[material_index].bind(program_id, pbr); // PBR Materialbindung
            _materials[material_index]._heightScale = _heightScale;
            if(customMetallic && _useCustomMetallic){//Zusätzliche Metallic und Roughness Werte binden
                _materials[material_index].useCustomMetallic(program_id, _metallic);
            }
            if(customRoughness && _useCustomRoughness){
                _materials[material_index].useCustomRoughness(program_id, _roughness);
            }
        }
        else if (!pbr) {
            _materials[material_index].bind(program_id, pbr); // Standard Materialbindung
        }

        // Zeichne das Mesh
        _meshes[i].draw(tessellation);
    }
}
    float _heightScale = 0.05;
    float _metallic = 0.0;
    float _roughness = 0.0;
    bool _useCustomMetallic = false;
    bool _useCustomRoughness = false;
    std::vector<Mesh> _meshes;
    std::vector<Material> _materials;
    std::vector<Texture> _textures; // TODO: move into material
    Transform _transform;
};