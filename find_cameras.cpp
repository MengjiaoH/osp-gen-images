#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#ifdef _WIN32
#define NOMINMAX
#include <malloc.h>
#else
#include <alloca.h>
#endif

#include <vector>
#include <fstream>

#include "ospray/ospray_cpp.h"

using namespace rkcommon::math;

#include "load_raw.h"
#include "parseArgs.h"
#include "make_ospvolume.h"
#include "make_tf.h"
#include "load_camera.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

const std::string voxel_type = "float32";
const vec2f range{-2.92272, 0.407719};
// const vec2f range{-1.0f, 1.0f};

int main(int argc, const char **argv)
{
    //initialize ospray
    OSPError init_error = ospInit(&argc, argv);
    if (init_error != OSP_NO_ERROR)
        return init_error;

    // parse Args
    Args args;
    parseArgs(argc, argv, args);
    std::cout << "volume file : " << args.filename << std::endl;
    std::cout << "dims: " << args.dims << std::endl;
    //load single volume 
    Volume volume;
    const vec3i dims{args.dims, args.dims, args.dims};
    volume = load_raw_volume(args.filename, dims, voxel_type);

    // Imgae size 
    vec2i imgSize;
    imgSize.x = 300; // width
    imgSize.y = 300; // height

    //create a list of camera positions 
    const int num = args.n_samples;
    const box3f worldBound = box3f(-dims / 2 * volume.spacing, dims / 2 * volume.spacing);
    std::vector<Camera> cameras = gen_cameras(num, worldBound);
    std::cout << "camera pos:" << cameras.size() << std::endl;
    
    std::ofstream outfile;
    // save camera to file
    outfile.open("/home/mengjiao/Desktop/projects/osp-gen-images/input.txt");
    for(int i = 0; i < cameras.size(); i++){
        outfile << cameras[i].pos.x << " " <<
                   cameras[i].pos.y << " " <<
                   cameras[i].pos.z << " " <<
                   cameras[i].dir.x << " " <<
                   cameras[i].dir.y << " " <<
                   cameras[i].dir.z << " " << 
                   cameras[i].up.x  << " " << 
                   cameras[i].up.y  << " " << 
                   cameras[i].up.z  << "\n";
    }
    outfile.close();
    
    // debug 
    // for(int i = 0; i < cameras.size(); i++){
    //     std::cout << "index: " << i << "\n";
    //     std::cout << "pos: " << cameras[i].pos << "\n";
    //     std::cout << "dir: " << cameras[i].dir << "\n";
    //     std::cout << "up: " << cameras[i].up << "\n";
    // }
    {
        // //! Transfer function
        const std::string colormap = "jet";
        ospray::cpp::TransferFunction transfer_function = makeTransferFunction(colormap, range);

        //! Volume
        ospray::cpp::Volume osp_volume = createStructuredVolume(volume);
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
        renderer.setParam("aoSamples", 0);
        renderer.setParam("backgroundColor", 1.0f); // white, transparent
        renderer.commit();

        // create and setup framebuffer
        ospray::cpp::FrameBuffer framebuffer(imgSize.x, imgSize.y, OSP_FB_SRGBA, OSP_FB_COLOR | OSP_FB_ACCUM);
        framebuffer.clear();

        for(int i = 0; i < cameras.size(); i++){
            framebuffer.clear();
            //create and setup camera
            ospray::cpp::Camera camera("perspective");
            camera.setParam("aspect", imgSize.x / (float)imgSize.y);
            camera.setParam("position", cameras[i].pos);
            camera.setParam("direction", cameras[i].dir);
            camera.setParam("up", cameras[i].up);
            camera.commit(); // commit each object to indicate modifications are done
            // render 10 more frames, which are accumulated to result in a better
            // converged image
            for (int frames = 0; frames < 10; frames++)
                framebuffer.renderFrame(renderer, camera, world);

            uint32_t *fb = (uint32_t *)framebuffer.map(OSP_FB_COLOR);
            std::string filename = "volume" + std::to_string(i+1) + ".jpg";
            // std::cout << filename << std::endl;
            stbi_write_jpg(filename.c_str(), imgSize.x, imgSize.y, 4, fb, 100);
            framebuffer.unmap(fb);
        }

    }

    ospShutdown();
    return 0;

}