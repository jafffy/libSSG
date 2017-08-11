#include "ssg_dbg.h"

#include <cstdio>
#include <Windows.h>

void ssg_dbg_fmt(const char* fmt, ...)
{
#ifdef _DEBUG
	static const int kMaxMessageLength = 4096;
	char message[kMaxMessageLength] = {0};

	va_list argptr;
	va_start(argptr, fmt);
	vsnprintf(message, kMaxMessageLength, fmt, argptr); // TODO: fix potential buffer overflow error.
	va_end(argptr);

	OutputDebugStringA(message);
#endif // _DEBUG
}