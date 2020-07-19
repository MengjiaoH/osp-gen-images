#pragma once

#include <vector>

#include "ospray/ospray_cpp.h"
#include "ospray/ospray_util.h"
#include "rkcommon/math/vec.h"
#include "rkcommon/math/box.h"

#include "load_raw.h"

using namespace rkcommon::math;

ospray::cpp::Volume createStructuredVolume(const Volume volume)
{
  ospray::cpp::Volume osp_volume("structuredRegular");

  auto voxels = *(volume.voxel_data);

  osp_volume.setParam("gridOrigin", vec3f(-volume.dims.x/ 2.f, -volume.dims.y/2.f, -volume.dims.z/2.f));
  osp_volume.setParam("gridSpacing", vec3f(1.f));
  osp_volume.setParam("data", ospray::cpp::CopiedData(voxels.data(), volume.dims));
  osp_volume.commit();
  return osp_volume;
}

