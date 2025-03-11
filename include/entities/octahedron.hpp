#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "vertex.hpp"

const float PI = acos(-1.0f);

struct Octahedron {
    Octahedron(float radius = 1.0f) : _radius(radius) {
        build();
    }

    const std::vector<Vertex>& getVertices() const { return _vertices; }
    const std::vector<uint32_t>& getIndices() const { return _indices; }

    void build() {
        _vertices.clear();
        _indices.clear();

        glm::vec3 positions[6] = {
            {0,  _radius,  0},
            { _radius, 0,  0},
            {0,  0,  _radius},
            {-_radius, 0,  0},
            {0,  0, -_radius},
            {0, -_radius,  0}
        };

        uint32_t triangles[8][3] = {
            {0, 1, 2}, {0, 2, 3}, {0, 3, 4}, {0, 4, 1},
            {5, 2, 1}, {5, 3, 2}, {5, 4, 3}, {5, 1, 4}
        };

        glm::vec2 faceUVs[3] = {
            {0.5f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f}
        };

        for (auto& tri : triangles) {
            glm::vec3 normal = glm::normalize(glm::cross(
                positions[tri[1]] - positions[tri[0]],
                positions[tri[2]] - positions[tri[0]]
            ));

            for (int i = 0; i < 3; ++i) {
                _vertices.push_back({positions[tri[i]], normal, glm::vec4(1.0f), faceUVs[i]});
            }

            GLuint baseIndex = _vertices.size() - 3;
            _indices.insert(_indices.end(), {baseIndex, baseIndex + 1, baseIndex + 2});

            _indices.insert(_indices.end(), {baseIndex + 2, baseIndex + 1, baseIndex});
        }
    }

    float _radius;
    std::vector<Vertex> _vertices;
    std::vector<uint32_t> _indices;
};