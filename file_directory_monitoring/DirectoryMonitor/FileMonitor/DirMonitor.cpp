#include "stdafx.h"

#include "DirMonitor.h"
#include <boost/bind.hpp>

DirMonitor::DirMonitor(const fs::path& dir, std::function<void(FilesContainer&)> notifyCallback) :
    m_dir(dir),
    m_callback(notifyCallback)
{
    m_dirHandle.reset(::CreateFile(dir.c_str(),
        FILE_LIST_DIRECTORY,
        FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
        NULL));

    if (INVALID_HANDLE_VALUE == m_dirHandle.get())
    {
        throw std::runtime_error("Cannot create handle for given dir.");
    }

    m_worker.reset(new boost::thread(boost::bind(&DirMonitor::TreadFunc, this)));
}

void DirMonitor::TreadFunc()
{
    m_evtHandles.fsNotifyHandle = FindFirstChangeNotification(
        m_dir.c_str(),
        TRUE,                         // watch subtree 
        FILE_NOTIFY_CHANGE_FILE_NAME);

    m_evtHandles.stopHandle = ::CreateEvent(NULL, TRUE, FALSE, TEXT("StopEvent"));

    std::vector<unsigned char> buf(sizeof(FILE_NOTIFY_INFORMATION) * 1024);
    DWORD dwBytesReturned = 0;
    OVERLAPPED overlapped;
    ::ZeroMemory(&overlapped, sizeof(OVERLAPPED));

    // Init dir. changes state.
    ReadDirectoryChangesW(m_dirHandle.get(), (LPVOID)&buf.at(0), buf.size(), TRUE, 
        FILE_NOTIFY_CHANGE_FILE_NAME, &dwBytesReturned, &overlapped, NULL);

    FilesContainer files;

    while (true)
    {
        DWORD dwWaitStatus = WaitForMultipleObjects(sizeof(m_evtHandles) / sizeof(HANDLE), 
            reinterpret_cast<HANDLE*>(&m_evtHandles), FALSE, INFINITE);

        switch (dwWaitStatus)
        {
        case WAIT_OBJECT_0: // NotifyChange

            files.clear();
            if (ReadDirectoryChangesW(m_dirHandle.get(), (LPVOID)&buf.at(0), buf.size(), TRUE, FILE_NOTIFY_CHANGE_FILE_NAME, 
                &dwBytesReturned, &overlapped, NULL))
            {
                size_t nextOffset = 0;
                PFILE_NOTIFY_INFORMATION itemPtr = reinterpret_cast<PFILE_NOTIFY_INFORMATION>(&buf.at(0));
                do
                {
                    itemPtr = reinterpret_cast<PFILE_NOTIFY_INFORMATION>(buf.data() + nextOffset);
                    std::wstring changedFile(itemPtr->FileName, (itemPtr->FileNameLength / sizeof(wchar_t)));
                    
                    nextOffset += itemPtr->NextEntryOffset;

                    itemPtr = reinterpret_cast<PFILE_NOTIFY_INFORMATION>(buf.data() + nextOffset);
                    std::wstring newFileName(itemPtr->FileName, (itemPtr->FileNameLength / sizeof(wchar_t)));
                  
                    files[fs::path(changedFile).filename()] = fs::path(newFileName).filename();

                    nextOffset += itemPtr->NextEntryOffset;

                } 
                while (NULL != itemPtr->NextEntryOffset);
            }
            
            m_callback(files);

            if (NULL == FindNextChangeNotification(m_evtHandles.fsNotifyHandle))
            {
                throw std::runtime_error("Cannot re-initialize ChangeNotification handle.");
            }

            break;

        case WAIT_OBJECT_0 + 1: // Stop evt.
            return;

        default:
            break;
        }
    }
}

DirMonitor::~DirMonitor()
{
    if (m_evtHandles.fsNotifyHandle && m_evtHandles.fsNotifyHandle != INVALID_HANDLE_VALUE)
    {
        FindCloseChangeNotification(m_evtHandles.fsNotifyHandle);
    }

    if (m_evtHandles.stopHandle && m_evtHandles.stopHandle != INVALID_HANDLE_VALUE)
    {
        FindCloseChangeNotification(m_evtHandles.stopHandle);
    }
}

void DirMonitor::Stop()
{
    SetEvent(m_evtHandles.stopHandle);
    m_worker->join();
}