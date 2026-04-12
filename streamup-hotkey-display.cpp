#include "streamup-hotkey-display-dock.hpp"
#include "streamup-hotkey-display.hpp"
#include "streamup-hotkey-display-settings.hpp"
#include "version.h"
#include <obs-module.h>
#include <obs-frontend-api.h>
#include <obs.h>
#include <unordered_set>
#include <unordered_map>
#include <string>
#include <vector>
#include <mutex>
#include <algorithm>
#include <thread>
#include <atomic>
#include <QMainWindow>
#include <QDockWidget>
#include <QMetaObject>
#include <util/platform.h>
#include "obs-websocket-api.h"

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef __APPLE__
#include <ApplicationServices/ApplicationServices.h>
#include <Carbon/Carbon.h>
#endif

#ifdef __linux__
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif

#define QT_UTF8(str) QString::fromUtf8(str)
#define QT_TO_UTF8(str) str.toUtf8().constData()

OBS_DECLARE_MODULE()
OBS_MODULE_AUTHOR("Andilippi")
OBS_MODULE_USE_DEFAULT_LOCALE("streamup-hotkey-display", "en-US")

#ifdef _WIN32
HHOOK keyboardHook;
HHOOK mouseHook;
#endif

#ifdef __linux__
Display *display;
std::thread linuxHookThread;
std::atomic<bool> linuxHookRunning{false};
#endif

std::unordered_set<int> pressedKeys;
std::unordered_set<int> activeModifiers;
std::mutex keyStateMutex; // Protects pressedKeys, activeModifiers, and loggedCombinations

#ifdef _WIN32
std::unordered_set<int> modifierKeys = {VK_CONTROL, VK_LCONTROL, VK_RCONTROL, VK_MENU, VK_LMENU, VK_RMENU,
					VK_SHIFT,   VK_LSHIFT,   VK_RSHIFT,   VK_LWIN, VK_RWIN};

std::unordered_set<int> singleKeys = {VK_INSERT, VK_DELETE, VK_HOME, VK_END, VK_PRIOR, VK_NEXT, VK_F1,  VK_F2,  VK_F3,
				      VK_F4,     VK_F5,     VK_F6,   VK_F7,  VK_F8,    VK_F9,   VK_F10, VK_F11, VK_F12};

// Additional key categories for single key capture
std::unordered_set<int> numpadKeys = {VK_NUMPAD0, VK_NUMPAD1, VK_NUMPAD2, VK_NUMPAD3, VK_NUMPAD4,
				      VK_NUMPAD5, VK_NUMPAD6, VK_NUMPAD7, VK_NUMPAD8, VK_NUMPAD9,
				      VK_MULTIPLY, VK_ADD, VK_SEPARATOR, VK_SUBTRACT, VK_DECIMAL, VK_DIVIDE};

std::unordered_set<int> numberKeys = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};

std::unordered_set<int> letterKeys = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
				      'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};

std::unordered_set<int> punctuationKeys = {VK_OEM_1,      VK_OEM_PLUS,   VK_OEM_COMMA, VK_OEM_MINUS, VK_OEM_PERIOD,
					   VK_OEM_2,      VK_OEM_3,      VK_OEM_4,     VK_OEM_5,     VK_OEM_6,
					   VK_OEM_7,      VK_OEM_102,    VK_SPACE,     VK_TAB,       VK_OEM_8,
					   VK_OEM_AX,     VK_OEM_CLEAR,  VK_BACK};
#endif

#ifdef __APPLE__
std::unordered_set<int> modifierKeys = {kVK_Control,      kVK_Command,      kVK_Option,      kVK_Shift,
					kVK_RightControl, kVK_RightCommand, kVK_RightOption, kVK_RightShift};

std::unordered_set<int> singleKeys = {
	kVK_ANSI_Keypad0, kVK_ANSI_Keypad1, kVK_ANSI_Keypad2, kVK_ANSI_Keypad3, kVK_ANSI_Keypad4,     kVK_ANSI_Keypad5,
	kVK_ANSI_Keypad6, kVK_ANSI_Keypad7, kVK_ANSI_Keypad8, kVK_ANSI_Keypad9, kVK_ANSI_KeypadClear, kVK_ANSI_KeypadEnter,
	kVK_Escape,       kVK_Delete,       kVK_Home,         kVK_End,          kVK_PageUp,           kVK_PageDown,
	kVK_Return};

// Additional key categories for single key capture
std::unordered_set<int> numpadKeys = {kVK_ANSI_Keypad0,     kVK_ANSI_Keypad1,    kVK_ANSI_Keypad2,
				      kVK_ANSI_Keypad3,     kVK_ANSI_Keypad4,    kVK_ANSI_Keypad5,
				      kVK_ANSI_Keypad6,     kVK_ANSI_Keypad7,    kVK_ANSI_Keypad8,
				      kVK_ANSI_Keypad9,     kVK_ANSI_KeypadClear, kVK_ANSI_KeypadEnter,
				      kVK_ANSI_KeypadPlus,  kVK_ANSI_KeypadMinus, kVK_ANSI_KeypadMultiply,
				      kVK_ANSI_KeypadDivide, kVK_ANSI_KeypadDecimal, kVK_ANSI_KeypadEquals};

std::unordered_set<int> numberKeys = {kVK_ANSI_0, kVK_ANSI_1, kVK_ANSI_2, kVK_ANSI_3, kVK_ANSI_4,
				      kVK_ANSI_5, kVK_ANSI_6, kVK_ANSI_7, kVK_ANSI_8, kVK_ANSI_9};

std::unordered_set<int> letterKeys = {kVK_ANSI_A, kVK_ANSI_B, kVK_ANSI_C, kVK_ANSI_D, kVK_ANSI_E, kVK_ANSI_F,
				      kVK_ANSI_G, kVK_ANSI_H, kVK_ANSI_I, kVK_ANSI_J, kVK_ANSI_K, kVK_ANSI_L,
				      kVK_ANSI_M, kVK_ANSI_N, kVK_ANSI_O, kVK_ANSI_P, kVK_ANSI_Q, kVK_ANSI_R,
				      kVK_ANSI_S, kVK_ANSI_T, kVK_ANSI_U, kVK_ANSI_V, kVK_ANSI_W, kVK_ANSI_X,
				      kVK_ANSI_Y, kVK_ANSI_Z};

std::unordered_set<int> punctuationKeys = {
	kVK_ANSI_Semicolon,     kVK_ANSI_Quote,        kVK_ANSI_Comma,       kVK_ANSI_Period,
	kVK_ANSI_Slash,         kVK_ANSI_Backslash,    kVK_ANSI_LeftBracket, kVK_ANSI_RightBracket,
	kVK_ANSI_Grave,         kVK_ANSI_Equal,        kVK_ANSI_Minus,       kVK_Space,
	kVK_Tab,                kVK_Delete,            kVK_ForwardDelete};

