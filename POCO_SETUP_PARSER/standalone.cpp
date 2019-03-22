#include"MainWindow.hpp"

std::string getExtension(const std::string& path) {
	size_t pos = path.rfind('.');
	return path.substr(pos + 1);
}

static HHOOK g_hhkKeyboard = NULL;

LRESULT myLowLevelKeyboardProc(int code, WPARAM wParam, LPARAM lParam)
{
	LOG(INFO) << "INSIDE PROC";
	KBDLLHOOKSTRUCT* details = (KBDLLHOOKSTRUCT*)lParam;

	if (code == HC_ACTION && wParam == WM_KEYDOWN)
	{
		if (details->vkCode == 0x41)
		{
			return 1;
		}
	}
	return 1;
	//return CallNextHookEx(g_hhkKeyboard, code, wParam, lParam);
}

bool InstallHook() {
	g_hhkKeyboard = SetWindowsHookEx(WH_KEYBOARD, myLowLevelKeyboardProc, 0, 0);
	if (g_hhkKeyboard == NULL)
		LOG(ERROR) << "ERROR - HOOK FAILED";
	return true;
}

