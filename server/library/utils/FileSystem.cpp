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
    if(!ofs.is_open()) 
    {
        std::string dir = Dirname(filename);
        Mkdir(dir);
        ofs.open(filename.c_str(), mode);
    }
    return ofs.is_open();
}

bool FileSystem::Mkdir(const std::string& dirname) 
{
    if(boost::filesystem::exists(dirname)) return true;
    return boost::filesystem::create_directories(dirname);
}

std::string FileSystem::Dirname(const std::string &filename)
{
    // if(!boost::filesystem::exists(filename))    return "";
    return boost::filesystem::path(filename).parent_path().string();
}

std::string FileSystem::Filename(const std::string& filename)
{
    if(!boost::filesystem::exists(filename))    return "";
    return boost::filesystem::path(filename).filename().string();
}

void FileSystem::GetAllFileName(const std::string &directory, const std::string& exe, std::list<std::string>& fileList)
{
    boost::filesystem::path path(directory);
    if(!boost::filesystem::exists(directory) || !boost::filesystem::is_directory(path)) return;

    for (boost::filesystem::directory_entry& file : boost::filesystem::directory_iterator(path)) 
    {
        if (boost::filesystem::is_regular_file(file.status()) && file.path().extension() == exe) 
        {
            fileList.push_back(file.path().filename().string());
        }
    }
}

void FileSystem::GetAllFilePath(const std::string &directory, const std::string& exe, std::list<std::string>& fileList)
{
    boost::filesystem::path path(directory);
    if(!boost::filesystem::exists(directory) || !boost::filesystem::is_directory(path)) return;

    for (boost::filesystem::directory_entry& file : boost::filesystem::directory_iterator(path)) 
    {
        if (boost::filesystem::is_regular_file(file.status()) && file.path().extension() == exe) 
        {
            fileList.push_back(file.path().string());
        }
    }
}

int FileSystem::__lstat(const char *file, struct stat *st)
{
    struct stat lst;
    int ret = lstat(file, &lst);
    if(st) 
    {
        *st = lst;
    }
    return ret;
}

int FileSystem::MkLimitedDir(const char* dirname) 
{
    if(access(dirname, F_OK) == 0) 
    {
        return 0;
    }
    return mkdir(dirname, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
}

std::string FileSystem::InitialPath()
{
    return boost::filesystem::initial_path().string();
}

std::string FileSystem::WorkPath()
{
    return boost::filesystem::current_path().string();
}

void FileSystem::WorkPath(const std::string &str)
{
    boost::filesystem::current_path(str);
}

#endif