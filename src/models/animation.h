#pragma once

#include <vector>
#include "node.h"
#include "tsl/robin_map.h"

enum TargetPath
{
    TRANSLATION,
    ROTATION,
    SCALE,
};

enum Interpolation
{
    LINEAR,
    STEP,
    CUBICSPLINE,
};

struct Channel
{
    Node *node;
    int sampler;
    TargetPath path;
};

struct Sampler
{
    int input;
    int output;
    Interpolation interpolation;
};

struct Animation
{
    std::string name;

    std::vector<Channel> channels;
    std::vector<Sampler> samplers;

    explicit Animation(std::string &name) : name(name) {}
};
