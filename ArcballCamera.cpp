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

#include "ArcballCamera.h"
#include <math.h>

ArcballCamera::ArcballCamera(const rkcommon::math::box3f &worldBounds,
                             const rkcommon::math::vec2i &windowSize)
    : zoomSpeed(1),
      invWindowSize(rkcommon::math::vec2f(1.0) / rkcommon::math::vec2f(windowSize)),
      centerTranslation(rkcommon::math::one),
      translation(rkcommon::math::one),
      rotation(rkcommon::math::one)
{
  rkcommon::math::vec3f diag = worldBounds.size();
  zoomSpeed             = rkcommon::math::max(length(diag) / 150.0, 0.001);
  diag = rkcommon::math::max(diag, rkcommon::math::vec3f(0.3f * length(diag)));

  centerTranslation =
      rkcommon::math::AffineSpace3f::translate(-worldBounds.center());
  translation =
      rkcommon::math::AffineSpace3f::translate(rkcommon::math::vec3f(0, 0, length(diag)));
  updateCamera();
}

void ArcballCamera::rotate(const rkcommon::math::vec2f &from,
                           const rkcommon::math::vec2f &to)
{
  rotation = screenToArcball(to) * screenToArcball(from) * rotation;
  updateCamera();
}

void ArcballCamera::zoom(float amount)
{
  amount *= zoomSpeed;
  translation =
      rkcommon::math::AffineSpace3f::translate(rkcommon::math::vec3f(0, 0, amount)) *
      translation;
  updateCamera();
}

void ArcballCamera::pan(const rkcommon::math::vec2f &delta)
{
  const rkcommon::math::vec3f t = rkcommon::math::vec3f(
      -delta.x * invWindowSize.x, delta.y * invWindowSize.y, 0);
  const rkcommon::math::vec3f worldt = translation.p.z * xfmVector(invCamera, t);
  centerTranslation =
      rkcommon::math::AffineSpace3f::translate(worldt) * centerTranslation;
  updateCamera();
}

rkcommon::math::vec3f ArcballCamera::eyePos() const
{
  return xfmPoint(invCamera, rkcommon::math::vec3f(0, 0, 1));
}

rkcommon::math::vec3f ArcballCamera::center() const
{
  return -centerTranslation.p;
}

rkcommon::math::vec3f ArcballCamera::lookDir() const
{
  return xfmVector(invCamera, rkcommon::math::vec3f(-1, 0, 0));
}

rkcommon::math::vec3f ArcballCamera::upDir() const
{
  return xfmVector(invCamera, rkcommon::math::vec3f(0, 0, -1));
}

void ArcballCamera::updateCamera()
{
  const rkcommon::math::AffineSpace3f rot    = rkcommon::math::LinearSpace3f(rotation);
  const rkcommon::math::AffineSpace3f camera = translation * rot * centerTranslation;
  invCamera                             = rcp(camera);
}

void ArcballCamera::updateWindowSize(const rkcommon::math::vec2i &windowSize)
{
  invWindowSize = rkcommon::math::vec2f(1) / rkcommon::math::vec2f(windowSize);
}

rkcommon::math::quaternionf ArcballCamera::screenToArcball(
    const rkcommon::math::vec2f &p)
{
  const float dist = dot(p, p);
  // If we're on/in the sphere return the point on it
  if (dist <= 1.f) {
    return rkcommon::math::quaternionf(0, p.x, p.y, std::sqrt(1.f - dist));
  } else {
    // otherwise we project the point onto the sphere
    const rkcommon::math::vec2f unitDir = normalize(p);
    return rkcommon::math::quaternionf(0, unitDir.x, unitDir.y, 0);
  }
}


 rkcommon::math::vec2f ArcballCamera::worldToPixel(rkcommon::math::vec3f worldPos, rkcommon::math::vec2i imgSize)
 {
   rkcommon::math::vec3f N = lookDir();
   rkcommon::math::vec3f U = upDir();
  //  U = cross(U, N);
   rkcommon::math::vec3f V = cross(N, U);
  //  U = cross(V, N);
   
   rkcommon::math::vec3f pos = eyePos();


  //  rkcommon::math::LinearSpace3f cameraToworld = 
  //   rkcommon::math::LinearSpace3f(U.x, V.x, N.x, 
  //                                  U.y, V.y, N.y,
  //                                  U.z, V.z, N.z);

  // rkcommon::math::vec3f cameraSpace = cameraToworld * worldPos;

  rkcommon::math::vec3f v = worldPos - pos;

  rkcommon::math::vec3f r = normalize(v);
  
  const float denom = dot(-r, -N);
  
  if (denom == 0.0) {
    return rkcommon::math::vec2f(-1, -1);
  }
  
  const float t = 1.0 / denom;

  rkcommon::math::vec2f imgPlaneSize;
  float fovy = 60.f;
  float aspect = imgSize.x / (float)imgSize.y;
  imgPlaneSize.y = 2.f * tanf(rkcommon::math::deg2rad(0.5f * fovy));
  imgPlaneSize.x = imgPlaneSize.y * (float)aspect;
  
  V *= imgPlaneSize.x;
  U *= imgPlaneSize.y;

  rkcommon::math::vec3f dir_00 = N - .5f * V - .5f * U;

  const rkcommon::math::vec3f screenDir = r * t - dir_00;
  
  rkcommon::math::vec2f sp = rkcommon::math::vec2f(dot(screenDir, normalize(V) ), dot(screenDir, normalize(U))) / imgPlaneSize;
  const float depth = rkcommon::math::sign(t) * length(v);
  
  // std::cout << "screen pos = (" << sp.x << ", " << sp.y << " depth " << depth << std::endl;

  // sp.x = (sp.x * 0.5f) + 0.5f;
  // sp.y = (sp.y * 0.5f) + 0.5f;

  sp = sp * imgSize;

  // if(sp.x <= 0.f || sp.y <= 0.f){
  //   std::cout << "sp x " << sp.x << " y " << sp.y << " world pos " << worldPos << std::endl;
  // }


  // if(sp.x >= 1.f || sp.y >= 1.f){
  //   std::cout << "sp x " << sp.x << " y " << sp.y << std::endl;
  // }


  // const float depth = sign(t) * length(v);



  return sp;



  // rkcommon::math::vec2f screenPos = rkcommon::math::vec2f(-cameraSpace.x / (float)cameraSpace.z, -cameraSpace.y / (float)cameraSpace.z);
  // rkcommon::math::vec2f rasterPos = rkcommon::math::vec2f((screenPos.x + imgSize.x / 2 ) / imgSize.x, (screenPos.y + imgSize.y / 2 ) / imgSize.y);

  // std::cout << "camera space pos " << cameraSpace.x << " " << cameraSpace.y << " " << cameraSpace.z <<  std::endl;
  // std::cout << "screen space " << screenPos.x << " " << screenPos.y << std::endl;
  
  // return rkcommon::math::vec2f(rasterPos.x * imgSize.x, rasterPos.y * imgSize.y);
 }