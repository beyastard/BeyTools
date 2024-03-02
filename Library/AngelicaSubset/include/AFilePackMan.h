#ifndef _AFILEPACKMAN_H_
#define _AFILEPACKMAN_H_

#include "AFilePackBase.h"

#include <vector>

class AFilePackMan
{
public:
	enum
	{
		OPEN_ENCRYPT = 0x0001,      // Encrypt flag
		OPEN_LOCALIZATION = 0x0002, // Localization flag
	};

	AFilePackMan();
	~AFilePackMan();

	bool SetAlgorithmID(uint32_t id);

	bool OpenFilePackage(const std::wstring& szPckFile, uint32_t dwOpenFlags = 0);
	bool OpenFilePackage(const std::wstring& szPckFile, const std::wstring& szFolder, uint32_t dwOpenFlags = 0);
	bool OpenFilePackageInGame(const std::wstring& szPckFile, uint32_t dwOpenFlags = 0);
	bool OpenFilePackageInGame(const std::wstring& szPckFile, const std::wstring& szFolder, uint32_t dwOpenFlags = 0);
	bool CreateFilePackage(const std::wstring& szPckFile, const std::wstring& szFolder, uint32_t dwOpenFlags = 0);

	bool CloseFilePackage(AFilePackBase* pFilePck);
	bool CloseAllPackages();

	int32_t GetNumPackages() { return (int32_t)m_FilePcks.size(); }
	AFilePackBase* GetFilePck(int32_t index) { return m_FilePcks[index]; }
	AFilePackBase* GetFilePck(const std::wstring& szPath) { return GetFilePck(szPath, false); }
	AFilePackBase* GetFilePck(const std::wstring& szPath, bool bCheckLocalization);
	AFilePackBase* GetLocalizationPack() { return m_pLocalizePack; }

private:
	std::vector<AFilePackBase*>	m_FilePcks;
	AFilePackBase* m_pLocalizePack;
};

extern AFilePackMan g_AFilePackMan;

#endif
