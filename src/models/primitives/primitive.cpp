#define TINYOBJLOADER_IMPLEMENTATION
//#define TINYOBJLOADER_USE_MAPBOX_EARCUT
#include "obj/tiny_obj_loader.h"

#include "primitive.h"
#include <iostream>

Primitive::Primitive(BufferFactory &bufferFactory, std::string path)
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

    positionBuffer = bufferFactory.createBuffer(attrib.vertices.size() * sizeof(float),
                                                vk::BufferUsageFlagBits::eVertexBuffer,
                                                VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
    positionBuffer->fill(attrib.vertices.data());

    auto positionIndices = std::vector<uint16_t>(shape.mesh.indices.size());
    for (size_t i = 0; i < shape.mesh.indices.size(); i++)
    {
        positionIndices[i] = shape.mesh.indices[i].vertex_index;
    }

    indexBuffer = bufferFactory.createBuffer(positionIndices.size() * sizeof(uint16_t),
                                                  vk::BufferUsageFlagBits::eIndexBuffer,
                                                  VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
    indexBuffer->fill(positionIndices.data());
}

void Primitive::destroy()
{
    positionBuffer->destroy();
    indexBuffer->destroy();
}

void Primitive::draw(vk::CommandBuffer commandBuffer)
{
    // TODO: instanced rendering
    vk::DeviceSize offset = 0;
    size_t size = indexBuffer->getIndexType() == vk::IndexType::eUint16 ? sizeof(uint16_t) : sizeof(uint32_t);
    auto count = indexBuffer->info.size / size;
    commandBuffer.bindVertexBuffers(0, 1, &positionBuffer->buffer, &offset);
    indexBuffer->bind(commandBuffer);
    commandBuffer.drawIndexed(count, 1, 0, 0, 0);
}
