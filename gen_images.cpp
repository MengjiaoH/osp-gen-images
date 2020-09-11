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

using namespace rkcommon::math;

#include "load_raw.h"
#include "parseArgs.h"
#include "make_ospvolume.h"
#include "make_tf.h"
#include "load_camera.h"
#include "ArcballCamera.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

const std::string voxel_type = "float32";
// const vec2f range{-1.0f, 1.0f};

template<typename T>
std::vector<T> s(std::vector<T> const &v, int m, int n) {
   auto first = v.begin() + m;
   auto last = v.begin() + n + 1;
   std::vector<T> vector(first, last);
   return vector;
}

int main(int argc, const char **argv)
{
    //initialize ospray
    OSPError init_error = ospInit(&argc, argv);
    if (init_error != OSP_NO_ERROR)
        return init_error;

    // parse Args
    Args args;
    parseArgs(argc, argv, args);
    
    // load cameras info 
    std::fstream infile("input.txt");
    if(!infile.is_open()) 
        std::cout << "Cannot read file" << "\n";

    std::string line;
    std::vector<std::vector<float>> v;
    int i = 0;

    while (std::getline(infile, line)){
        float value;
        std::stringstream ss(line);
        v.push_back(std::vector<float>());
        while (ss >> value){
            v[i].push_back(value);
        }
        ++i;
    }
    // std::cout << "v size " << v.size() << std::endl;
    std::vector<Camera> cameras;
    for (int i = 0; i < v.size(); i++){
        std::vector<float> temp = v[i];
        vec3f pos{temp[0], temp[1], temp[2]};
        vec3f dir{temp[3], temp[4], temp[5]};
        vec3f up{temp[6], temp[7], temp[8]};
        cameras.emplace_back(pos, dir, up);
    }
    // debug
    for(int i = 0; i < cameras.size(); i++){
        std::cout << "index: " << i << "\n";
        std::cout << "pos: " << cameras[i].pos << "\n";
        std::cout << "dir: " << cameras[i].dir << "\n";
        std::cout << "up: " << cameras[i].up << "\n";
    }
    // load all volume files 
    std::vector<timesteps> files;
    for(const auto &dir : args.timeStepPaths){
        DIR *dp = opendir(dir.c_str());
        if (!dp) {
            throw std::runtime_error("failed to open directory: " + dir);
        }
        for(dirent *e = readdir(dp); e; e = readdir(dp)){
            std::string name = e ->d_name;
            // std::cout << "name " << name << std::endl;
            if(name.length() > 3){
                // std::cout << name.substr(10, name.find(".") - 10) << std::endl;
                const int timestep = std::stoi(name.substr(10, name.find(".") - 10));
                const std::string filename = dir + "/" + name;
                std::cout << filename << " " << timestep << std::endl;
                timesteps t(timestep, filename);
                files.push_back(t);
            }
        }
    }
    // Sort time steps 
    std::sort(files.begin(), files.end(), sort_timestep());

    // Imgae size 
    vec2i imgSize;
    imgSize.x = 256; // width
    imgSize.y = 256; // height

    int index = 1;

    // for each file render images with varying camera position 
    for(auto f : files){
        // timesteps f = files[0];
        std::cout << "volume file : " << f.fileDir << std::endl;
        Volume volume;
        const vec3i dims{args.dims, args.dims, args.dims};
        volume = load_raw_volume(f.fileDir, dims, voxel_type);
        // box3f worldBound = box3f(-dims / 2 * volume.spacing, dims / 2 * volume.spacing);
        // ArcballCamera arcballCamera(worldBound, imgSize);
        // vec3f cam_pos = vec3f{200.f, 0.f, 0.f};
        vec2f range = vec2f{-2.92272, 0.407719};

        // std::cout << "camera pos " << arcballCamera.eyePos() << std::endl;
        // std::cout << "camera look dir " << arcballCamera.lookDir() << std::endl;
        // std::cout << "camera up dir " << arcballCamera.upDir() << std::endl;
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
            renderer.setParam("pixelSamples", 10);
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
                for (int frames = 0; frames < 100; frames++)
                    framebuffer.renderFrame(renderer, camera, world);

                uint32_t *fb = (uint32_t *)framebuffer.map(OSP_FB_COLOR);
                // std::cout << "file dir " << f.fileDir << std::endl;
                std::string filename = "volume_ts" + std::to_string(f.timeStep)+ "_cam" + std::to_string(i)  + ".jpg";
                // + "_" + std::to_string(index)
                // std::cout << filename << std::endl;
                stbi_write_jpg(filename.c_str(), imgSize.x, imgSize.y, 4, fb, 100);
                framebuffer.unmap(fb);
                // index++;
            }

        }
    }
    ospShutdown();
    

    return 0;

}