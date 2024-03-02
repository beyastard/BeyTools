#ifndef _AWSCRIPTFILE_H_
#define _AWSCRIPTFILE_H_

#include "ABaseDef.h"
#include "AFile.h"

class AWScriptFile
{
public:
	enum
	{
		MAX_LINELEN = 2048 // Maximum line length in characters
	};

	struct SCRIPTINFO
	{
		uint8_t* pFileBuf; // Pointer to file data buffer
		wchar_t* pStart;   // Start address of buffer
		wchar_t* pEnd;     // End address of buffer
		wchar_t* pCur;     // Current pointer
		int32_t iLine;     // Line counter
	};

	AWScriptFile();
	virtual ~AWScriptFile();

	wchar_t m_szToken[MAX_LINELEN];

	bool Open(AFile* pFile);
	bool Open(const wchar_t* szFile);
	void Close();

	void ResetScriptFile();
	bool GetNextToken(bool bCrossLine);
	bool PeekNextToken(bool bCrossLine);
	bool SkipLine();
	bool MatchToken(wchar_t* wszToken, bool bCaseSensitive);
	bool MatchToken(wchar_t* szToken);

	float GetNextTokenAsFloat(bool bCrossLine);
	int32_t GetNextTokenAsInt(bool bCrossLine);

	bool IsEnd() { return m_Script.pCur >= m_Script.pEnd; }
	int32_t GetCurLine() { return m_Script.iLine; }

protected:
	SCRIPTINFO m_Script;
};

#endif
