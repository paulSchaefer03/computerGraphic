#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <map>
#include <entities/vertex.hpp>

struct Icosphere {

    Icosphere(float radius = 1.0f, int subdivisions = 1)
        : _radius(radius), _subdivisions(subdivisions) {
        build();
        calculateTangents(_vertices, _indices);
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
    const float PI = acos(-1.0f);
    using Index = uint32_t;
    using Lookup = std::map<std::pair<Index, Index>, Index>;

    void build() {
        using Index = uint32_t;
        using VertexList = std::vector<glm::vec3>;
        using Triangle = std::array<Index, 3>;
        using TriangleList = std::vector<Triangle>;

        const float X = 0.525731112119133606f;
        const float Z = 0.850650808352039932f;

        // Initial Icosphere
        VertexList rawVertices = {
            {-X, 0, Z}, {X, 0, Z}, {-X, 0, -Z}, {X, 0, -Z},
            {0, Z, X}, {0, Z, -X}, {0, -Z, X}, {0, -Z, -X},
            {Z, X, 0}, {-Z, X, 0}, {Z, -X, 0}, {-Z, -X, 0}
        };
        TriangleList triangles = {
            {0, 4, 1}, {0, 9, 4}, {9, 5, 4}, {4, 5, 8}, {4, 8, 1},
            {8, 10, 1}, {8, 3, 10}, {5, 3, 8}, {5, 2, 3}, {2, 7, 3},
            {7, 10, 3}, {7, 6, 10}, {7, 11, 6}, {11, 0, 6}, {0, 1, 6},
            {6, 1, 10}, {9, 0, 11}, {9, 11, 2}, {9, 2, 5}, {7, 2, 11}
        };

        // Subdivide triangles
        for (int i = 0; i < _subdivisions; ++i) {
            triangles = subdivide(rawVertices, triangles);
        }

        // Normalize vertices and calculate UVs
        _vertices.clear();
        for (const auto& pos : rawVertices) {
            glm::vec3 normalized = glm::normalize(pos) * _radius;
            glm::vec3 normal = glm::normalize(normalized);
            float u = 0.5f + atan2(normal.z, normal.x) / (2.0f * PI);
            float v = 0.5f - asin(normal.y) / PI;
            _vertices.push_back({normalized, normal, glm::vec4(1.0f), glm::vec2(u, v)});
        }

        // Convert triangles to indices
        _indices.clear();
        for (const auto& tri : triangles) {
            _indices.insert(_indices.end(), {tri[0], tri[1], tri[2]});
        }
    }


private:
    using Index = uint32_t;

    Index vertex_for_edge(Lookup& lookup, std::vector<glm::vec3>& vertices,
                          Index first, Index second) {
        auto key = std::make_pair(std::min(first, second), std::max(first, second));

        auto inserted = lookup.insert({key, vertices.size()});
        if (inserted.second) {
            glm::vec3 middle = glm::normalize((vertices[first] + vertices[second]) / 2.0f);
            vertices.push_back(middle);
        }

        return inserted.first->second;
    }

    std::vector<std::array<Index, 3>> subdivide(std::vector<glm::vec3>& vertices,
                                                const std::vector<std::array<Index, 3>>& triangles) {
        Lookup lookup;
        std::vector<std::array<Index, 3>> result;
        const float PI = acos(-1.0f);
        for (const auto& tri : triangles) {
            std::array<Index, 3> mid;
            for (int edge = 0; edge < 3; ++edge) {
                mid[edge] = vertex_for_edge(lookup, vertices, tri[edge], tri[(edge + 1) % 3]);
            }

            result.push_back({tri[0], mid[0], mid[2]});
            result.push_back({tri[1], mid[1], mid[0]});
            result.push_back({tri[2], mid[2], mid[1]});
            result.push_back({mid[0], mid[1], mid[2]});
        }

        return result;
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
