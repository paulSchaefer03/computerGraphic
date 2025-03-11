#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "vertex.hpp"

struct ESphere
{
    const float PI = acos(-1.0f);

    ESphere(uint32_t sector_count, uint32_t stack_count)
        : _sector_count(sector_count), _stack_count(stack_count) {
        build();
    }


    const std::vector<Vertex>& getVertices() const { return _vertices; }
    const std::vector<uint32_t>& getIndices() const { return _indices; }

    void build() {
        _vertices.clear();
        _indices.clear();
                // https://www.songho.ca/opengl/gl_sphere.html
        float pi = 3.14159265358979323846f;
        float radius = 0.5f;
        // precalc expensive operations
        float length_recip = 1.0f / radius;
        float sector_step = 2.0f * pi / static_cast<float>(_sector_count);
        float stack_step = pi / static_cast<float>(_stack_count);

        // preallocate some space for vertices
        _vertices.reserve((_sector_count + 1) * (_stack_count + 1));

        // create vertices
        for (uint32_t i = 0; i <= _stack_count; i++) {
            float stack_angle = pi / 2.0f - static_cast<float>(i) * stack_step;
            float xy = radius * std::cos(stack_angle);
            float z = radius * std::sin(stack_angle);

            for (uint32_t k = 0; k <= _sector_count; k++) {
                Vertex vertex;

                float sector_angle = static_cast<float>(k) * sector_step;
                vertex.position.x = xy * std::cos(sector_angle);
                vertex.position.y = xy * std::sin(sector_angle);
                vertex.position.z = z;
                vertex.position.x = round(vertex.position.x * 1e5) / 1e5;
                vertex.position.y = round(vertex.position.y * 1e5) / 1e5;
                vertex.position.z = round(vertex.position.z * 1e5) / 1e5;


                // Falls letzter Vertex in einem Sektor, gleiche ihn dem ersten an
                if (k == _sector_count) {
                    vertex.position = _vertices[i * (_sector_count + 1)].position;
                    vertex.uv = glm::vec2(1.0f, static_cast<float>(i) / _stack_count);
                } else {
                    vertex.uv[0] = static_cast<float>(k % _sector_count) / _sector_count;
                    vertex.uv[1] = static_cast<float>(i) / _stack_count;
                }

                vertex.normal = glm::normalize(vertex.position);
                vertex.color = glm::vec4(0.9, 0.9, 0, 1);
                _vertices.push_back(vertex);
            }
        }

        // create indices
        // k1--k1+1
        // |  / |
        // | /  |
        // k2--k2+1
        for (uint32_t i = 0; i < _stack_count; i++) {
            uint32_t k1 = i * (_sector_count + 1); // beginning of current stack
            uint32_t k2 = k1 + _sector_count + 1;  // beginning of next stack

            for (uint32_t j = 0; j < (uint32_t)_sector_count; j++, k1++, k2++) {
                // 2 triangles per sector excluding first and last stacks
                if (i != 0) {
                    _indices.insert(_indices.end(), {
                        k1, k2, k1 + 1
                    });
                }
                if (i != _stack_count - 1) {
                    _indices.insert(_indices.end(), {
                        k1 + 1, k2, k2 + 1
                    });
                }
            }
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

    uint32_t _sector_count;
    uint32_t _stack_count;
    std::vector<Vertex> _vertices;
    std::vector<uint32_t> _indices;
};