// Fix #6: Proper macOS keycode lookup maps (keycodes are NOT contiguous)
static const std::unordered_map<char, int> macLetterKeycodes = {
	{'A', kVK_ANSI_A}, {'B', kVK_ANSI_B}, {'C', kVK_ANSI_C}, {'D', kVK_ANSI_D},
	{'E', kVK_ANSI_E}, {'F', kVK_ANSI_F}, {'G', kVK_ANSI_G}, {'H', kVK_ANSI_H},
	{'I', kVK_ANSI_I}, {'J', kVK_ANSI_J}, {'K', kVK_ANSI_K}, {'L', kVK_ANSI_L},
	{'M', kVK_ANSI_M}, {'N', kVK_ANSI_N}, {'O', kVK_ANSI_O}, {'P', kVK_ANSI_P},
	{'Q', kVK_ANSI_Q}, {'R', kVK_ANSI_R}, {'S', kVK_ANSI_S}, {'T', kVK_ANSI_T},
	{'U', kVK_ANSI_U}, {'V', kVK_ANSI_V}, {'W', kVK_ANSI_W}, {'X', kVK_ANSI_X},
	{'Y', kVK_ANSI_Y}, {'Z', kVK_ANSI_Z}
};
static const std::unordered_map<char, int> macDigitKeycodes = {
	{'0', kVK_ANSI_0}, {'1', kVK_ANSI_1}, {'2', kVK_ANSI_2}, {'3', kVK_ANSI_3},
	{'4', kVK_ANSI_4}, {'5', kVK_ANSI_5}, {'6', kVK_ANSI_6}, {'7', kVK_ANSI_7},
	{'8', kVK_ANSI_8}, {'9', kVK_ANSI_9}
};
#endif

#ifdef __linux__
std::unordered_set<int> modifierKeys = {XK_Control_L, XK_Control_R, XK_Super_L, XK_Super_R,
					XK_Alt_L,     XK_Alt_R,     XK_Shift_L, XK_Shift_R};

std::unordered_set<int> singleKeys = {XK_Insert, XK_Delete, XK_Home, XK_End, XK_Page_Up, XK_Page_Down, XK_F1,
				      XK_F2,     XK_F3,     XK_F4,   XK_F5,  XK_F6,      XK_F7,        XK_F8,
				      XK_F9,     XK_F10,    XK_F11,  XK_F12, XK_Return};

// Additional key categories for single key capture
std::unordered_set<int> numpadKeys = {XK_KP_0,        XK_KP_1,      XK_KP_2,       XK_KP_3,
				      XK_KP_4,        XK_KP_5,      XK_KP_6,       XK_KP_7,
				      XK_KP_8,        XK_KP_9,      XK_KP_Add,     XK_KP_Subtract,
				      XK_KP_Multiply, XK_KP_Divide, XK_KP_Decimal, XK_KP_Enter};

std::unordered_set<int> numberKeys = {XK_0, XK_1, XK_2, XK_3, XK_4, XK_5, XK_6, XK_7, XK_8, XK_9};

std::unordered_set<int> letterKeys = {XK_a, XK_b, XK_c, XK_d, XK_e, XK_f, XK_g, XK_h, XK_i,
				      XK_j, XK_k, XK_l, XK_m, XK_n, XK_o, XK_p, XK_q, XK_r,
				      XK_s, XK_t, XK_u, XK_v, XK_w, XK_x, XK_y, XK_z,
				      XK_A, XK_B, XK_C, XK_D, XK_E, XK_F, XK_G, XK_H, XK_I,
				      XK_J, XK_K, XK_L, XK_M, XK_N, XK_O, XK_P, XK_Q, XK_R,
				      XK_S, XK_T, XK_U, XK_V, XK_W, XK_X, XK_Y, XK_Z};

std::unordered_set<int> punctuationKeys = {
	XK_semicolon, XK_comma,     XK_period,    XK_slash,      XK_backslash,   XK_apostrophe,
	XK_grave,     XK_bracketleft, XK_bracketright, XK_equal,      XK_minus,       XK_space,
	XK_Tab,       XK_BackSpace, XK_Return};
#endif

std::unordered_set<std::string> loggedCombinations;

// Single key capture settings
bool captureNumpad = false;
bool captureNumbers = false;
bool captureLetters = false;
bool capturePunctuation = false;
std::unordered_set<int> whitelistedKeySet;

// Logging settings
bool enableLogging = false;

// Display settings
std::string keySeparator = " + ";
std::string lastKeyCombination;

HotkeyDisplayDock *hotkeyDisplayDock = nullptr;
obs_hotkey_id toggleHotkeyId = OBS_INVALID_HOTKEY_ID;
obs_websocket_vendor websocket_vendor = nullptr;

// Deferred hook enable: hooks must be installed after the event loop is running
static bool deferredHookEnable = false;

// Key name lookup tables for performance
#ifdef _WIN32
static const std::unordered_map<int, const char *> keyNameMap = {
	{VK_LBUTTON, "Left Click"},  {VK_RBUTTON, "Right Click"},  {VK_MBUTTON, "Middle Click"},
	{VK_XBUTTON1, "X Button 1"}, {VK_XBUTTON2, "X Button 2"},  {VK_CONTROL, "Ctrl"},
	{VK_LCONTROL, "Ctrl"},       {VK_RCONTROL, "Ctrl"},        {VK_MENU, "Alt"},
	{VK_LMENU, "Alt"},           {VK_RMENU, "Alt"},            {VK_SHIFT, "Shift"},
	{VK_LSHIFT, "Shift"},        {VK_RSHIFT, "Shift"},         {VK_LWIN, "Win"},
	{VK_RWIN, "Win"},            {VK_RETURN, "Enter"},         {VK_SPACE, "Space"},
	{VK_BACK, "Backspace"},      {VK_TAB, "Tab"},              {VK_ESCAPE, "Escape"},
	{VK_PRIOR, "Page Up"},       {VK_NEXT, "Page Down"},       {VK_END, "End"},
	{VK_HOME, "Home"},           {VK_LEFT, "Left Arrow"},      {VK_UP, "Up Arrow"},
	{VK_RIGHT, "Right Arrow"},   {VK_DOWN, "Down Arrow"},      {VK_INSERT, "Insert"},
	{VK_DELETE, "Delete"},       {VK_F1, "F1"},                {VK_F2, "F2"},
	{VK_F3, "F3"},               {VK_F4, "F4"},                {VK_F5, "F5"},
	{VK_F6, "F6"},               {VK_F7, "F7"},                {VK_F8, "F8"},
	{VK_F9, "F9"},               {VK_F10, "F10"},              {VK_F11, "F11"},
	{VK_F12, "F12"}};
#endif

#ifdef __APPLE__
static const std::unordered_map<int, const char *> keyNameMap = {
	{kVK_Control, "Ctrl"},          {kVK_RightControl, "Ctrl"},   {kVK_Command, "Cmd"},
	{kVK_RightCommand, "Cmd"},      {kVK_Option, "Alt"},          {kVK_RightOption, "Alt"},
	{kVK_Shift, "Shift"},           {kVK_RightShift, "Shift"},    {kVK_ANSI_KeypadEnter, "Enter"},
	{kVK_Return, "Enter"},          {kVK_Space, "Space"},         {kVK_Delete, "Backspace"},
	{kVK_Tab, "Tab"},               {kVK_Escape, "Escape"},       {kVK_PageUp, "Page Up"},
	{kVK_PageDown, "Page Down"},    {kVK_End, "End"},             {kVK_Home, "Home"},
	{kVK_LeftArrow, "Left Arrow"},  {kVK_UpArrow, "Up Arrow"},    {kVK_RightArrow, "Right Arrow"},
	{kVK_DownArrow, "Down Arrow"},  {kVK_Help, "Insert"},         {kVK_F1, "F1"},
	{kVK_F2, "F2"},                 {kVK_F3, "F3"},               {kVK_F4, "F4"},
	{kVK_F5, "F5"},                 {kVK_F6, "F6"},               {kVK_F7, "F7"},
	{kVK_F8, "F8"},                 {kVK_F9, "F9"},               {kVK_F10, "F10"},
	{kVK_F11, "F11"},               {kVK_F12, "F12"}};
