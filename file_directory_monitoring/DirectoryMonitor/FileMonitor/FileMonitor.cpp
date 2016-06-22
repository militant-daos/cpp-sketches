// FileMonitor.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include "boost\filesystem.hpp"
#include "FileUtils.h"
#include "DirMonitor.h"

namespace fs = boost::filesystem;

int main()
{
    try
    {
        // Recursive enumeration test.
        FilesContainer enumFiles;

        fs::path target("d:\\tmp"); // Change this path to the directory of interest.
        FileUtils::QueryEnumerateDirectory(target, enumFiles);

        // Directory monitoring test.
        DirMonitor monitor(target, [](FilesContainer& files)
        {
            for (auto filePair = files.cbegin(); filePair != files.cend(); ++filePair)
            {
                std::wcout << L"Old name: " << filePair->first << " ; new name: " << filePair->second << std::endl;
            }
        });

        std::cout << "Hit <Enter> to quit\n";
        getchar();
        monitor.SetStop();
        monitor.GetWorkerThread()->join();
    }
    catch (const std::exception& e)
    {
        std::cout << e.what();
        return -1;
    }

    return 0;
}

