//This function or variable may be unsafe. 
//Consider using strncpy_s instead. To disable deprecation, 
//use _CRT_SECURE_NO_WARNINGS. See online help for details.
#pragma warning(push)
#pragma warning (disable: 4996)

#include "context.c"
#include "egl_context.c"
#include "init.c"
#include "input.c"
#include "monitor.c"
#include "osmesa_context.c"
#include "vulkan.c"
#include "wgl_context.c"
#include "win32_init.c"
#include "win32_joystick.c"
#include "win32_monitor.c"
#include "win32_thread.c"
#include "win32_time.c"
#include "win32_window.c"
#include "window.c"

#pragma warning(pop)