#endif

#ifdef __linux__
static const std::unordered_map<int, const char *> keyNameMap = {
	{XK_Control_L, "Ctrl"},    {XK_Control_R, "Ctrl"},   {XK_Super_L, "Super"},
	{XK_Super_R, "Super"},     {XK_Alt_L, "Alt"},        {XK_Alt_R, "Alt"},
	{XK_Shift_L, "Shift"},     {XK_Shift_R, "Shift"},    {XK_Return, "Enter"},
	{XK_space, "Space"},       {XK_BackSpace, "Backspace"}, {XK_Tab, "Tab"},
	{XK_Escape, "Escape"},     {XK_Page_Up, "Page Up"},  {XK_Page_Down, "Page Down"},
	{XK_End, "End"},           {XK_Home, "Home"},        {XK_Left, "Left Arrow"},
	{XK_Up, "Up Arrow"},       {XK_Right, "Right Arrow"}, {XK_Down, "Down Arrow"},
	{XK_Insert, "Insert"},     {XK_Delete, "Delete"},    {XK_F1, "F1"},
	{XK_F2, "F2"},             {XK_F3, "F3"},            {XK_F4, "F4"},
	{XK_F5, "F5"},             {XK_F6, "F6"},            {XK_F7, "F7"},
	{XK_F8, "F8"},             {XK_F9, "F9"},            {XK_F10, "F10"},
	{XK_F11, "F11"},           {XK_F12, "F12"}};
#endif

// Lock-free variant — caller must hold keyStateMutex
static bool isModifierKeyPressedLocked()
{
	for (const int key : activeModifiers) {
		if (pressedKeys.count(key)) {
			return true;
		}
	}
	return false;
}

std::string getKeyName(int vkCode)
{
	// Try lookup table first (O(1) average case)
	auto it = keyNameMap.find(vkCode);
	if (it != keyNameMap.end()) {
		return it->second;
	}

	// Fallback for keys not in lookup table
#ifdef _WIN32
	UINT scanCode = MapVirtualKey(vkCode, MAPVK_VK_TO_VSC);
	char keyName[128];
	if (GetKeyNameTextA(scanCode << 16, keyName, sizeof(keyName)) > 0) {
		return std::string(keyName);
	}
#endif

	return "Unknown";
}

// Lock-free variant — caller must hold keyStateMutex
static std::string getCurrentCombinationLocked()
{
	std::vector<int> orderedKeys;
#ifdef _WIN32
	orderedKeys = {VK_CONTROL, VK_LCONTROL, VK_RCONTROL, VK_LWIN,   VK_RWIN,  VK_MENU,
		       VK_LMENU,   VK_RMENU,    VK_SHIFT,    VK_LSHIFT, VK_RSHIFT};
#endif

#ifdef __APPLE__
	orderedKeys = {kVK_Control,      kVK_Command,      kVK_Option,      kVK_Shift,
		       kVK_RightControl, kVK_RightCommand, kVK_RightOption, kVK_RightShift};
#endif

#ifdef __linux__
	orderedKeys = {XK_Control_L, XK_Control_R, XK_Super_L, XK_Super_R, XK_Alt_L, XK_Alt_R, XK_Shift_L, XK_Shift_R};
#endif

	std::vector<std::string> keys;
	keys.reserve(pressedKeys.size());

	// Add modifier keys in order, deduplicating display names
	// (e.g. both VK_LCONTROL and VK_RCONTROL map to "Ctrl")
	std::unordered_set<std::string> addedModifierNames;
	for (const int key : orderedKeys) {
		if (pressedKeys.count(key)) {
			std::string name = getKeyName(key);
			if (addedModifierNames.insert(name).second) {
				keys.push_back(name);
			}
		}
	}

	// Add non-modifier keys
	for (const int key : pressedKeys) {
		if (modifierKeys.find(key) == modifierKeys.end()) {
			keys.push_back(getKeyName(key));
		}
	}

	// Join keys with configured separator
	std::string combination;
	if (!keys.empty()) {
		combination = keys[0];
		for (size_t i = 1; i < keys.size(); ++i) {
			combination += keySeparator + keys[i];
		}
	}

	return combination;
}

bool shouldCaptureSingleKey(int keyCode)
{
	// Check if key is in the whitelist
	if (whitelistedKeySet.count(keyCode) > 0) {
		return true;
	}

	// Check category-based capture settings
	if (captureNumpad && numpadKeys.count(keyCode) > 0) {
		return true;
	}
	if (captureNumbers && numberKeys.count(keyCode) > 0) {
		return true;
	}
	if (captureLetters && letterKeys.count(keyCode) > 0) {
		return true;
	}
	if (capturePunctuation && punctuationKeys.count(keyCode) > 0) {
		return true;
	}

	// Check if it's in the default singleKeys set (F1-F12, Insert, Delete, etc.)
	if (singleKeys.count(keyCode) > 0) {
		return true;
	}

	return false;
}

// Lock-free variant — caller must hold keyStateMutex
static bool shouldLogCombinationLocked()
{
	// Check if SHIFT is the only modifier
	bool onlyShiftPressed = false;
#ifdef _WIN32
	onlyShiftPressed = activeModifiers.size() == 1 &&
	    (activeModifiers.count(VK_SHIFT) > 0 || activeModifiers.count(VK_LSHIFT) > 0 || activeModifiers.count(VK_RSHIFT) > 0);
#elif defined(__APPLE__)
	onlyShiftPressed = activeModifiers.size() == 1 && (activeModifiers.count(kVK_Shift) > 0 || activeModifiers.count(kVK_RightShift) > 0);
#elif defined(__linux__)
	onlyShiftPressed = activeModifiers.size() == 1 && (activeModifiers.count(XK_Shift_L) > 0 || activeModifiers.count(XK_Shift_R) > 0);
#endif

	if (onlyShiftPressed) {
		// Allow SHIFT + any non-modifier key (F keys, letters, numbers, etc.)
		// Only block SHIFT by itself (no other keys pressed)
		for (const int key : pressedKeys) {
			if (modifierKeys.find(key) == modifierKeys.end()) {
				return true; // Found a non-modifier key, so allow logging
			}
		}
		return false; // Only SHIFT is pressed, no action key
	}
	return true;
}

void emitWebSocketEvent(const std::string &keyCombination)
{
	lastKeyCombination = keyCombination;

	if (!websocket_vendor) {
		return;
	}

	obs_data_t *event_data = obs_data_create();
	obs_data_set_string(event_data, "key_combination", keyCombination.c_str());

	// Add all key presses as an array
	obs_data_array_t *key_presses_array = obs_data_array_create();
	{
		std::lock_guard<std::mutex> lock(keyStateMutex);
		for (const int key : pressedKeys) {
			obs_data_t *key_data = obs_data_create();
			obs_data_set_string(key_data, "key", getKeyName(key).c_str());
			obs_data_array_push_back(key_presses_array, key_data);
			obs_data_release(key_data);
		}
	}
	obs_data_set_array(event_data, "key_presses", key_presses_array);
	obs_data_array_release(key_presses_array);

	obs_websocket_vendor_emit_event(websocket_vendor, "key_pressed", event_data);
	obs_data_release(event_data);
}

