#include "tangentcalc.h"

TangentCalc::TangentCalc(tinygltf::Model *gltfModel, VertexCountIndex *vertexCountIndex)
{
    interface.m_getNumFaces = getNumFaces;
    interface.m_getNumVerticesOfFace = getNumVerticesOfFace;
    interface.m_getPosition = getPosition;
    interface.m_getNormal = getNormal;
    interface.m_getTexCoord = getTexCoord;
    interface.m_setTSpaceBasic = setTSpaceBasic;

    context.m_pInterface = &interface;

    context.m_pUserData = vertexCountIndex;
}

int TangentCalc::getNumFaces(const SMikkTSpaceContext *context)
{
    auto indexCount = static_cast<VertexCountIndex*>(context->m_pUserData)->indexCount;
    return indexCount  / 3;
}

int TangentCalc::getNumVerticesOfFace(const SMikkTSpaceContext *context, const int face)
{
    return 3;
}

void TangentCalc::getPosition(const SMikkTSpaceContext *context, float *posOut, const int face, const int vert)
{
    auto vertexCountIndex = static_cast<VertexCountIndex*>(context->m_pUserData);
    auto vertices = vertexCountIndex->vertices;
    auto index = get_vertex_index(context, face, vert);
    auto vertex = vertices[index];
    posOut[0] = vertex.x;
    posOut[1] = vertex.y;
    posOut[2] = vertex.z;
}

void TangentCalc::getNormal(const SMikkTSpaceContext *context, float *normOut, const int face, const int vert)
{
    auto vertexCountIndex = static_cast<VertexCountIndex*>(context->m_pUserData);
    auto normals = vertexCountIndex->normals;
    auto index = get_vertex_index(context, face, vert);
    auto normal = normals[index];
    normOut[0] = normal.x;
    normOut[1] = normal.y;
    normOut[2] = normal.z;
}

void TangentCalc::getTexCoord(const SMikkTSpaceContext *context, float *texcOut, const int face, const int vert)
{
    auto vertexCountIndex = static_cast<VertexCountIndex*>(context->m_pUserData);
    auto uvs = vertexCountIndex->uvs;
    auto index = get_vertex_index(context, face, vert);
    auto uv = uvs[index];
    texcOut[0] = uv.x;
    texcOut[1] = uv.y;
}

void
TangentCalc::setTSpaceBasic(const SMikkTSpaceContext *context, const float *tangent, const float sign, const int face,
                            const int vert)
{
    auto index = get_vertex_index(context, face, vert);

    auto vertexCountIndex = static_cast<VertexCountIndex*>(context->m_pUserData);
    auto tangents = vertexCountIndex->tangents;
    auto fTangent = glm::vec3(tangent[0], tangent[1], tangent[2]);
    auto fSign = sign;

    tangents[index] = fTangent;
}

int TangentCalc::get_vertex_index(const SMikkTSpaceContext *context, int face, int vert)
{
    auto vertexCountIndex = static_cast<VertexCountIndex*>(context->m_pUserData);
    auto ind16 = vertexCountIndex->ind16;
    auto ind32 = vertexCountIndex->ind32;
    auto indices_index = (face * 3) + vert;
    if (vertexCountIndex->ind16)
    {
        int index = ind16[indices_index];
        return index;
    }
    else
    {
        int index = ind32[indices_index];
        return index;
    }
    return -1;
}

void TangentCalc::calculate()
{
    genTangSpaceDefault(&context);
}
