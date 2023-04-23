#include "ProgramOpt.h"
#include <iostream>
int main(int argc, char* argv[])
{

    ProgramOpt options;
    options.addOptions("help,h", "list all options");
    options.addOptionsDefault<int>("default,d", 10, "test default value");
    options.addOptionsImplicit<std::string>("implict,i", "impl", "test implict value");
    options.addOptionsMultiToken<int>("mult,m", "test multiToken");

    if(options.parse(argc, argv))
    {
        if(options.hasParam("h") || options.hasParam("help"))
            std::cout << options.getOpts() << std::endl;
    }
    else
    {
        std::cout << "parse program options error, input \"-h\" or \"--help\" for help" << std::endl;
    }

    std::cout << "initial path: " << FileSystem::InitialPath() << std::endl;
    std::cout << "current path: " << FileSystem::WorkPath() << std::endl;

    
    if(options.hasParam("default"))   std::cout << "default: " << options.getArgs()["default"].as<int>() << std::endl;
    if(options.hasParam("d"))   std::cout << "default: " << options.getArgs()["d"].as<int>() << std::endl;
    if(options.hasParam("implict"))   std::cout << "implict: " << options.getArgs()["implict"].as<std::string>() << std::endl;
    if(options.hasParam("i"))   std::cout << "implict: " << options.getArgs()["i"].as<std::string>() << std::endl;
    if(options.hasParam("mult"))
    {
        std::cout << "multi token: ";
        for(auto& i: options.getArgs()["mult"].as<std::vector<int>>())
            std::cout << i << " ";
        std::cout << std::endl;
    }

    if(options.hasParam("m"))
    {
        std::cout << "multi token: ";
        for(auto& i: options.getArgs()["m"].as<std::vector<int>>())
            std::cout << i << " ";
        std::cout << std::endl;
    }
    return 0;
}