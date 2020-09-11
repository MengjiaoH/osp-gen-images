// ======================================================================== //
// Copyright 2017-2019 Intel Corporation                                    //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

#pragma once


#include "rkcommon/math/rkmath.h"
#include "rkcommon/math/AffineSpace.h"
#include "rkcommon/math/vec.h"

class ArcballCamera
{
 public:
  ArcballCamera(const rkcommon::math::box3f &worldBounds,
                const rkcommon::math::vec2i &windowSize);

  // All mouse positions passed should be in [-1, 1] normalized screen coords
  void rotate(const rkcommon::math::vec2f &from, const rkcommon::math::vec2f &to);
  void zoom(float amount);
  void pan(const rkcommon::math::vec2f &delta);

  rkcommon::math::vec3f eyePos() const;
  rkcommon::math::vec3f center() const;
  rkcommon::math::vec3f lookDir() const;
  rkcommon::math::vec3f upDir() const;

  void updateWindowSize(const rkcommon::math::vec2i &windowSize);
  rkcommon::math::vec2f worldToPixel(rkcommon::math::vec3f worldPos, rkcommon::math::vec2i imgSize);

 protected:
  void updateCamera();

  // Project the point in [-1, 1] screen space onto the arcball sphere
  rkcommon::math::quaternionf screenToArcball(const rkcommon::math::vec2f &p);

  float zoomSpeed;
  rkcommon::math::vec2f invWindowSize;
  rkcommon::math::AffineSpace3f centerTranslation, translation, invCamera;
  rkcommon::math::quaternionf rotation;

};