#ifdef _WIN32
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode == HC_ACTION) {
		KBDLLHOOKSTRUCT *p = (KBDLLHOOKSTRUCT *)lParam;
		if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
			std::string keyCombination;
			bool shouldLog = false;
			{
				std::lock_guard<std::mutex> lock(keyStateMutex);
				pressedKeys.insert(p->vkCode);
				if (modifierKeys.count(p->vkCode)) {
					activeModifiers.insert(p->vkCode);
				}

				bool comboTrigger = pressedKeys.size() > 1 && isModifierKeyPressedLocked() && shouldLogCombinationLocked();
				bool singleTrigger = !comboTrigger && shouldCaptureSingleKey(p->vkCode) && !isModifierKeyPressedLocked();

				if (comboTrigger) {
					keyCombination = getCurrentCombinationLocked();
				} else if (singleTrigger) {
					keyCombination = getKeyName(p->vkCode);
				}

				if (!keyCombination.empty()) {
					if (loggedCombinations.find(keyCombination) == loggedCombinations.end()) {
						loggedCombinations.insert(keyCombination);
						shouldLog = true;
					}
				}
			}
			if (shouldLog) {
				if (enableLogging) {
					blog(LOG_INFO, "[StreamUP Hotkey Display] Keys pressed: %s", keyCombination.c_str());
				}
				if (hotkeyDisplayDock) {
					QMetaObject::invokeMethod(hotkeyDisplayDock, "setLog", Qt::QueuedConnection,
						Q_ARG(QString, QString::fromStdString(keyCombination)));
				}
				emitWebSocketEvent(keyCombination);
			}
		} else if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
			std::lock_guard<std::mutex> lock(keyStateMutex);
			pressedKeys.erase(p->vkCode);
			if (modifierKeys.count(p->vkCode)) {
				activeModifiers.erase(p->vkCode);
			}
			loggedCombinations.clear();
		}
	}
	return CallNextHookEx(keyboardHook, nCode, wParam, lParam);
}

LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode == HC_ACTION) {
		MSLLHOOKSTRUCT *p = (MSLLHOOKSTRUCT *)lParam;

		std::string keyCombination;
		bool hasModifier = false;
		{
			std::lock_guard<std::mutex> lock(keyStateMutex);
			hasModifier = isModifierKeyPressedLocked();
			if (hasModifier) {
				keyCombination = getCurrentCombinationLocked();
			}
		}

		if (hasModifier) {

			bool actionDetected = false;

			// Handle mouse button clicks
			switch (wParam) {
			case WM_LBUTTONDOWN:
				keyCombination += keySeparator + "Left Click";
				actionDetected = true;
				break;
			case WM_RBUTTONDOWN:
				keyCombination += keySeparator + "Right Click";
				actionDetected = true;
				break;
			case WM_MBUTTONDOWN:
				keyCombination += keySeparator + "Middle Click";
				actionDetected = true;
				break;
			case WM_XBUTTONDOWN:
				if (HIWORD(p->mouseData) == XBUTTON1) {
					keyCombination += keySeparator + "X Button 1";
				} else if (HIWORD(p->mouseData) == XBUTTON2) {
					keyCombination += keySeparator + "X Button 2";
				}
				actionDetected = true;
				break;
			}

			// Handle scroll actions
			if (wParam == WM_MOUSEWHEEL) {
				if (GET_WHEEL_DELTA_WPARAM(p->mouseData) > 0)
					keyCombination += keySeparator + "Scroll Up";
				else
					keyCombination += keySeparator + "Scroll Down";
				actionDetected = true;
			} else if (wParam == WM_MOUSEHWHEEL) {
				if (GET_WHEEL_DELTA_WPARAM(p->mouseData) > 0)
					keyCombination += keySeparator + "Scroll Right";
				else
					keyCombination += keySeparator + "Scroll Left";
				actionDetected = true;
			}

			// Fix #4: Cleaned up double if(enableLogging) check
			// Fix #5: Added emitWebSocketEvent for mouse actions
			// Fix #1: Use QueuedConnection for cross-thread Qt GUI access
			if (actionDetected) {
				if (enableLogging) {
					blog(LOG_INFO, "[StreamUP Hotkey Display] Mouse action detected: %s", keyCombination.c_str());
				}
				if (hotkeyDisplayDock) {
					QMetaObject::invokeMethod(hotkeyDisplayDock, "setLog", Qt::QueuedConnection,
						Q_ARG(QString, QString::fromStdString(keyCombination)));
				}
				emitWebSocketEvent(keyCombination);
			}
		}
	}
	return CallNextHookEx(mouseHook, nCode, wParam, lParam);
}

#endif

#ifdef __APPLE__
CFMachPortRef eventTap = nullptr;

bool checkMacOSAccessibilityPermissions()
{
	// Check if we have accessibility permissions
	bool hasPermission = AXIsProcessTrusted();

	if (!hasPermission) {
		blog(LOG_WARNING, "[StreamUP Hotkey Display] Accessibility permissions not granted!");
		blog(LOG_WARNING, "[StreamUP Hotkey Display] Please grant Accessibility permissions in System Preferences:");
		blog(LOG_WARNING, "[StreamUP Hotkey Display]   System Preferences -> Security & Privacy -> Privacy -> Accessibility");
		blog(LOG_WARNING, "[StreamUP Hotkey Display]   Add OBS to the list and check the checkbox");
	}

	return hasPermission;
}

void startMacOSKeyboardHook()
{
	// Check permissions first
	if (!checkMacOSAccessibilityPermissions()) {
		blog(LOG_ERROR, "[StreamUP Hotkey Display] Cannot start keyboard hook without Accessibility permissions");

		// Show user-friendly error in the dock
		if (hotkeyDisplayDock) {
			QString errorTitle = QString::fromUtf8(obs_module_text("Error.Accessibility.Permission"));
			QString errorInstructions = QString::fromUtf8(obs_module_text("Error.Accessibility.Instructions"));
			hotkeyDisplayDock->setLog(errorTitle + "\n\n" + errorInstructions);
		}
		return;
	}

	// Create event tap for keyboard AND mouse events
	CGEventMask eventMask = CGEventMaskBit(kCGEventKeyDown) | CGEventMaskBit(kCGEventKeyUp) |
				CGEventMaskBit(kCGEventLeftMouseDown) | CGEventMaskBit(kCGEventRightMouseDown) |
				CGEventMaskBit(kCGEventOtherMouseDown) | CGEventMaskBit(kCGEventScrollWheel);

	// Fix #14: Use kCGEventTapOptionListenOnly since the plugin only monitors events
	eventTap = CGEventTapCreate(kCGSessionEventTap, kCGHeadInsertEventTap, kCGEventTapOptionListenOnly,
				    eventMask, CGEventCallback, nullptr);

	if (!eventTap) {
		blog(LOG_ERROR, "[StreamUP Hotkey Display] Failed to create event tap!");
		if (hotkeyDisplayDock) {
			hotkeyDisplayDock->setLog(QString::fromUtf8(obs_module_text("Error.EventTap.Failed")));
		}
		return;
	}

	CFRunLoopSourceRef runLoopSource = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, eventTap, 0);
	CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopCommonModes);
	CGEventTapEnable(eventTap, true);
	CFRelease(runLoopSource);

	blog(LOG_INFO, "[StreamUP Hotkey Display] macOS keyboard and mouse hook started successfully");
}

