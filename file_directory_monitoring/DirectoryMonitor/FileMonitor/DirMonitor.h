#pragma once

#include <boost\filesystem.hpp>
#include <functional>
#include <boost\thread.hpp>
#include "HandleGuard.h"
#include "FileUtils.h"

namespace fs = boost::filesystem;

class DirMonitor
{
public:
    DirMonitor(const fs::path& dir, std::function<void(FilesContainer&)> notifyCallback);
    ~DirMonitor();
    void SetStop();
    boost::thread* GetWorkerThread();

private:
    void TreadFunc();

private:
    HandleGuard m_dirHandle;
    HANDLE m_auxHandles[2];
    fs::path m_dir;
    std::function<void(FilesContainer&)> m_callback;
    std::unique_ptr<boost::thread> m_worker;
};