#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <assimp/mesh.h>
#include <glbinding/gl46core/gl.h>
#include "vertex.hpp"
#include "cubesphere.hpp"
#include "icosphere.hpp"
#include "cube.hpp"
#include "octahedron.hpp"
#include "eSphere.hpp"

using namespace gl46core;

struct Mesh {
    enum Primitive { eCube, eSphere, cubeSphere, icoSphere, cube, octahedron };

    // load cube primitive
    void init() {
        // create vertices
        float n = -0.5f; // for readability
        float p = +0.5f; // for readability
        std::vector<Vertex> vertices = {
            {{n, n, p}, {0, 0, +1}, {1, 0, 0, 1}, {0.33, 0.75}}, // front
            {{p, n, p}, {0, 0, +1}, {1, 0, 0, 1}, {0.66, 0.75}},
            {{n, p, p}, {0, 0, +1}, {1, 0, 0, 1}, {0.33, 0.50}},
            {{p, p, p}, {0, 0, +1}, {1, 0, 0, 1}, {0.66, 0.50}},
            {{n, n, n}, {0, 0, -1}, {1, 0, 0, 1}, {0.33, 0.00}}, // back
            {{p, n, n}, {0, 0, -1}, {1, 0, 0, 1}, {0.66, 0.00}},
            {{n, p, n}, {0, 0, -1}, {1, 0, 0, 1}, {0.33, 0.25}},
            {{p, p, n}, {0, 0, -1}, {1, 0, 0, 1}, {0.66, 0.25}},
            {{n, n, n}, {-1, 0, 0}, {0, 1, 0, 1}, {0.00, 0.50}}, // left
            {{n, n, p}, {-1, 0, 0}, {0, 1, 0, 1}, {0.00, 0.25}},
            {{n, p, n}, {-1, 0, 0}, {0, 1, 0, 1}, {0.33, 0.50}},
            {{n, p, p}, {-1, 0, 0}, {0, 1, 0, 1}, {0.33, 0.25}},
            {{p, n, n}, {+1, 0, 0}, {0, 1, 0, 1}, {1.00, 0.50}}, // right
            {{p, n, p}, {+1, 0, 0}, {0, 1, 0, 1}, {1.00, 0.25}},
            {{p, p, n}, {+1, 0, 0}, {0, 1, 0, 1}, {0.66, 0.50}},
            {{p, p, p}, {+1, 0, 0}, {0, 1, 0, 1}, {0.66, 0.25}},
            {{n, p, n}, {0, +1, 0}, {0, 0, 1, 1}, {0.33, 0.25}}, // top
            {{n, p, p}, {0, +1, 0}, {0, 0, 1, 1}, {0.33, 0.50}},
            {{p, p, n}, {0, +1, 0}, {0, 0, 1, 1}, {0.66, 0.25}},
            {{p, p, p}, {0, +1, 0}, {0, 0, 1, 1}, {0.66, 0.50}},
            {{n, n, n}, {0, -1, 0}, {0, 0, 1, 1}, {0.33, 0.75}}, // bottom
            {{n, n, p}, {0, -1, 0}, {0, 0, 1, 1}, {0.33, 1.00}},
            {{p, n, n}, {0, -1, 0}, {0, 0, 1, 1}, {0.66, 0.75}},
            {{p, n, p}, {0, -1, 0}, {0, 0, 1, 1}, {0.66, 1.00}},
        };

        // create indices
        std::vector<uint32_t> indices = {
            0, 1, 3, 3, 2, 0, // front
            5, 4, 7, 7, 4, 6, // back
            8, 9, 11, 11, 10, 8, // left
            13, 12, 15, 15, 12, 14, // right
            16, 17, 19, 19, 18, 16, // top
            23, 21, 20, 23, 20, 22, // bottom
        };
        describe_layout(vertices, indices);
    }
    // load sphere primitive
    void initESphere(uint32_t sector_count, uint32_t stack_count) {
        ESphere esphere(sector_count, stack_count);
        std::vector<Vertex> vertices;
        vertices = esphere.getVertices();
        std::vector<uint32_t> indices;
        indices = esphere.getIndices();
        describe_layout(vertices, indices);
    }

