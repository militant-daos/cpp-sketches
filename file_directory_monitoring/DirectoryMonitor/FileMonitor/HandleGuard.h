#pragma once

#include <Windows.h>

class HandleGuard
{
public:
    explicit HandleGuard(HANDLE handle = NULL) :
        m_handle(handle)
    {
    }

    ~HandleGuard()
    {
        if (m_handle && INVALID_HANDLE_VALUE != m_handle)
        {
            CloseHandle(m_handle);
        }
    }

    HANDLE get()
    {
        return m_handle;
    }

    HANDLE release()
    {
        HANDLE tmp = m_handle;
        m_handle = NULL;
        return tmp;
    }

    void reset(HANDLE newHandle)
    {
        if (m_handle && INVALID_HANDLE_VALUE != m_handle)
        {
            CloseHandle(m_handle);
        }
        m_handle = newHandle;
    }

    void swap(HandleGuard& arg)
    {
        HANDLE tmp = m_handle;
        reset(arg.release());
        arg.reset(tmp);
    }

private:
    HandleGuard(const HandleGuard& rhs);
    HandleGuard(const HandleGuard&& rhs);
    HandleGuard& operator=(const HandleGuard& rhs);
    HandleGuard& operator=(const HandleGuard&& rhs);

private:
    HANDLE m_handle;
};