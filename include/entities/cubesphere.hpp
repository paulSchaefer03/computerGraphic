#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "vertex.hpp"

struct Cubesphere
{
    const float PI = acos(-1.0f);

    Cubesphere(float radius = 1.0f, int subdivisions = 3)
        : _radius(radius), _subdivisions(subdivisions) {
        build();
    }

    void setRadius(float radius) {
        _radius = radius;
        build();
    }

    void setSubdivisions(int subdivisions) {
        _subdivisions = subdivisions;
        build();
    }

    const std::vector<Vertex>& getVertices() const { return _vertices; }
    const std::vector<uint32_t>& getIndices() const { return _indices; }

    void build() {
        _vertices.clear();
        _indices.clear();

        // Build each face of the cube
        buildFace(glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, 1));  // +X
        buildFace(glm::vec3(-1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, -1)); // -X
        buildFace(glm::vec3(0, 1, 0), glm::vec3(1, 0, 0), glm::vec3(0, 0, -1));  // +Y
        buildFace(glm::vec3(0, -1, 0), glm::vec3(1, 0, 0), glm::vec3(0, 0, 1));  // -Y
        buildFace(glm::vec3(0, 0, 1), glm::vec3(1, 0, 0), glm::vec3(0, 1, 0));  // +Z
        buildFace(glm::vec3(0, 0, -1), glm::vec3(1, 0, 0), glm::vec3(0, -1, 0)); // -Z

        calculateTangents(_vertices, _indices);
    }

    void buildFace(glm::vec3 faceNormal, glm::vec3 up, glm::vec3 right) {
        int faceStartIndex = _vertices.size();
        float step = 1.0f / _subdivisions;

        for (int i = 0; i <= _subdivisions; ++i) {
            for (int j = 0; j <= _subdivisions; ++j) {
                // Map i, j to a local position on the cube face
                glm::vec3 pos = faceNormal + (j * step - 0.5f) * 2.0f * right + (i * step - 0.5f) * 2.0f * up;
                glm::vec3 normalized = glm::normalize(pos);
                glm::vec3 finalPos = normalized * _radius;
                // Global UV mapping based on cube coordinates
                float u = 0.5f + atan2(normalized.z, normalized.x) / (2.0f * PI);
                float v = 0.5f - asin(normalized.y) / PI; 

            // Correct edge alignment by calculating global texture position
                if (fabs(faceNormal.x) > 0.5f) {
                    // X Faces
                    u = (normalized.z + 1.0f) * 0.5f;
                    v = (normalized.y + 1.0f) * 0.5f;
                } else if (fabs(faceNormal.y) > 0.5f) {
                    // Y Faces
                    u = (normalized.x + 1.0f) * 0.5f;
                    v = (normalized.z + 1.0f) * 0.5f;
                    u = 1.0f - u;

                } else if (fabs(faceNormal.z) > 0.5f) {
                    // Z Faces
                    u = (normalized.x + 1.0f) * 0.5f;
                    v = (normalized.y + 1.0f) * 0.5f;
                } 

                // Add vertex with smoothed normal
                glm::vec3 normal = normalized;
                _vertices.push_back({finalPos, normal, glm::vec4(1.0f), {u, v}});
            }
        }

        // Generate indices for the face
        for (int i = 0; i < _subdivisions; ++i) {
            for (int j = 0; j < _subdivisions; ++j) {
                int k1 = faceStartIndex + i * (_subdivisions + 1) + j;
                int k2 = k1 + _subdivisions + 1;

                // Two triangles per quad
                _indices.push_back(k1);
                _indices.push_back(k2);
                _indices.push_back(k1 + 1);

                _indices.push_back(k1 + 1);
                _indices.push_back(k2);
                _indices.push_back(k2 + 1);
            }
        }
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

    float _radius;
    int _subdivisions;
    std::vector<Vertex> _vertices;
    std::vector<uint32_t> _indices;
};
