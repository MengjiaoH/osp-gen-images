#pragma once

#include <vector>

#include "ospray/ospray_cpp.h"
#include "ospray/ospray_util.h"
#include "rkcommon/math/vec.h"
#include "rkcommon/math/box.h"

using namespace rkcommon::math;

ospray::cpp::TransferFunction makeTransferFunction(const std::string tfColorMap, const vec2f &valueRange)
{
    ospray::cpp::TransferFunction transferFunction("piecewiseLinear");

    std::vector<vec3f> colors;
    std::vector<float> opacities;

    if (tfColorMap == "jet") {
        colors.emplace_back(0, 0, 0.562493);
        colors.emplace_back(0, 0, 1);
        colors.emplace_back(0, 1, 1);
        colors.emplace_back(0.500008, 1, 0.500008);
        colors.emplace_back(1, 1, 0);
        colors.emplace_back(1, 0, 0);
        colors.emplace_back(0.500008, 0, 0);
    } else if (tfColorMap == "rgb") {
        colors.emplace_back(0, 0, 1);
        colors.emplace_back(0, 1, 0);
        colors.emplace_back(1, 0, 0);
    } else {
        colors.emplace_back(0.f, 0.f, 0.f);
        colors.emplace_back(1.f, 1.f, 1.f);
    }

    std::string tfOpacityMap = "linear";

    if (tfOpacityMap == "linear") {
        opacities.emplace_back(0.f);
        opacities.emplace_back(1.f);
    }

    transferFunction.setParam("color", ospray::cpp::CopiedData(colors));
    transferFunction.setParam("opacity", ospray::cpp::CopiedData(opacities));
    transferFunction.setParam("valueRange", valueRange);
    transferFunction.commit();

    return transferFunction;
}