void stopMacOSKeyboardHook()
{
	if (eventTap) {
		CFRelease(eventTap);
		eventTap = nullptr;
	}
}

CGEventRef CGEventCallback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void *refcon)
{
	(void)proxy;
	(void)refcon;

	// Re-enable event tap if the system disabled it
	if (type == kCGEventTapDisabledByTimeout || type == kCGEventTapDisabledByUserInput) {
		if (eventTap) {
			CGEventTapEnable(eventTap, true);
			blog(LOG_WARNING, "[StreamUP Hotkey Display] macOS event tap was disabled, re-enabled");
		}
		return event;
	}

	// Handle keyboard events
	if (type == kCGEventKeyDown || type == kCGEventKeyUp) {
		CGKeyCode keyCode = (CGKeyCode)CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode);

		std::string keyCombination;
		bool shouldLog = false;

		if (type == kCGEventKeyDown) {
			std::lock_guard<std::mutex> lock(keyStateMutex);
			pressedKeys.insert(keyCode);
			if (modifierKeys.count(keyCode)) {
				activeModifiers.insert(keyCode);
			}

			bool comboTrigger = pressedKeys.size() > 1 && isModifierKeyPressedLocked() && shouldLogCombinationLocked();
			bool singleTrigger = !comboTrigger && shouldCaptureSingleKey(keyCode) && !isModifierKeyPressedLocked();

			if (comboTrigger) {
				keyCombination = getCurrentCombinationLocked();
			} else if (singleTrigger) {
				keyCombination = getKeyName(keyCode);
			}

			if (!keyCombination.empty()) {
				if (loggedCombinations.find(keyCombination) == loggedCombinations.end()) {
					loggedCombinations.insert(keyCombination);
					shouldLog = true;
				}
			}
		} else {
			std::lock_guard<std::mutex> lock(keyStateMutex);
			pressedKeys.erase(keyCode);
			if (modifierKeys.count(keyCode)) {
				activeModifiers.erase(keyCode);
			}
			loggedCombinations.clear();
		}

		if (shouldLog) {
			if (enableLogging) {
				blog(LOG_INFO, "[StreamUP Hotkey Display] Keys pressed: %s", keyCombination.c_str());
			}
			if (hotkeyDisplayDock) {
				QMetaObject::invokeMethod(hotkeyDisplayDock, "setLog", Qt::QueuedConnection,
					Q_ARG(QString, QString::fromStdString(keyCombination)));
			}
			emitWebSocketEvent(keyCombination);
		}
	}
	// Handle mouse events (only when modifier keys are pressed)
	else if (type == kCGEventLeftMouseDown || type == kCGEventRightMouseDown ||
	         type == kCGEventOtherMouseDown || type == kCGEventScrollWheel) {
		std::string keyCombination;
		bool hasModifier = false;
		{
			std::lock_guard<std::mutex> lock(keyStateMutex);
			hasModifier = isModifierKeyPressedLocked();
			if (hasModifier) {
				keyCombination = getCurrentCombinationLocked();
			}
		}

		if (hasModifier) {
			if (type == kCGEventLeftMouseDown) {
				keyCombination += keySeparator + "Left Click";
			} else if (type == kCGEventRightMouseDown) {
				keyCombination += keySeparator + "Right Click";
			} else if (type == kCGEventOtherMouseDown) {
				int64_t buttonNumber = CGEventGetIntegerValueField(event, kCGMouseEventButtonNumber);
				if (buttonNumber == 2) {
					keyCombination += keySeparator + "Middle Click";
				} else {
					keyCombination += keySeparator + "Button " + std::to_string(buttonNumber + 1);
				}
			} else if (type == kCGEventScrollWheel) {
				int64_t deltaY = CGEventGetIntegerValueField(event, kCGScrollWheelEventDeltaAxis1);
				int64_t deltaX = CGEventGetIntegerValueField(event, kCGScrollWheelEventDeltaAxis2);

				if (deltaY > 0) {
					keyCombination += keySeparator + "Scroll Up";
				} else if (deltaY < 0) {
					keyCombination += keySeparator + "Scroll Down";
				} else if (deltaX > 0) {
					keyCombination += keySeparator + "Scroll Right";
				} else if (deltaX < 0) {
					keyCombination += keySeparator + "Scroll Left";
				}
			}

			if (enableLogging) {
				blog(LOG_INFO, "[StreamUP Hotkey Display] Mouse action detected: %s", keyCombination.c_str());
			}
			if (hotkeyDisplayDock) {
				QMetaObject::invokeMethod(hotkeyDisplayDock, "setLog", Qt::QueuedConnection,
					Q_ARG(QString, QString::fromStdString(keyCombination)));
			}
			emitWebSocketEvent(keyCombination);
		}
	}
	return event;
}
#endif

