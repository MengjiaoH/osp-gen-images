#include <iostream>
#include <dirent.h>
#include <vector>

#include "parseArgs.h"
#include "load_raw.h"
#include <ospray/ospray_cpp.h>
#include "ospray/ospray_cpp/ext/rkcommon.h"

using namespace rkcommon::math;
const std::string voxel_type = "float32";

// Load all files and calculate the range of 
// entire time series for generating transfer function 

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
                const int timestep = std::stoi(name.substr(10, name.find(".") - 10));
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
    const vec3i dims{args.dims, args.dims, args.dims};

    int count = 500;
    for(auto f : files){
        // if(f.timeStep % count == 0){
            volumes.push_back(load_raw_volume(f.fileDir, dims, voxel_type));
        // }
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



}