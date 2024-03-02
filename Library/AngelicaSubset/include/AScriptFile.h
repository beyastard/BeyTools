#ifndef _ASCRIPTFILE_H_
#define _ASCRIPTFILE_H_

////////////////////////////////////////////////////////////////////////////////////
//
//	Script file is pure text file without the first 4-byte flag of Angelica files. When
//	a script file is opened, it will be loaded into memory and freed when Close() is
//	called. So never forget to close a script file after you opened it.
//
//	Script file has the same comment style as C/C++, that means all content in a line
//	after // or paragraph between /* */ pair will be ignored.
//
//	Token: token is a text string. It only contain characters whose corresponding
//	ASCII codes > 32 except some special characters: , ; ( ) ", you can use these
//	characters and spece to separate tokens. Of course //, /* and */ won't be consided
//	as tokens.
//
//	For example:
//
//		abdad		is a token
//		123.22		is a token
//		[p%^@1]		is a token
//
//	A special case is, all text in a line and between "" or () will be consided as ONE
//	token. examples:
//
//	"293, is a number",	'293, is a number' will be read as a token
//	(213, 222, 10.2),	'213, 222, 10.2' will be read as a token
//	"(%$# QWE)"			'(%$# QWE)' will be read as a token
//
//	Use GetNextToken() or PeekNextToken to next token and it will be stored in
//	m_szToken of A3DScriptFile object. m_szToken is a public member so it can be accessd freely.
//
////////////////////////////////////////////////////////////////////////////////////

#include "ABaseDef.h"
#include "AFile.h"

class AScriptFile
{
public:
	enum
	{
		MAX_LINELEN = 2048 // Maximum line length in characters
	};

	struct SCRIPTINFO
	{
		uint8_t* pStart;   // Start address of buffer
		uint8_t* pEnd;     // End address of buffer
		uint8_t* pCur;     // Current pointer
		int32_t iLine;     // Line counter
	};

	AScriptFile();
	virtual ~AScriptFile();

	bool Open(AFile* pFile);
	bool Open(const std::wstring& szFile);
	void Close();

	void ResetScriptFile();
	bool GetNextToken(bool bCrossLine);
	bool PeekNextToken(bool bCrossLine);
	bool SkipLine();
	bool MatchToken(char* szToken, bool bCaseSensitive);
	bool MatchToken(char* szToken);

	float GetNextTokenAsFloat(bool bCrossLine);
	int32_t GetNextTokenAsInt(bool bCrossLine);

	bool IsEnd() const { return m_Script.pCur >= m_Script.pEnd; }
	int32_t GetCurLine() const { return m_Script.iLine; }

	char m_szToken[MAX_LINELEN];

protected:
	SCRIPTINFO m_Script;
};

#endif

