#ifndef _BASE64_H_
#define _BASE64_H_

#include "ABaseDef.h"

int32_t	base64_encode(uint8_t* __instr, int32_t __inlen, char* __outstr);
int32_t	base64_decode(char* __instr, int32_t __inlen, uint8_t* __outstr);

#endif