#ifdef __linux__
void linuxKeyboardHookThreadFunc()
{
	display = XOpenDisplay(nullptr);
	if (!display) {
		blog(LOG_ERROR, "[StreamUP Hotkey Display] Failed to open X display!");
		linuxHookRunning = false;
		return;
	}

	Window root = DefaultRootWindow(display);
	XSelectInput(display, root, KeyPressMask | KeyReleaseMask | ButtonPressMask);

	// Set up non-blocking event checking
	int fd = ConnectionNumber(display);
	fd_set readfds;
	struct timeval tv;

	blog(LOG_INFO, "[StreamUP Hotkey Display] Linux keyboard hook thread started");

	XEvent event;
	while (linuxHookRunning) {
		// Use select to check for events with timeout
		FD_ZERO(&readfds);
		FD_SET(fd, &readfds);
		tv.tv_sec = 0;
		tv.tv_usec = 100000; // 100ms timeout

		int result = select(fd + 1, &readfds, nullptr, nullptr, &tv);
		if (result < 0) {
			blog(LOG_ERROR, "[StreamUP Hotkey Display] select() error in X11 event loop");
			break;
		}

		if (result == 0) {
			// Timeout - check if we should continue running
			continue;
		}

		// Process all pending events
		while (linuxHookRunning && XPending(display)) {
			XNextEvent(display, &event);
			if (event.type == X11_KeyPress) {
				KeySym keysym = XLookupKeysym(&event.xkey, 0);
				std::string keyCombination;
				bool shouldLog = false;
				{
					std::lock_guard<std::mutex> lock(keyStateMutex);
					pressedKeys.insert(keysym);
					if (modifierKeys.count(keysym)) {
						activeModifiers.insert(keysym);
					}

					bool comboTrigger = pressedKeys.size() > 1 && isModifierKeyPressedLocked() && shouldLogCombinationLocked();
					bool singleTrigger = !comboTrigger && shouldCaptureSingleKey(keysym) && !isModifierKeyPressedLocked();

					if (comboTrigger) {
						keyCombination = getCurrentCombinationLocked();
					} else if (singleTrigger) {
						keyCombination = getKeyName(keysym);
					}

					if (!keyCombination.empty()) {
						if (loggedCombinations.find(keyCombination) == loggedCombinations.end()) {
							loggedCombinations.insert(keyCombination);
							shouldLog = true;
						}
					}
				}
				if (shouldLog) {
					if (enableLogging) {
						blog(LOG_INFO, "[StreamUP Hotkey Display] Keys pressed: %s", keyCombination.c_str());
					}
					if (hotkeyDisplayDock) {
						QMetaObject::invokeMethod(hotkeyDisplayDock, "setLog", Qt::QueuedConnection,
							Q_ARG(QString, QString::fromStdString(keyCombination)));
					}
					emitWebSocketEvent(keyCombination);
				}
			} else if (event.type == X11_KeyRelease) {
				KeySym keysym = XLookupKeysym(&event.xkey, 0);
				std::lock_guard<std::mutex> lock(keyStateMutex);
				pressedKeys.erase(keysym);
				if (modifierKeys.count(keysym)) {
					activeModifiers.erase(keysym);
				}
				if (activeModifiers.empty() ||
				    std::none_of(activeModifiers.begin(), activeModifiers.end(),
				                 [](int key) { return pressedKeys.count(key) > 0; })) {
					loggedCombinations.clear();
				}
			} else if (event.type == ButtonPress) {
				std::string keyCombination;
				bool hasModifier = false;
				{
					std::lock_guard<std::mutex> lock(keyStateMutex);
					hasModifier = isModifierKeyPressedLocked();
					if (hasModifier) {
						keyCombination = getCurrentCombinationLocked();
					}
				}

				if (hasModifier) {
					unsigned int button = event.xbutton.button;
					switch (button) {
					case 1:
						keyCombination += keySeparator + "Left Click";
						break;
					case 2:
						keyCombination += keySeparator + "Middle Click";
						break;
					case 3:
						keyCombination += keySeparator + "Right Click";
						break;
					case 4:
						keyCombination += keySeparator + "Scroll Up";
						break;
					case 5:
						keyCombination += keySeparator + "Scroll Down";
						break;
					case 6:
						keyCombination += keySeparator + "Scroll Left";
						break;
					case 7:
						keyCombination += keySeparator + "Scroll Right";
						break;
					case 8:
						keyCombination += keySeparator + "Back Button";
						break;
					case 9:
						keyCombination += keySeparator + "Forward Button";
						break;
					default:
						keyCombination += keySeparator + "Button " + std::to_string(button);
						break;
					}

					if (enableLogging) {
						blog(LOG_INFO, "[StreamUP Hotkey Display] Mouse action detected: %s",
						     keyCombination.c_str());
					}
					if (hotkeyDisplayDock) {
						QMetaObject::invokeMethod(hotkeyDisplayDock, "setLog", Qt::QueuedConnection,
							Q_ARG(QString, QString::fromStdString(keyCombination)));
					}
					emitWebSocketEvent(keyCombination);
				}
			}
		}
	}

	if (display) {
		XCloseDisplay(display);
		display = nullptr;
	}

	blog(LOG_INFO, "[StreamUP Hotkey Display] Linux keyboard hook thread stopped");
}

static bool isWaylandSession()
{
	const char *sessionType = getenv("XDG_SESSION_TYPE");
	const char *waylandDisplay = getenv("WAYLAND_DISPLAY");
	return (sessionType && strcmp(sessionType, "wayland") == 0) || (waylandDisplay && strlen(waylandDisplay) > 0);
}

void startLinuxKeyboardHook()
{
	if (linuxHookRunning) {
		blog(LOG_WARNING, "[StreamUP Hotkey Display] Linux hook already running");
		return;
	}

	if (isWaylandSession()) {
		blog(LOG_WARNING, "[StreamUP Hotkey Display] Wayland session detected. Global keyboard capture requires X11.");
		blog(LOG_WARNING, "[StreamUP Hotkey Display] Try running OBS with XWayland or set QT_QPA_PLATFORM=xcb");
		if (hotkeyDisplayDock) {
			QMetaObject::invokeMethod(hotkeyDisplayDock, "setLog", Qt::QueuedConnection,
				Q_ARG(QString, QString::fromUtf8(obs_module_text("Warning.Wayland"))));
		}
	}

	linuxHookRunning = true;
	linuxHookThread = std::thread(linuxKeyboardHookThreadFunc);
}

void stopLinuxKeyboardHook()
{
	if (!linuxHookRunning) {
		return;
	}

	blog(LOG_INFO, "[StreamUP Hotkey Display] Stopping Linux keyboard hook...");
	linuxHookRunning = false;

	// Wait for thread to finish
	if (linuxHookThread.joinable()) {
		linuxHookThread.join();
	}

	// Display is closed in the thread function
	display = nullptr;
}
#endif

void LoadHotkeyDisplayDock()
{
	const auto main_window = static_cast<QMainWindow *>(obs_frontend_get_main_window());
	obs_frontend_push_ui_translation(obs_module_get_string);

	hotkeyDisplayDock = new HotkeyDisplayDock(main_window);

	const QString title = QString::fromUtf8(obs_module_text("Dock.Title"));
	const auto name = "HotkeyDisplayDock";

	obs_frontend_add_dock_by_id(name, title.toUtf8().constData(), hotkeyDisplayDock);

	obs_frontend_pop_ui_translation();
}

static void applySettingsDefaults(obs_data_t *data)
{
	obs_data_set_default_string(data, "sceneName", StyleConstants::DEFAULT_SCENE_NAME);
	obs_data_set_default_string(data, "textSource", StyleConstants::DEFAULT_TEXT_SOURCE);
	obs_data_set_default_int(data, "onScreenTime", StyleConstants::DEFAULT_ONSCREEN_TIME);
	obs_data_set_default_bool(data, "displayInTextSource", false);
	obs_data_set_default_string(data, "prefix", "");
	obs_data_set_default_string(data, "suffix", "");
	obs_data_set_default_bool(data, "hookEnabled", false);
	obs_data_set_default_bool(data, "captureNumpad", false);
	obs_data_set_default_bool(data, "captureNumbers", false);
	obs_data_set_default_bool(data, "captureLetters", false);
	obs_data_set_default_bool(data, "capturePunctuation", false);
	obs_data_set_default_string(data, "whitelistedKeys", "");
	obs_data_set_default_bool(data, "enableLogging", false);
	obs_data_set_default_string(data, "keySeparator", " + ");
	obs_data_set_default_int(data, "maxHistory", 10);
}

obs_data_t *SaveLoadSettingsCallback(obs_data_t *save_data, bool saving)
{
	char *configPath = obs_module_config_path("configs.json");
	obs_data_t *data = nullptr;

	if (saving) {
		if (obs_data_save_json(save_data, configPath)) {
			blog(LOG_INFO, "[StreamUP Hotkey Display] Settings saved to %s", configPath);
		} else {
			blog(LOG_WARNING, "[StreamUP Hotkey Display] Failed to save settings to file.");
		}
	} else {
		data = obs_data_create_from_json_file(configPath);

		if (!data) {
			blog(LOG_INFO, "[StreamUP Hotkey Display] Settings not found. Creating settings file...");

			char *dirPath = obs_module_config_path("");
			os_mkdirs(dirPath);
			bfree(dirPath);

			data = obs_data_create();

			if (obs_data_save_json(data, configPath)) {
				blog(LOG_INFO, "[StreamUP Hotkey Display] Default settings saved to %s", configPath);
			} else {
				blog(LOG_WARNING, "[StreamUP Hotkey Display] Failed to save default settings to file.");
			}

			obs_data_release(data);
			data = obs_data_create_from_json_file(configPath);
		} else {
			blog(LOG_INFO, "[StreamUP Hotkey Display] Settings loaded successfully from %s", configPath);
		}
	}

	if (data) {
		applySettingsDefaults(data);
	}

	bfree(configPath);
	return data;
}

