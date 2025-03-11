#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "vertex.hpp"

// Cube Primitive
struct Cube {
    Cube(float size = 1.0f) : _size(size) {
        build();
    }

    const std::vector<Vertex>& getVertices() const { return _vertices; }
    const std::vector<uint32_t>& getIndices() const { return _indices; }

    void build() {
        _vertices.clear();
        _indices.clear();

        float half = _size / 2.0f;

        // Define the vertices of a cube
        glm::vec3 positions[8] = {
            {-half, -half, -half}, {half, -half, -half},
            {half,  half, -half}, {-half,  half, -half},
            {-half, -half,  half}, {half, -half,  half},
            {half,  half,  half}, {-half,  half,  half}
        };

        glm::vec2 uvs[4] = {
            {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}
        };

        glm::vec3 normals[6] = {
            {0, 0, -1}, {0, 0, 1},
            {0, -1, 0}, {0, 1, 0},
            {-1, 0, 0}, {1, 0, 0}
        };

        // Define the faces of the cube (each face has 2 triangles)
        uint32_t faceIndices[6][4] = {
            {0, 1, 2, 3}, // Back
            {4, 5, 6, 7}, // Front
            {0, 1, 5, 4}, // Bottom
            {3, 2, 6, 7}, // Top
            {0, 3, 7, 4}, // Left
            {1, 2, 6, 5}  // Right
        };

        for (int i = 0; i < 6; ++i) {
            int idx = _vertices.size();
            for (int j = 0; j < 4; ++j) {
                _vertices.push_back({positions[faceIndices[i][j]], normals[i], glm::vec4(0.8f, 0.0f, 0.0f, 1.0f), uvs[j]});
            }

            // Add indices for both triangle windings
            _indices.push_back(idx + 0);
            _indices.push_back(idx + 1);
            _indices.push_back(idx + 2);
            _indices.push_back(idx + 2);
            _indices.push_back(idx + 3);
            _indices.push_back(idx + 0);

             // Reverse winding for backface rendering
            _indices.push_back(idx + 2);
            _indices.push_back(idx + 1);
            _indices.push_back(idx + 0);
            _indices.push_back(idx + 0);
            _indices.push_back(idx + 3);
            _indices.push_back(idx + 2);
        }

        calculateTangents(_vertices, _indices);
    }

    void calculateTangents(std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) {
        std::vector<glm::vec3> tangents(vertices.size(), glm::vec3(0.0f));
        std::vector<glm::vec3> bitangents(vertices.size(), glm::vec3(0.0f));

        for (size_t i = 0; i < indices.size(); i += 3) {
            Vertex& v0 = vertices[indices[i + 0]];
            Vertex& v1 = vertices[indices[i + 1]];
            Vertex& v2 = vertices[indices[i + 2]];

            glm::vec3 edge1 = v1.position - v0.position;
            glm::vec3 edge2 = v2.position - v0.position;
            glm::vec2 deltaUV1 = v1.uv - v0.uv;
            glm::vec2 deltaUV2 = v2.uv - v0.uv;

            float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

            glm::vec3 tangent = f * (deltaUV2.y * edge1 - deltaUV1.y * edge2);
            glm::vec3 bitangent = f * (-deltaUV2.x * edge1 + deltaUV1.x * edge2);

            tangents[indices[i + 0]] += tangent;
            tangents[indices[i + 1]] += tangent;
            tangents[indices[i + 2]] += tangent;

            bitangents[indices[i + 0]] += bitangent;
            bitangents[indices[i + 1]] += bitangent;
            bitangents[indices[i + 2]] += bitangent;
        }

        for (size_t i = 0; i < vertices.size(); ++i) {
            vertices[i].tangent = glm::normalize(tangents[i]);
            vertices[i].bitangent = glm::normalize(bitangents[i]);
        }
    }

    float _size;
    std::vector<Vertex> _vertices;
    std::vector<uint32_t> _indices;
};

void renderCube(Cube& cube) {
        static GLuint VAO = 0, VBO = 0, EBO = 0;

        if (VAO == 0) {
            // Erstelle VAO, VBO und EBO
            glGenVertexArrays(1, &VAO);
            glGenBuffers(1, &VBO);
            glGenBuffers(1, &EBO);

            glBindVertexArray(VAO);

            // Buffer für die Vertices
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, cube.getVertices().size() * sizeof(Vertex), cube.getVertices().data(), GL_STATIC_DRAW);

            // Buffer für die Indices
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, cube.getIndices().size() * sizeof(uint32_t), cube.getIndices().data(), GL_STATIC_DRAW);

            // Position
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
            glEnableVertexAttribArray(0);

            // Normalen
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
            glEnableVertexAttribArray(1);

            // UV-Koordinaten
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
            glEnableVertexAttribArray(2);
        }

        // Zeichne den Würfel
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, cube.getIndices().size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
}

GLuint _quadVAO = 0, _quadVBO = 0;
void renderFullscreenQuad() {
    if (_quadVAO == 0) {
        float quadVertices[] = {
            // Position         // Tex-Koordinaten
            -1.0f,  1.0f,       0.0f, 1.0f,  // oben links
            -1.0f, -1.0f,       0.0f, 0.0f,  // unten links
             1.0f, -1.0f,       1.0f, 0.0f,  // unten rechts

            -1.0f,  1.0f,       0.0f, 1.0f,  // oben links
             1.0f, -1.0f,       1.0f, 0.0f,  // unten rechts
             1.0f,  1.0f,       1.0f, 1.0f   // oben rechts
        };
        glGenVertexArrays(1, &_quadVAO);
        glGenBuffers(1, &_quadVBO);
        glBindVertexArray(_quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, _quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    }
    glBindVertexArray(_quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}