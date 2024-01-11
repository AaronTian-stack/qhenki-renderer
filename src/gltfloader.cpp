// Define these only in *one* .cc file.
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
// #define TINYGLTF_NOEXCEPTION // optional. disable exception handling.

#include <iostream>
#include "gltfloader.h"

GLTFLoader::GLTFLoader()
{

}

int GLTFLoader::load()
{
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;

    bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, "Box.glb"); // for binary glTF(.glb)

    if (!warn.empty()) {
        printf("Warn: %s\n", warn.c_str());
    }

    if (!err.empty()) {
        printf("Err: %s\n", err.c_str());
    }

    if (!ret) {
        printf("Failed to parse glTF\n");
        return -1;
    }

    // Assume the root node is the scene
    int rootNodeIndex = model.defaultScene >= 0 ? model.defaultScene : 0;
    processNode(rootNodeIndex);

    return 0;
}

void GLTFLoader::processNode(int nodeIndex)
{
    const tinygltf::Node& node = model.nodes[nodeIndex];
    // nodes also have transformations but im going to ignore that for now

    // Process mesh data if the node has a mesh
    if (node.mesh >= 0)
    {
        const tinygltf::Mesh &mesh = model.meshes[node.mesh];

        // Process each primitive in the mesh
        for (const auto &primitive : mesh.primitives)
        {
            if (primitive.mode != TINYGLTF_MODE_TRIANGLES)
            {
                std::cout << "Not triangles\n";
                continue;
            }
            // TODO: need to loop through all the attributes (switch maybe for defined types such as position, normal, texcoord, etc.)
            // then each gets extracted into their own separate vulkan buffers (since the order in the gltf file might not match what i want)
            // these buffers will be contained in some sort of model or node class (depending on how i want to structure it / how many gltf features
            // i want)

            /*// Extract vertex data
            const tinygltf::Accessor &accessor = model.accessors[primitive.attributes.at("POSITION")];
            const tinygltf::BufferView &bufferView = model.bufferViews[accessor.bufferView];
            const tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];

            const float* vertices = reinterpret_cast<const float*>(&buffer.data[0] + bufferView.byteOffset + accessor.byteOffset);
            int vertexCount = accessor.count;

            // Iterate through vertices
            for (int i = 0; i < vertexCount; ++i)
            {
                float x = vertices[i * 3];
                float y = vertices[i * 3 + 1];
                float z = vertices[i * 3 + 2];

                // Process x, y, z (position) for each vertex
                std::cout << "Vertex " << i << ": (" << x << ", " << y << ", " << z << ")\n";
            }*/
        }
    }

    // Recursively process child nodes
    for (int childIndex : node.children)
    {
        processNode(childIndex);
    }
}
