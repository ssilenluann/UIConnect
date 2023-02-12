#ifndef UTILS_FILESYSTEM_H
#define UTILS_FILESYSTEM_H

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <boost/filesystem.hpp>
#include <fstream>

class FileSystem
{
public:
    static bool OpenForRead(std::ifstream& ifs, const std::string& filename
                    ,std::ios_base::openmode mode);
    static bool OpenForWrite(std::ofstream& ofs, const std::string& filename
                    ,std::ios_base::openmode mode);
    static bool Mkdir(const std::string& dirname);
    static std::string Dirname(const std::string& filename);
    static std::string Filename(const std::string& filename);
    static void GetAllFile(const std::string &directory, const std::string& exe, std::list<std::string>& fileList);

    static int MkLimitedDir(const char* dirname);
    static int __lstat(const char* file, struct stat* st = nullptr);

    static std::string InitialPath();
    static std::string WorkPath();
    static void WorkPath(const std::string& path);
};

#endif