void parseWhitelistKeys(const QString &whitelist)
{
	whitelistedKeySet.clear();

	if (whitelist.isEmpty()) {
		return;
	}

	// Split by comma and process each key
	QStringList keys = whitelist.split(',', Qt::SkipEmptyParts);
	for (const QString &key : keys) {
		QString trimmedKey = key.trimmed().toUpper();

		if (trimmedKey.isEmpty()) {
			continue;
		}

		// Map common key names to key codes
#ifdef _WIN32
		// Single character keys
		if (trimmedKey.length() == 1) {
			QChar ch = trimmedKey[0];
			if (ch.isLetter()) {
				whitelistedKeySet.insert(ch.unicode());
			} else if (ch.isDigit()) {
				whitelistedKeySet.insert(ch.unicode());
			}
		}
		// Special key names
		else if (trimmedKey == "SPACE") whitelistedKeySet.insert(VK_SPACE);
		else if (trimmedKey == "TAB") whitelistedKeySet.insert(VK_TAB);
		else if (trimmedKey == "ENTER") whitelistedKeySet.insert(VK_RETURN);
		else if (trimmedKey == "ESC" || trimmedKey == "ESCAPE") whitelistedKeySet.insert(VK_ESCAPE);
		else if (trimmedKey.startsWith("F") && trimmedKey.length() >= 2) {
			bool ok = false;
			int fNum = trimmedKey.mid(1).toInt(&ok);
			if (ok && fNum >= 1 && fNum <= 12) {
				whitelistedKeySet.insert(VK_F1 + fNum - 1);
			}
		}
#endif

#ifdef __APPLE__
		// Fix #6: Use proper lookup maps instead of arithmetic on non-contiguous keycodes
		if (trimmedKey.length() == 1) {
			QChar ch = trimmedKey[0];
			if (ch.isLetter()) {
				char upper = ch.unicode();
				auto it = macLetterKeycodes.find(upper);
				if (it != macLetterKeycodes.end()) {
					whitelistedKeySet.insert(it->second);
				}
			} else if (ch.isDigit()) {
				char digit = ch.unicode();
				auto it = macDigitKeycodes.find(digit);
				if (it != macDigitKeycodes.end()) {
					whitelistedKeySet.insert(it->second);
				}
			}
		}
		// Special key names
		else if (trimmedKey == "SPACE") whitelistedKeySet.insert(kVK_Space);
		else if (trimmedKey == "TAB") whitelistedKeySet.insert(kVK_Tab);
		else if (trimmedKey == "ENTER") whitelistedKeySet.insert(kVK_Return);
		else if (trimmedKey == "ESC" || trimmedKey == "ESCAPE") whitelistedKeySet.insert(kVK_Escape);
		else if (trimmedKey.startsWith("F") && trimmedKey.length() >= 2) {
			bool ok = false;
			int fNum = trimmedKey.mid(1).toInt(&ok);
			if (ok && fNum >= 1 && fNum <= 12) {
				// macOS F-key codes: kVK_F1=0x7A, kVK_F2=0x78, etc. — not contiguous
				static const int macFKeys[] = {kVK_F1, kVK_F2, kVK_F3, kVK_F4, kVK_F5, kVK_F6,
					kVK_F7, kVK_F8, kVK_F9, kVK_F10, kVK_F11, kVK_F12};
				whitelistedKeySet.insert(macFKeys[fNum - 1]);
			}
		}
#endif

#ifdef __linux__
		// Single character keys
		if (trimmedKey.length() == 1) {
			QChar ch = trimmedKey[0];
			if (ch.isLetter()) {
				// XK_a through XK_z (lowercase)
				int lowerOffset = ch.unicode() - 'A';
				if (lowerOffset >= 0 && lowerOffset < 26) {
					whitelistedKeySet.insert(XK_a + lowerOffset);
					whitelistedKeySet.insert(XK_A + lowerOffset);
				}
			} else if (ch.isDigit()) {
				int digit = ch.digitValue();
				whitelistedKeySet.insert(XK_0 + digit);
			}
		}
		// Special key names
		else if (trimmedKey == "SPACE") whitelistedKeySet.insert(XK_space);
		else if (trimmedKey == "TAB") whitelistedKeySet.insert(XK_Tab);
		else if (trimmedKey == "ENTER") whitelistedKeySet.insert(XK_Return);
		else if (trimmedKey == "ESC" || trimmedKey == "ESCAPE") whitelistedKeySet.insert(XK_Escape);
		else if (trimmedKey.startsWith("F") && trimmedKey.length() >= 2) {
			bool ok = false;
			int fNum = trimmedKey.mid(1).toInt(&ok);
			if (ok && fNum >= 1 && fNum <= 12) {
				whitelistedKeySet.insert(XK_F1 + fNum - 1);
			}
		}
#endif
	}
}

void loadSingleKeyCaptureSettings(obs_data_t *settings)
{
	if (!settings) {
		return;
	}

	captureNumpad = obs_data_get_bool(settings, "captureNumpad");
	captureNumbers = obs_data_get_bool(settings, "captureNumbers");
	captureLetters = obs_data_get_bool(settings, "captureLetters");
	capturePunctuation = obs_data_get_bool(settings, "capturePunctuation");

	QString whitelist = QString::fromUtf8(obs_data_get_string(settings, "whitelistedKeys"));
	parseWhitelistKeys(whitelist);

	enableLogging = obs_data_get_bool(settings, "enableLogging");

	// Load separator (default " + ")
	const char *sep = obs_data_get_string(settings, "keySeparator");
	keySeparator = (sep && strlen(sep) > 0) ? sep : " + ";
}

void loadDockSettings(HotkeyDisplayDock *dock, obs_data_t *settings)
{
	if (!dock || !settings) {
		return;
	}

	QString scene = QString::fromUtf8(obs_data_get_string(settings, "sceneName"));
	dock->setSceneName(scene.isEmpty() ? StyleConstants::DEFAULT_SCENE_NAME : scene);

	QString source = QString::fromUtf8(obs_data_get_string(settings, "textSource"));
	dock->setTextSource(source.isEmpty() ? StyleConstants::DEFAULT_TEXT_SOURCE : source);

	int time = obs_data_get_int(settings, "onScreenTime");
	dock->setOnScreenTime(time > 0 ? time : StyleConstants::DEFAULT_ONSCREEN_TIME);

	dock->setPrefix(QString::fromUtf8(obs_data_get_string(settings, "prefix")));
	dock->setSuffix(QString::fromUtf8(obs_data_get_string(settings, "suffix")));
	dock->setDisplayInTextSource(obs_data_get_bool(settings, "displayInTextSource"));

	int maxHist = obs_data_get_int(settings, "maxHistory");
	dock->setMaxHistory(maxHist > 0 ? maxHist : StyleConstants::DEFAULT_MAX_HISTORY);
}

