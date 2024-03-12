#ifndef __AI_POLICY_LOADER_H__
#define __AI_POLICY_LOADER_H__

#include "Policy.h"

class CPolicyLoader
{
public:
    CPolicyLoader() { manager = new CPolicyDataManager(); }
    ~CPolicyLoader()
    {
        if (manager)
        {
            delete manager;
            manager = nullptr;
        }
    }

    bool Load(const char* path) { return manager->Load(path) ? true : false; }
    bool Load(const wchar_t* path) { return manager->Load(path) ? true : false; }
    bool Load(const std::string& path) { return manager->Load(path) ? true : false; }
    bool Load(const std::wstring& path) { return manager->Load(path) ? true : false; }

    bool Save(const char* path) { return manager->Save(path) ? true : false; }
    bool Save(const wchar_t* path) { return manager->Save(path) ? true : false; }
    bool Save(const std::string& path) { return manager->Save(path) ? true : false; }
    bool Save(const std::wstring& path) { return manager->Save(path) ? true : false; }

    void Release()
    {
        if (manager)
            manager->Release();
    }

    CPolicyDataManager* GetPolicyManager() { return manager; }

private:
    CPolicyDataManager* manager;
};

#endif
