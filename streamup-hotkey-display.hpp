#pragma once

#include <obs.h>

#ifdef _WIN32
#include <windows.h>
extern HHOOK keyboardHook;
extern HHOOK mouseHook;
extern LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
extern LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam);
#endif

#ifdef __APPLE__
#include <ApplicationServices/ApplicationServices.h>
#include <Carbon/Carbon.h>
extern CFMachPortRef eventTap;
extern void startMacOSKeyboardHook();
extern void stopMacOSKeyboardHook();
CGEventRef CGEventCallback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void *refcon);
#endif

#ifdef __linux__
#include <X11/Xlib.h>
#include <X11/Xutil.h>
// X11 defines macros that clash with Qt enums (QEvent::None, QEvent::KeyPress, etc.)
// Save X11 constants before undefining, so Linux hook code can still use them
constexpr int X11_KeyPress = KeyPress;
constexpr int X11_KeyRelease = KeyRelease;
#undef None
#undef KeyPress
#undef KeyRelease
#undef Status
#undef Bool
#undef CursorShape
#undef Expose
#undef FocusIn
#undef FocusOut
#undef FontChange
#undef Unsorted
#include <atomic>
extern Display *display;
extern std::atomic<bool> linuxHookRunning;
extern void startLinuxKeyboardHook();
extern void stopLinuxKeyboardHook();
#endif

// Shared function declarations
extern obs_data_t *SaveLoadSettingsCallback(obs_data_t *save_data, bool saving);
extern void loadSingleKeyCaptureSettings(obs_data_t *settings);
