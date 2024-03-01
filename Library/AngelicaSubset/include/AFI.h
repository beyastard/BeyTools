#ifndef _AFI_H_
#define _AFI_H_

#include "ABaseDef.h"

extern std::wstring g_strBaseDir;

bool af_Initialize();
bool af_Finalize();
bool af_SetBaseDir(const std::wstring& szBaseDir);
const std::wstring& af_GetBaseDir();

void af_GetRelativePathNoBase(const std::wstring& szFullpath, const std::wstring& szParentPath, std::wstring& szRelativepath);
void af_GetFullPathNoBase(std::wstring& szFullpath, const std::wstring& szBaseDir, const std::wstring& szFilename);

void af_GetFullPath(std::wstring& szFullPath, const std::wstring& szFolderName, const std::wstring& szFileName);
void af_GetFullPath(std::wstring& szFullPath, const std::wstring& szFileName);

void af_GetRelativePath(const std::wstring& szFullPath, const std::wstring& szFolderName, std::wstring& szRelativePath);
void af_GetRelativePath(const std::wstring& szFullPath, std::wstring& szRelativePath);

bool af_GetFileTitle(const std::wstring& lpszFile, std::wstring& lpszTitle, uint16_t cbBuf);
bool af_GetFileTitle(const std::wstring& lpszFile, std::wstring& strTitle);

bool af_GetFilePath(const std::wstring& lpszFile, std::wstring& lpszPath, uint16_t cbBuf);
bool af_GetFilePath(const std::wstring& lpszFile, std::wstring& strPath);

bool af_CheckFileExt(const std::wstring& szFileName, const std::wstring& szExt, int32_t iExtLen = -1, int32_t iFileNameLen = -1);

bool af_ChangeFileExt(std::wstring& szFileNameBuf, int32_t iBufLen, const std::wstring& szNewExt);
bool af_ChangeFileExt(std::wstring& strFileName, const std::wstring& szNewExt);

bool af_IsFileExist(const std::wstring& szFileName);

#endif
