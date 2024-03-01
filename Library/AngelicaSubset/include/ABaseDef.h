#ifndef _ABASEDEF_H_
#define _ABASEDEF_H_

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>

#include <cassert>
#include <codecvt>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <string>

#define min2(a, b) (((a) > (b)) ? (b) : (a))
#define min3(a, b, c) (min2(min2((a), (b)), (c)))
#define max2(a, b) (((a) > (b)) ? (a) : (b))
#define max3(a, b, c) (max2(max2((a), (b)), (c)))
#define min4(a, b, c, d) (min2(min2((a), (b)), min2((c), (d))))
#define max4(a, b, c, d) (max2(max2((a), (b)), max2((c), (d))))

// Disable copy constructor and operator = 
#define DISABLE_COPY_AND_ASSIGNMENT(ClassName)	\
	private:\
	ClassName(const ClassName&);\
	ClassName& operator = (const ClassName&);

#define DISABLE_NEW(ClassName) \
public:                        \
	void* operator new(std::size_t) = delete;

#define STATIC_CLASS(ClassName) \
public:                         \
	ClassName() = delete;       \
	~ClassName() = delete;

#endif
