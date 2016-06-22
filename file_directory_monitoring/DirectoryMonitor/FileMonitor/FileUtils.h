#pragma once

#include <map>
#include <boost\filesystem.hpp>

namespace fs = boost::filesystem;

typedef std::map<fs::path, fs::path> FilesContainer;

class FileUtils
{
public:
    static void QueryEnumerateDirectory(const fs::path& directory, FilesContainer& fileList);
};