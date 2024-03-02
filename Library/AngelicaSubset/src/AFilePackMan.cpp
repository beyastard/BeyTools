#include "AFilePackMan.h"
#include "AFilePackage.h"
#include "AFilePackGame.h"
#include "AFI.h"
#include "AFPI.h"

#include <algorithm>

uint32_t AFPCK_GUARDBYTE0 = 0xfdfdfeee;
uint32_t AFPCK_GUARDBYTE1 = 0xf00dbeef;
uint32_t AFPCK_MASKDWORD = 0xa8937462;
uint32_t AFPCK_CHECKMASK = 0x59374231;

AFilePackMan g_AFilePackMan;

AFilePackMan::AFilePackMan()
    : m_FilePcks()
    , m_pLocalizePack(nullptr)
{}

AFilePackMan::~AFilePackMan()
{
    CloseAllPackages();
}

bool AFilePackMan::SetAlgorithmID(uint32_t id)
{
    switch (id)
    {
    case 111:
        AFPCK_GUARDBYTE0 = 0xab12908f;
        AFPCK_GUARDBYTE1 = 0xb3231902;
        AFPCK_MASKDWORD = 0x2a63810e;
        AFPCK_CHECKMASK = 0x18734563;
        break;
    default:
        AFPCK_GUARDBYTE0 = 0xfdfdfeee + id * 0x72341f2;
        AFPCK_GUARDBYTE1 = 0xf00dbeef + id * 0x1237a73;
        AFPCK_MASKDWORD = 0xa8937462 + id * 0xab2321f;
        AFPCK_CHECKMASK = 0x59374231 + id * 0x987a223;
        break;
    }

    return true;
}

bool AFilePackMan::OpenFilePackage(const std::wstring& szPckFile, uint32_t dwOpenFlags)
{
    bool bEncrypt = (dwOpenFlags & OPEN_ENCRYPT) ? true : false;
    bool bLocalization = (dwOpenFlags & OPEN_LOCALIZATION) ? true : false;

    if (bLocalization && m_pLocalizePack)
    {
        // Only one localization package can exist at one moment
        AFERRLOG((L"AFilePackMan::OpenFilePackage(), Localization package has existed!"));
        return false;
    }

    AFilePackage* pFilePackage;
    if (!(pFilePackage = new AFilePackage))
    {
        AFERRLOG((L"AFilePackMan::OpenFilePackage(), Not enough memory!"));
        return false;
    }

    if (bLocalization)
    {
        if (!pFilePackage->Open(szPckFile, L"", AFilePackage::OPENEXIST, bEncrypt))
        {
            delete pFilePackage;
            AFERRLOG((L"AFilePackMan::OpenFilePackage(), Can not open package [%s]", szPckFile.c_str()));
            return false;
        }

        m_pLocalizePack = pFilePackage;
    }
    else
    {
        if (!pFilePackage->Open(szPckFile, AFilePackage::OPENEXIST, bEncrypt))
        {
            delete pFilePackage;
            AFERRLOG((L"AFilePackMan::OpenFilePackage(), Can not open package [%s]", szPckFile.c_str()));
            return false;
        }

        m_FilePcks.push_back(pFilePackage);
    }

    return true;
}

bool AFilePackMan::OpenFilePackage(const std::wstring& szPckFile, const std::wstring& szFolder, uint32_t dwOpenFlags)
{
    bool bEncrypt = (dwOpenFlags & OPEN_ENCRYPT) ? true : false;
    bool bLocalization = (dwOpenFlags & OPEN_LOCALIZATION) ? true : false;

    if (bLocalization && m_pLocalizePack)
    {
        // Only one localization package can exist at one moment
        AFERRLOG((L"AFilePackMan::OpenFilePackage(), Localization package has existed!"));
        return false;
    }

    AFilePackage* pFilePackage;
    if (!(pFilePackage = new AFilePackage))
    {
        AFERRLOG((L"AFilePackMan::OpenFilePackage(), Not enough memory!"));
        return false;
    }

    if (!pFilePackage->Open(szPckFile, szFolder, AFilePackage::OPENEXIST, bEncrypt))
    {
        delete pFilePackage;
        AFERRLOG((L"AFilePackMan::OpenFilePackage(), Can not open package [%s]", szPckFile.c_str()));
        return false;
    }

    if (bLocalization)
        m_pLocalizePack = pFilePackage;
    else
        m_FilePcks.push_back(pFilePackage);

    return true;
}

