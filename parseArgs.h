#pragma once


#include <iostream>
#include <vector>

struct Args
{
    std::string extension;
    std::string filename;
    std::string view_file;
    std::string opacity_file;
    std::string color_file;
    std::string out_dir;
    std::vector<std::string> timeStepPaths;
    std::string variableName;
    int timeStep = 0;
    int dims = 0;
    int n_samples = 100;
};

std::string getFileExt(const std::string& s) 
{

   size_t i = s.rfind('.', s.length());
   if (i != std::string::npos) {
      return(s.substr(i+1, s.length() - i));
   }

   return("");
}

void parseArgs(int argc, const char **argv, Args &args)
{
    for(int i = 0; i < argc; i++)
    {
        std::string arg = argv[i];
        if(arg == "-f"){
            args.filename = argv[++i];
        }else if(arg=="-view"){
            args.view_file = argv[++i];
        }else if(arg=="-op"){
            args.opacity_file = argv[++i];
        }else if(arg=="-color"){
            args.color_file = argv[++i];
        }else if(arg=="-out_dir"){
            args.out_dir = argv[++i];
        }else if(arg == "-time-step"){
            args.timeStep = std::atoi(argv[++i]);
        }else if(arg == "-variable"){
            args.variableName = argv[++i];
        }else if(arg == "-dims"){
            args.dims = std::atoi(argv[++i]);
        }else if(arg == "-n_samples"){
            args.n_samples = std::atoi(argv[++i]);
        }else if(arg == "-multi-ts"){
            for(; i + 1 < argc; ++i){
                if(argv[i+1][0] == '-'){
                    break;
                }
                args.timeStepPaths.push_back(argv[++i]);
            }
        }
    }
    // find file extension
    std::string extension = getFileExt(args.filename);
    args.extension = extension;
}