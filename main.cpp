// Copyright 2009-2020 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

/* This is a small example tutorial how to use OSPRay in an application.
 *
 * On Linux build it in the build_directory with
 *   g++ ../apps/ospTutorial/ospTutorial.cpp -I ../ospray/include \
 *       -I ../../rkcommon -L . -lospray -Wl,-rpath,. -o ospTutorial
 * On Windows build it in the build_directory\$Configuration with
 *   cl ..\..\apps\ospTutorial\ospTutorial.cpp /EHsc -I ..\..\ospray\include ^
 *      -I ..\.. -I ..\..\..\rkcommon ospray.lib
 */

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <iostream>
#include <dirent.h>
#ifdef _WIN32
#define NOMINMAX
#include <malloc.h>
#else
#include <alloca.h>
#endif

#include <vector>

#include <ospray/ospray_cpp.h>

#include "load_raw.h"
#include "parseArgs.h"
#include "make_ospvolume.h"
#include "make_tf.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

using namespace rkcommon::math;
const std::string voxel_type = "float32";
const vec3f dims{128, 128, 128};

int main(int argc, const char **argv)
{
    // parse Args
    Args args;
    parseArgs(argc, argv, args);

    // load data
    std::vector<timesteps> files;
    for(const auto &dir : args.timeStepPaths){
        DIR *dp = opendir(dir.c_str());
        if (!dp) {
            throw std::runtime_error("failed to open directory: " + dir);
        }
        for(dirent *e = readdir(dp); e; e = readdir(dp)){
            std::string name = e ->d_name;
            if(name.length() > 3){
                const int timestep = std::stoi(name.substr(6, name.find(".") - 6));
                const std::string filename = dir + "/" + name;
                // std::cout << filename << " " << timestep << std::endl;
                timesteps t(timestep, filename);
                files.push_back(t);
            }
        }
    }
    // Sort time steps 
    std::sort(files.begin(), files.end(), sort_timestep());

    // load volumes 
    std::vector<Volume> volumes;

    int count = 3;
    for(auto f : files){
        if(f.timeStep % count == 0){
            volumes.push_back(load_raw_volume(f.fileDir, dims, voxel_type));
        }
    }    

    box3f worldBound = box3f(-dims / 2 * volumes[0].spacing, dims / 2 * volumes[0].spacing);
    vec2f range; 
    Volume &temp1 = *std::min_element(volumes.begin(), volumes.end(), [](Volume &v1, Volume &v2){
        return v1.range.x < v2.range.x;
    });
    Volume &temp2 = *std::max_element(volumes.begin(), volumes.end(), [](Volume &v1, Volume &v2){
        return v1.range.y > v2.range.y;
    });
    range.x = temp1.range.x; range.y = temp2.range.y;
    std::cout << "global range: " << range << std::endl;
    
    // image size
    vec2i imgSize;
    imgSize.x = 64; // width
    imgSize.y = 64; // height

    // camera 
    // TODO: need to change positions
    vec3f cam_pos{0.f, 0.f, -90.f};
    vec3f cam_up{0.f, 1.f, 0.f};
    vec3f cam_view{0.f, 0.f, 64.f};

    // initialize OSPRay; OSPRay parses (and removes) its commandline parameters,
    // e.g. "--osp:debug"
    OSPError init_error = ospInit(&argc, argv);
    if (init_error != OSP_NO_ERROR)
    return init_error;

    // use scoped lifetimes of wrappers to release everything before ospShutdown()
    {
        for(int i = 0; i < volumes.size(); i++){
            // create and setup camera
            ospray::cpp::Camera camera("perspective");
            camera.setParam("aspect", imgSize.x / (float)imgSize.y);
            camera.setParam("position", cam_pos);
            camera.setParam("direction", cam_view);
            camera.setParam("up", cam_up);
            camera.commit(); // commit each object to indicate modifications are done

            // //! Transfer function
            const std::string colormap = "jet";
            ospray::cpp::TransferFunction transfer_function = makeTransferFunction(colormap, range);

            //! Volume
            ospray::cpp::Volume osp_volume = createStructuredVolume(volumes[i]);
            //! Volume Model
            ospray::cpp::VolumetricModel volume_model(osp_volume);
            volume_model.setParam("transferFunction", transfer_function);
            volume_model.commit();
            // put the model into a group (collection of models)
            ospray::cpp::Group group;
            group.setParam("volume", ospray::cpp::CopiedData(volume_model));
            group.commit();

            // put the group into an instance (give the group a world transform)
            ospray::cpp::Instance instance(group);
            instance.commit();

            // put the instance in the world
            ospray::cpp::World world;
            world.setParam("instance", ospray::cpp::CopiedData(instance));

            // create and setup light for Ambient Occlusion
            ospray::cpp::Light light("ambient");
            light.commit();

            world.setParam("light", ospray::cpp::CopiedData(light));
            world.commit();

            // create renderer, choose Scientific Visualization renderer
            ospray::cpp::Renderer renderer("scivis");

            // complete setup of renderer
            renderer.setParam("aoSamples", 1);
            renderer.setParam("backgroundColor", 1.0f); // white, transparent
            renderer.commit();

            // create and setup framebuffer
            ospray::cpp::FrameBuffer framebuffer(imgSize.x, imgSize.y, OSP_FB_SRGBA, OSP_FB_COLOR | OSP_FB_ACCUM);
            framebuffer.clear();

            // render 10 more frames, which are accumulated to result in a better
            // converged image
            for (int frames = 0; frames < 10; frames++)
                framebuffer.renderFrame(renderer, camera, world);

            uint32_t *fb = (uint32_t *)framebuffer.map(OSP_FB_COLOR);
            std::string filename = "volume" + std::to_string(i) + ".jpg";
            // std::cout << filename << std::endl;
            stbi_write_jpg(filename.c_str(), imgSize.x, imgSize.y, 4, fb, 100);
            // writePPM(filename.c_str(), imgSize, fb);
            framebuffer.unmap(fb);
        }
    }

    ospShutdown();

    return 0;
}
