#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <dirent.h>
#ifdef _WIN32
#define NOMINMAX
#include <malloc.h>
#else
#include <alloca.h>
#endif

#include <vector>
#include <fstream>

#include "ospray/ospray_cpp.h"
#include "ospray/ospray_cpp/ext/rkcommon.h"

using namespace rkcommon::math;

#include "load_raw.h"
#include "parseArgs.h"
#include "make_ospvolume.h"
#include "make_tf.h"
#include "load_camera.h"
#include "ArcballCamera.h"
#include "ParamReader.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

const std::string voxel_type = "float32";

int main(int argc, const char **argv)
{
    //initialize ospray
    OSPError init_error = ospInit(&argc, argv);
    if (init_error != OSP_NO_ERROR)
        return init_error;

    // parse Args
    Args args;
    parseArgs(argc, argv, args);
    // std::cout << "debug" << std::endl;
    // load volume
    // const vec3i dims{args.dims, args.dims*2, args.dims};
    const vec3i dims{768, 336, 512};
    Volume volume = load_raw_volume(args.filename, dims, voxel_type);
    volume.dims = dims;

    // std::cout << "debug 0" << std::endl;
    // load view file
    std::unique_ptr<ParamReader> p_reader(new ParamReader(args.view_file));
    std::cout << p_reader->params.size() << std::endl;
    std::vector<VolParam> params = p_reader->params;

    std::string out_dir = args.out_dir;
    // std::cout << "debug 1" << std::endl;
    
    // Imgae size 
    vec2i imgSize;
    imgSize.x = 256; // width
    imgSize.y = 256; // height
    
    {
        // //! Transfer function
        const std::string colormap = "jet";
        ospray::cpp::TransferFunction transfer_function = makeTransferFunction(colormap, volume.range);

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
        renderer.setParam("aoSamples", 10);
        renderer.setParam("shadows", true);
        renderer.setParam("pixelSamples", 2);
        renderer.setParam("backgroundColor", 1.0f); // white, transparent
        renderer.commit();

        // create and setup framebuffer
        ospray::cpp::FrameBuffer framebuffer(imgSize.x, imgSize.y, OSP_FB_SRGBA, OSP_FB_COLOR | OSP_FB_ACCUM);
        framebuffer.clear();

        for(int i = 0; i < params.size(); i++){
            std::cout << "index " << i << std::endl;
            framebuffer.clear();
            //create and setup camera
            // std::cout << "debug0" << std::endl;
            Camera c = gen_cameras_from_vtk(params[i], volume);
            // std::cout << "debug1" << std::endl;
            ospray::cpp::Camera camera("perspective");
            camera.setParam("aspect", imgSize.x / (float)imgSize.y);
            camera.setParam("position", c.pos);
            camera.setParam("direction", c.dir);
            camera.setParam("up", c.up);
            camera.setParam("fovy", c.fovy);
            camera.commit(); // commit each object to indicate modifications are done
            // render 10 more frames, which are accumulated to result in a better
            // converged image
            for (int frames = 0; frames < 100; frames++)
                framebuffer.renderFrame(renderer, camera, world);

            uint32_t *fb = (uint32_t *)framebuffer.map(OSP_FB_COLOR);
            // std::cout << "file dir " << f.fileDir << std::endl;
            std::string filename = out_dir + "/png/" + "volume_cam" + std::to_string(i)  + ".png";
            std::string jpg_filename = out_dir + "/jpg/" + "volume_cam_" + std::to_string(i)  + ".jpg";
            // + "_" + std::to_string(index)
            // std::cout << filename << std::endl;
            stbi_write_png(filename.c_str(), imgSize.x, imgSize.y, 4, fb, imgSize.x * 4);
            stbi_write_jpg(jpg_filename.c_str(), imgSize.x, imgSize.y, 4, fb, 100);
            framebuffer.unmap(fb);

        }
    }

    
    
    ospShutdown();
    return 0;
}