#pragma once

#include "gltf/tiny_gltf.h"

class GLTFLoader
{
private:
    tinygltf::Model model;
    void processNode(int nodeIndex);
public:
    GLTFLoader();
    int load();
};
