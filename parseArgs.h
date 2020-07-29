#pragma once


#include <iostream>
#include <vector>

struct Args
{
    std::string extension;
    std::string filename;
    std::vector<std::string> timeStepPaths;
    std::string variableName;
    int timeStep = 0;
    int dims = 0;
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
        }else if(arg == "-time-step"){
            args.timeStep = std::atoi(argv[++i]);
        }else if(arg == "-variable"){
            args.variableName = argv[++i];
        }else if(arg == "-dims"){
            args.dims = std::atoi(argv[++i]);
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