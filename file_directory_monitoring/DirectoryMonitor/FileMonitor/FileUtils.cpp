#include "stdafx.h"
#include "FileUtils.h"

void FileUtils::QueryEnumerateDirectory(const fs::path& directory, FilesContainer& fileList)
{
    if (directory.empty())
    {
        return;
    }

    fs::recursive_directory_iterator dirWalker(directory);
    fs::recursive_directory_iterator endMark;

    while (dirWalker != endMark)
    {
        if (fs::is_regular_file(dirWalker->path()))
        {
            fileList[dirWalker->path().filename()] = dirWalker->path();
        }
        ++dirWalker;
    }
}