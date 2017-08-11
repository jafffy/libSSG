#ifndef SSG_H_
#define SSG_H_

#include "core\\ssg_color4.h"

bool app_init();
void app_destroy();
void app_update(float deltaTime);
void app_render();

const wchar_t*	app_get_window_title();
int				app_get_window_width();
int				app_get_window_height();
ssg_color4f		app_get_clear_color(); // TODO: There can be potential performance issue by copying.

#endif // SSG_H_