void applyDockUISettings(HotkeyDisplayDock *dock, bool hookEnabled)
{
	if (!dock) {
		return;
	}

	const char *actionText = hookEnabled ? obs_module_text("Dock.Button.Disable") : obs_module_text("Dock.Button.Enable");
	const char *state = hookEnabled ? "active" : "inactive";

	dock->getToggleAction()->setChecked(hookEnabled);
	dock->getToggleAction()->setText(actionText);
	QLabel *label = dock->getLabel();
	label->setProperty("hotkeyState", state);
	label->style()->unpolish(label);
	label->style()->polish(label);
}

static void toggleHotkeyCallback(void *data, obs_hotkey_id id, obs_hotkey_t *hotkey, bool pressed)
{
	(void)id;
	(void)hotkey;
	(void)data;
	if (!pressed || !hotkeyDisplayDock)
		return;

	// Hotkey callbacks run on a non-UI thread — marshal to main thread
	QMetaObject::invokeMethod(hotkeyDisplayDock, "toggleKeyboardHook", Qt::QueuedConnection);
}

static void frontendEventCallback(enum obs_frontend_event event, void *data)
{
	(void)data;

	if (event == OBS_FRONTEND_EVENT_FINISHED_LOADING) {
		// Event loop is now running — safe to install low-level hooks
		if (deferredHookEnable && hotkeyDisplayDock) {
			if (hotkeyDisplayDock->enableHooks()) {
				hotkeyDisplayDock->setHookEnabled(true);
			} else {
				hotkeyDisplayDock->setHookEnabled(false);
			}
			applyDockUISettings(hotkeyDisplayDock, hotkeyDisplayDock->isHookEnabled());
			deferredHookEnable = false;
		}
	} else if (event == OBS_FRONTEND_EVENT_EXIT) {
		// Unhook BEFORE the event loop stops — prevents system-wide input lag during shutdown
		if (hotkeyDisplayDock && hotkeyDisplayDock->isHookEnabled()) {
			hotkeyDisplayDock->disableHooks();
			hotkeyDisplayDock->setHookEnabled(false);
		}
	}
}

bool obs_module_load()
{
	blog(LOG_INFO, "[StreamUP Hotkey Display] loaded version %s", PROJECT_VERSION);

	LoadHotkeyDisplayDock();

	obs_data_t *settings = SaveLoadSettingsCallback(nullptr, false);

	if (settings && hotkeyDisplayDock) {
		loadDockSettings(hotkeyDisplayDock, settings);
		loadSingleKeyCaptureSettings(settings);
		bool hookEnabled = obs_data_get_bool(settings, "hookEnabled");
		if (hookEnabled) {
			// Defer hook installation until the event loop is running.
			// Low-level hooks (WH_KEYBOARD_LL, WH_MOUSE_LL) require the
			// installing thread to pump messages. During obs_module_load the
			// Qt event loop isn't running yet, so hooks time out on every
			// input event and lag the entire system.
			deferredHookEnable = true;
		}
		applyDockUISettings(hotkeyDisplayDock, false);
		obs_data_release(settings);
	} else if (hotkeyDisplayDock) {
		hotkeyDisplayDock->setSceneName(StyleConstants::DEFAULT_SCENE_NAME);
		hotkeyDisplayDock->setTextSource(StyleConstants::DEFAULT_TEXT_SOURCE);
		hotkeyDisplayDock->setOnScreenTime(StyleConstants::DEFAULT_ONSCREEN_TIME);
		hotkeyDisplayDock->setPrefix("");
		hotkeyDisplayDock->setSuffix("");
		hotkeyDisplayDock->setDisplayInTextSource(false);
		applyDockUISettings(hotkeyDisplayDock, false);
	}

	// Register OBS hotkey for toggle
	toggleHotkeyId = obs_hotkey_register_frontend("streamup_hotkey_display_toggle",
		obs_module_text("Hotkey.Toggle"), toggleHotkeyCallback, nullptr);

	// Register frontend event callback for deferred hook init and clean shutdown
	obs_frontend_add_event_callback(frontendEventCallback, nullptr);

	return true;
}

// WebSocket request callbacks
static void wsGetStatus(obs_data_t *request_data, obs_data_t *response_data, void *priv_data)
{
	(void)request_data;
	(void)priv_data;
	obs_data_set_bool(response_data, "enabled", hotkeyDisplayDock ? hotkeyDisplayDock->isHookEnabled() : false);
	obs_data_set_string(response_data, "last_combination", lastKeyCombination.c_str());
}

static void wsEnable(obs_data_t *request_data, obs_data_t *response_data, void *priv_data)
{
	(void)request_data;
	(void)priv_data;
	if (hotkeyDisplayDock && !hotkeyDisplayDock->isHookEnabled()) {
		QMetaObject::invokeMethod(hotkeyDisplayDock, "toggleKeyboardHook", Qt::QueuedConnection);
	}
	obs_data_set_bool(response_data, "success", true);
}

static void wsDisable(obs_data_t *request_data, obs_data_t *response_data, void *priv_data)
{
	(void)request_data;
	(void)priv_data;
	if (hotkeyDisplayDock && hotkeyDisplayDock->isHookEnabled()) {
		QMetaObject::invokeMethod(hotkeyDisplayDock, "toggleKeyboardHook", Qt::QueuedConnection);
	}
	obs_data_set_bool(response_data, "success", true);
}

static void wsGetLastCombination(obs_data_t *request_data, obs_data_t *response_data, void *priv_data)
{
	(void)request_data;
	(void)priv_data;
	obs_data_set_string(response_data, "combination", lastKeyCombination.c_str());
}

// WebSocket vendor registration in obs_module_post_load (correct lifecycle)
void obs_module_post_load()
{
	websocket_vendor = obs_websocket_register_vendor("streamup-hotkey-display");
	if (!websocket_vendor) {
		blog(LOG_WARNING, "[StreamUP Hotkey Display] obs-websocket not found, WebSocket features disabled");
		return;
	}

	obs_websocket_vendor_register_request(websocket_vendor, "get_status", wsGetStatus, nullptr);
	obs_websocket_vendor_register_request(websocket_vendor, "enable", wsEnable, nullptr);
	obs_websocket_vendor_register_request(websocket_vendor, "disable", wsDisable, nullptr);
	obs_websocket_vendor_register_request(websocket_vendor, "get_last_combination", wsGetLastCombination, nullptr);
}

void obs_module_unload()
{
	obs_frontend_remove_event_callback(frontendEventCallback, nullptr);

	// Safety net: hooks should already be removed by OBS_FRONTEND_EVENT_EXIT,
	// but clean up if something went wrong
#ifdef _WIN32
	if (keyboardHook) {
		UnhookWindowsHookEx(keyboardHook);
		keyboardHook = NULL;
	}
	if (mouseHook) {
		UnhookWindowsHookEx(mouseHook);
		mouseHook = NULL;
	}
#elif defined(__APPLE__)
	stopMacOSKeyboardHook();
#elif defined(__linux__)
	stopLinuxKeyboardHook();
#endif

	if (toggleHotkeyId != OBS_INVALID_HOTKEY_ID) {
		obs_hotkey_unregister(toggleHotkeyId);
	}

	websocket_vendor = nullptr;
}

MODULE_EXPORT const char *obs_module_description(void)
{
	return obs_module_text("Plugin.Description");
}

MODULE_EXPORT const char *obs_module_name(void)
{
	return obs_module_text("Plugin.Name");
}
