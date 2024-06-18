#pragma once

#include <mikktspace/mikktspace.h>
#include <gltf/tiny_gltf.h>
#include <glm/glm.hpp>

struct VertexCountIndex
{
    size_t vertexCount;
    size_t indexCount;
    bool isInd16; // false is 32

    glm::vec3 *vertices;
    glm::vec3 *normals;
    glm::vec2 *uvs;
    uint16_t *ind16;
    uint32_t *ind32;

    glm::vec3 *tangents;
};

class TangentCalc
{
private:
    SMikkTSpaceInterface interface{};
    SMikkTSpaceContext context{};

    static int getNumFaces(const SMikkTSpaceContext *context);
    static int getNumVerticesOfFace(const SMikkTSpaceContext *context, const int face);
    static void getPosition(const SMikkTSpaceContext *context, float posOut[], const int face, const int vert);
    static void getNormal(const SMikkTSpaceContext *context, float normOut[], const int face, const int vert);
    static void getTexCoord(const SMikkTSpaceContext *context, float texcOut[], const int face, const int vert);

    static int get_vertex_index(const SMikkTSpaceContext *context, int face, int vert);

    static void setTSpaceBasic(const SMikkTSpaceContext *context, const float tangent[], const float sign, const int face, const int vert);

public:
    TangentCalc(tinygltf::Model *gltfModel, VertexCountIndex *vertexCountIndex);
    void calculate();
};