    void initCubeSphere(float radius = 1.0f, uint32_t subdivisions = 3){
        Cubesphere cubesphere(radius, subdivisions);
        std::vector<Vertex> vertices;
        vertices = cubesphere.getVertices();
        std::vector<uint32_t> indices;
        indices = cubesphere.getIndices();
        describe_layout(vertices, indices);
    }
    void initIcoSphere(float radius = 1.0f, uint32_t subdivisions = 3){
        Icosphere icosphere(radius, subdivisions);
        std::vector<Vertex> vertices;
        vertices = icosphere.getVertices();
        std::vector<uint32_t> indices;
        indices = icosphere.getIndices();
        describe_layout(vertices, indices);
    }
    void initCube(float size = 1.0f) {
        Cube cube(size);
        std::vector<Vertex> vertices;
        vertices = cube.getVertices();
        std::vector<uint32_t> indices;
        indices = cube.getIndices();
        describe_layout(vertices, indices);
    }
    void initOctahedron(float radius = 1.0f) {
        Octahedron octahedron(radius);
        std::vector<Vertex> vertices;
        vertices = octahedron.getVertices();
        std::vector<uint32_t> indices;
        indices = octahedron.getIndices();
        describe_layout(vertices, indices);
    }
    // load mesh from assimp scene
    void init(aiMesh* mesh_p) {
        std::vector<Vertex> vertices;
        vertices.reserve(mesh_p->mNumVertices);
        for (uint32_t i = 0; i < mesh_p->mNumVertices; i++) {
            Vertex vertex;
            // extract positions
            vertex.position.x = mesh_p->mVertices[i].x;
            vertex.position.y = mesh_p->mVertices[i].y;
            vertex.position.z = mesh_p->mVertices[i].z;
            // extract normals
            vertex.normal.x = mesh_p->mNormals[i].x;
            vertex.normal.y = mesh_p->mNormals[i].y;
            vertex.normal.z = mesh_p->mNormals[i].z;
            // extract uv/st coords
            if (mesh_p->HasTextureCoords(0)) {
                vertex.uv.s = mesh_p->mTextureCoords[0][i].x;
                vertex.uv.t = mesh_p->mTextureCoords[0][i].y;
            }
            else vertex.uv = {0, 0};
            // extract vertex colors (if present)
            if (mesh_p->HasVertexColors(0)) {
                vertex.color.r = mesh_p->mColors[0][i].r;
                vertex.color.g = mesh_p->mColors[0][i].g;
                vertex.color.b = mesh_p->mColors[0][i].b;
                vertex.color.a = mesh_p->mColors[0][i].a;
            }
            else vertex.color = {1, 1, 1, 1};
            vertices.push_back(vertex);
        }

        std::vector<uint32_t> indices;
        indices.reserve(mesh_p->mNumFaces * 3);
        for (int i = 0; i < mesh_p->mNumFaces; i++) {
            aiFace face = mesh_p->mFaces[i];
            assert(face.mNumIndices == 3);
            for (int j = 0; j < face.mNumIndices; j++) {
                indices.push_back(face.mIndices[j]);
            }
        }
        _material_index = mesh_p->mMaterialIndex;
        describe_layout(vertices, indices);
    }
    // describe memory layout
    void describe_layout(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices) {
        _index_count = indices.size();

        // describe vertex buffer
        GLsizeiptr vertex_byte_count = vertices.size() * sizeof(Vertex);
        glCreateBuffers(1, &_vertex_buffer_object);
        // upload data to GPU buffer
        glNamedBufferStorage(_vertex_buffer_object, vertex_byte_count, vertices.data(), BufferStorageMask::GL_NONE_BIT);

        // describe index buffer (element buffer)
        GLsizeiptr element_byte_count = indices.size() * sizeof(uint32_t);
        glCreateBuffers(1, &_element_buffer_object);
        // upload data to GPU buffer
        glNamedBufferStorage(_element_buffer_object, element_byte_count, indices.data(), BufferStorageMask::GL_NONE_BIT);

        // create vertex array buffer
        glCreateVertexArrays(1, &_vertex_array_object);
        // assign both vertex and index (element) buffers
        glVertexArrayVertexBuffer(_vertex_array_object, 0, _vertex_buffer_object, 0, sizeof(Vertex));
        glVertexArrayElementBuffer(_vertex_array_object, _element_buffer_object);
        // struct Vertex {
        //     glm::vec3 position; <---
        //     glm::vec3 normal;
        //     glm::vec4 color;
        //     glm::vec2 uv;
        // };
        // total size of 3 floats, starts at byte 0*GL_FLOAT
        glVertexArrayAttribFormat(_vertex_array_object, 0, 3, GL_FLOAT, GL_FALSE, 0 * sizeof(GL_FLOAT));
        glVertexArrayAttribBinding(_vertex_array_object, 0, 0);
        glEnableVertexArrayAttrib(_vertex_array_object, 0);
        // struct Vertex {
        //     glm::vec3 position;
        //     glm::vec3 normal; <---
        //     glm::vec4 color;
        //     glm::vec2 uv;
        // };
        // total size of 3 floats, starts at byte 3*GL_FLOAT
        glVertexArrayAttribFormat(_vertex_array_object, 1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT));
        glVertexArrayAttribBinding(_vertex_array_object, 1, 0);
        glEnableVertexArrayAttrib(_vertex_array_object, 1);
        // struct Vertex {
        //     glm::vec3 position;
        //     glm::vec3 normal;
        //     glm::vec4 color; <---
        //     glm::vec2 uv;
        // };
        // total size of 4 floats, starts at byte 6*GL_FLOAT
        glVertexArrayAttribFormat(_vertex_array_object, 2, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(GL_FLOAT));
        glVertexArrayAttribBinding(_vertex_array_object, 2, 0);
        glEnableVertexArrayAttrib(_vertex_array_object, 2);
        // struct Vertex {
        //     glm::vec3 position;
        //     glm::vec3 normal;
        //     glm::vec4 color;
        //     glm::vec2 uv;
        // };
        // total size of 2 floats, starts at byte 10*GL_FLOAT
        glVertexArrayAttribFormat(_vertex_array_object, 3, 2, GL_FLOAT, GL_FALSE, 10 * sizeof(GL_FLOAT));
        glVertexArrayAttribBinding(_vertex_array_object, 3, 0);
        glEnableVertexArrayAttrib(_vertex_array_object, 3);

