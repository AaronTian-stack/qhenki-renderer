#define TINYOBJLOADER_IMPLEMENTATION
//#define TINYOBJLOADER_USE_MAPBOX_EARCUT
#include "obj/tiny_obj_loader.h"

#include "primitive.h"
#include <iostream>

std::pair<uPtr<Buffer>, uPtr<Buffer>> Primitive::create(BufferFactory &bufferFactory, std::string path)
{
    tinyobj::ObjReaderConfig reader_config;

    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(path, reader_config)) {
        if (!reader.Error().empty()) {
            std::cerr << "TinyObjReader: " << reader.Error();
        }
        exit(1);
    }

    if (!reader.Warning().empty()) {
        std::cout << "TinyObjReader: " << reader.Warning();
    }

    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();

    if (shapes.size() > 1)
        throw std::runtime_error("not a simple primitive");

    auto &shape = shapes[0];

    auto posBuffer = bufferFactory.createBuffer(attrib.vertices.size() * sizeof(float),
                                                vk::BufferUsageFlagBits::eVertexBuffer,
                                                VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
    posBuffer->fill(attrib.vertices.data());

    auto positionIndices = std::vector<uint16_t>(shape.mesh.indices.size());
    for (size_t i = 0; i < shape.mesh.indices.size(); i++)
    {
        positionIndices[i] = shape.mesh.indices[i].vertex_index;
    }

    auto indexBuffer = bufferFactory.createBuffer(positionIndices.size() * sizeof(uint16_t),
                                                  vk::BufferUsageFlagBits::eIndexBuffer,
                                                  VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
    indexBuffer->fill(positionIndices.data());

    return {std::move(posBuffer), std::move(indexBuffer)};
}

void Primitive::create(BufferFactory &bufferFactory)
{
    auto cube = create(bufferFactory, "../resources/objs/cube.obj");
    auto sphere = create(bufferFactory, "../resources/objs/sphere.obj");
    cubePosBuffer = std::move(cube.first);
    cubeIndexBuffer = std::move(cube.second);
    spherePosBuffer = std::move(sphere.first);
    sphereIndexBuffer = std::move(sphere.second);
}

void Primitive::destroy()
{
    cubePosBuffer->destroy();
    cubeIndexBuffer->destroy();
    spherePosBuffer->destroy();
    sphereIndexBuffer->destroy();

//    primitivePipeline->destroy();
//    primitiveShader->destroy();
}
