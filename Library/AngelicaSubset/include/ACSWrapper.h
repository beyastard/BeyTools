#ifndef _ACSWRAPPER_H_
#define _ACSWRAPPER_H_

#include "ABaseDef.h"

#include <mutex>

template<typename MutexType>
class MutexGuard
{
public:
    MutexGuard(MutexType& mutex)
        : m_lock(mutex)
    {}

    // Prevent copying and assignment
    MutexGuard(const MutexGuard&) = delete;
    MutexGuard& operator=(const MutexGuard&) = delete;

private:
    std::lock_guard<MutexType> m_lock;
};

/*
Example usage:

bool ExampleOpen(const std::wstring& filename, const std::wstring mode)
{
    FILE* pFile = _wfopen(filename.c_str(), mode.c_str());
    if (pFile == nullptr)
        return false;

    std::mutex myMutex;
    {
        MutexGuard guard(myMutex);

        fseek(pFile, 0, SEEK_END);
        long size = ftell(pFile);
        fseek(pFile, 0, SEEK_SET);
    }

    fclose(pFile);
    return true;
}
*/

#endif