bool AFilePackMan::OpenFilePackageInGame(const std::wstring& szPckFile, uint32_t dwOpenFlags)
{
    bool bEncrypt = (dwOpenFlags & OPEN_ENCRYPT) ? true : false;
    bool bLocalization = (dwOpenFlags & OPEN_LOCALIZATION) ? true : false;

    if (bLocalization && m_pLocalizePack)
    {
        // Only one localization package can exist at one moment
        AFERRLOG((L"AFilePackMan::OpenFilePackageInGame(), Localization package has existed!"));
        return false;
    }

    AFilePackGame* pFilePackage;
    if (!(pFilePackage = new AFilePackGame))
    {
        AFERRLOG((L"AFilePackMan::OpenFilePackageInGame(), Not enough memory!"));
        return false;
    }

    if (bLocalization)
    {
        if (!pFilePackage->Open(szPckFile, L"", AFilePackGame::OPENEXIST, bEncrypt))
        {
            delete pFilePackage;
            AFERRLOG((L"AFilePackMan::OpenFilePackageInGame(), Can not open package [%s]", szPckFile.c_str()));
            return false;
        }

        m_pLocalizePack = pFilePackage;
    }
    else
    {
        if (!pFilePackage->Open(szPckFile, AFilePackGame::OPENEXIST, bEncrypt))
        {
            delete pFilePackage;
            AFERRLOG((L"AFilePackMan::OpenFilePackageInGame(), Can not open package [%s]", szPckFile.c_str()));
            return false;
        }

        m_FilePcks.push_back(pFilePackage);
    }

    return true;
}

bool AFilePackMan::OpenFilePackageInGame(const std::wstring& szPckFile, const std::wstring& szFolder, uint32_t dwOpenFlags)
{
    bool bEncrypt = (dwOpenFlags & OPEN_ENCRYPT) ? true : false;
    bool bLocalization = (dwOpenFlags & OPEN_LOCALIZATION) ? true : false;

    if (bLocalization && m_pLocalizePack)
    {
        // Only one localization package can exist at one moment
        AFERRLOG((L"AFilePackMan::OpenFilePackageInGame(), Localization package has existed!"));
        return false;
    }

    AFilePackGame* pFilePackage;
    if (!(pFilePackage = new AFilePackGame))
    {
        AFERRLOG((L"AFilePackMan::OpenFilePackageInGame(), Not enough memory!"));
        return false;
    }

    if (!pFilePackage->Open(szPckFile, szFolder, AFilePackGame::OPENEXIST, bEncrypt))
    {
        delete pFilePackage;
        AFERRLOG((L"AFilePackMan::OpenFilePackageInGame(), Can not open package [%s]", szPckFile));
        return false;
    }

    if (bLocalization)
        m_pLocalizePack = pFilePackage;
    else
        m_FilePcks.push_back(pFilePackage);

    return true;
}

bool AFilePackMan::CreateFilePackage(const std::wstring& szPckFile, const std::wstring& szFolder, uint32_t dwOpenFlags)
{
    bool bEncrypt = (dwOpenFlags & OPEN_ENCRYPT) ? true : false;
    bool bLocalization = (dwOpenFlags & OPEN_LOCALIZATION) ? true : false;

    if (bLocalization && m_pLocalizePack)
    {
        //	Only one localization package can exist at one moment
        AFERRLOG((L"AFilePackMan::CreateFilePackage(), Localization package has existed!"));
        return false;
    }

    AFilePackage* pFilePackage;
    if (!(pFilePackage = new AFilePackage))
    {
        AFERRLOG((L"AFilePackMan::CreateFilePackage(), Not enough memory!"));
        return false;
    }

    if (!pFilePackage->Open(szPckFile, szFolder, AFilePackage::CREATENEW, bEncrypt))
    {
        delete pFilePackage;
        AFERRLOG((L"AFilePackMan::CreateFilePackage(), Can not open package [%s]", szPckFile));
        return false;
    }

    if (bLocalization)
        m_pLocalizePack = pFilePackage;
    else
        m_FilePcks.push_back(pFilePackage);

    return true;
}

bool AFilePackMan::CloseFilePackage(AFilePackBase* pFilePck)
{
    if (!pFilePck)
        return true;

    if (m_pLocalizePack == pFilePck)
    {
        pFilePck->Close();
        delete pFilePck;
        m_pLocalizePack = nullptr;
        return true;
    }
    else
    {
        auto it = std::find(m_FilePcks.begin(), m_FilePcks.end(), pFilePck);
        if (it != m_FilePcks.end())
        {
            pFilePck->Close();
            delete pFilePck;
            m_FilePcks.erase(it);
            return true;
        }
    }

    return false;
}

bool AFilePackMan::CloseAllPackages()
{
    if (m_pLocalizePack)
    {
        m_pLocalizePack->Close();
        delete m_pLocalizePack;
        m_pLocalizePack = nullptr;
    }

    for (auto it = m_FilePcks.begin(); it != m_FilePcks.end(); ++it)
    {
        AFilePackBase* pFilePck = *it;
        pFilePck->Close();
        delete pFilePck;
    }

    m_FilePcks.clear();

    return true;
}

AFilePackBase* AFilePackMan::GetFilePck(const std::wstring& szPath, bool bCheckLocalization)
{
    if (bCheckLocalization && m_pLocalizePack)
    {
        // Check if specified file exists in the localization package
        if (m_pLocalizePack->IsFileExist(szPath))
            return m_pLocalizePack;
    }

    // Normalize file name
    std::wstring szLowPath = szPath;
    std::transform(szLowPath.begin(), szLowPath.end(), szLowPath.begin(), ::towlower);
    AFilePackage::NormalizeFileName(szLowPath);

    // Check other packages through file path
    for (auto& pPack : m_FilePcks)
    {
        if (szLowPath.find(pPack->GetFolder()) == 0)
            return pPack;
    }

    return nullptr;
}
