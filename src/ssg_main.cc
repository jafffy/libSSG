#include <Windows.h>
#include <cstdio>

#include "ssg.h"
#include "ssg_timer.h"
#include "ssg_dbg.h"

int CALLBACK WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	app_init();

	ssg_timer oneFrameTimer;
	oneFrameTimer.start();

	while (true) {
		float deltaTime = oneFrameTimer.elapsedTime();
		oneFrameTimer.reset();

		app_update(deltaTime);
		app_render();
	}

	app_destroy();

	return 0;
}