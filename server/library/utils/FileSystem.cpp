#ifndef UTILS_FILESYSTEM_HPP
#define UTILS_FILESYSTEM_HPP

#include "FileSystem.h"

bool FileSystem::OpenForRead(std::ifstream& ifs, const std::string& filename
                    ,std::ios_base::openmode mode) 
{
    ifs.open(filename.c_str(), mode);
    return ifs.is_open();
}

bool FileSystem::OpenForWrite(std::ofstream& ofs, const std::string& filename
                    ,std::ios_base::openmode mode) 
{
    ofs.open(filename.c_str(), mode);   
    if(!ofs.is_open()) {
        std::string dir = Dirname(filename);
        Mkdir(dir);
        ofs.open(filename.c_str(), mode);
    }
    return ofs.is_open();
}

bool FileSystem::Mkdir(const std::string& dirname) {
    if(__lstat(dirname.c_str()) == 0) {
        return true;
    }
    char* path = strdup(dirname.c_str());
    char* ptr = strchr(path + 1, '/');
    do {
        for(; ptr; *ptr = '/', ptr = strchr(ptr + 1, '/')) {
            *ptr = '\0';
            if(__mkdir(path) != 0) {
                break;
            }
        }
        if(ptr != nullptr) {
            break;
        } else if(__mkdir(path) != 0) {
            break;
        }
        free(path);
        return true;
    } while(0);
    free(path);
    return false;
}

std::string FileSystem::Dirname(const std::string& filename) {
    if(filename.empty()) {
        return ".";
    }
    auto pos = filename.rfind('/');
    if(pos == 0) {
        return "/";
    } else if(pos == std::string::npos) {
        return ".";
    } else {
        return filename.substr(0, pos);
    }
}

int FileSystem::__lstat(const char* file, struct stat* st) {
    struct stat lst;
    int ret = lstat(file, &lst);
    if(st) {
        *st = lst;
    }
    return ret;
}

int FileSystem::__mkdir(const char* dirname) {
    if(access(dirname, F_OK) == 0) {
        return 0;
    }
    return mkdir(dirname, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
}
#endif