        if(sizeof(Vertex) > 48){
            // struct Vertex {
            //     glm::vec3 position;
            //     glm::vec3 normal;
            //     glm::vec4 color;
            //     glm::vec2 uv;
            //     glm::vec3 tangent; <---
            //     glm::vec3 bitangent; <---
            // };
            // total size of 3 floats, starts at byte 12*GL_FLOAT
            glVertexArrayAttribFormat(_vertex_array_object, 4, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(GL_FLOAT));
            glVertexArrayAttribBinding(_vertex_array_object, 4, 0);
            glEnableVertexArrayAttrib(_vertex_array_object, 4);
            // struct Vertex {
            //     glm::vec3 position;
            //     glm::vec3 normal;
            //     glm::vec3 color;
            //     glm::vec2 uv;
            //     glm::vec3 tangent;
            //     glm::vec3 bitangent; <---
            // };
            // total size of 3 floats, starts at byte 15*GL_FLOAT
            glVertexArrayAttribFormat(_vertex_array_object, 5, 3, GL_FLOAT, GL_FALSE, 15 * sizeof(GL_FLOAT));
            glVertexArrayAttribBinding(_vertex_array_object, 5, 0);
            glEnableVertexArrayAttrib(_vertex_array_object, 5);
        }

    }
    // clean up mesh buffers
    void destroy() {
        glDeleteBuffers(1, &_vertex_buffer_object);
        glDeleteBuffers(1, &_element_buffer_object);
        glDeleteVertexArrays(1, &_vertex_array_object);
    }
    // draw the mesh using previously bound pipeline
    void draw(bool tessellate = false) {
        glBindVertexArray(_vertex_array_object);
        if(tessellate){
            glPatchParameteri(GL_PATCH_VERTICES, 3);
            glDrawElements(GL_PATCHES, _index_count, GL_UNSIGNED_INT, nullptr);
        } else {
            glDrawElements(GL_TRIANGLES, _index_count, GL_UNSIGNED_INT, nullptr);
        }
    }

    GLuint _vertex_buffer_object;
    GLuint _element_buffer_object;
    GLuint _vertex_array_object;
    GLsizei _index_count;
    uint32_t _material_index = 0;
    bool _use_standard_color = false;
};