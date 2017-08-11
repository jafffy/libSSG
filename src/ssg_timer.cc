#include "ssg_timer.h"

#include <Windows.h>

class ssg_timer_impl
{
	friend class ssg_timer;

	LARGE_INTEGER performanceFrequency;
	LARGE_INTEGER lastTimestamp;

	ssg_timer_impl()
	{
		QueryPerformanceFrequency(&performanceFrequency);
	}
};

ssg_timer::ssg_timer()
	: impl(new ssg_timer_impl())
{
}

ssg_timer::~ssg_timer()
{
	if (impl != nullptr) {
		delete impl;
		impl = nullptr;
	}
}

void ssg_timer::start()
{
	QueryPerformanceCounter(&impl->lastTimestamp);
}

void ssg_timer::reset()
{
	QueryPerformanceCounter(&impl->lastTimestamp);
}

float ssg_timer::elapsedTime()
{
	LARGE_INTEGER currentTime, elapsedMicroseconds;
	QueryPerformanceCounter(&currentTime);

	elapsedMicroseconds.QuadPart = currentTime.QuadPart - impl->lastTimestamp.QuadPart;
	elapsedMicroseconds.QuadPart *= 1000000; // To prevent precision loss.
	elapsedMicroseconds.QuadPart /= impl->performanceFrequency.QuadPart;

	return elapsedMicroseconds.QuadPart / 1000000.0f;
}