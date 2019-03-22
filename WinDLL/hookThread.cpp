#include "hookThread.h"


LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	BOOL fEatKeystroke = FALSE;
	if (hookThread::callback)
		return hookThread::callback(nCode, wParam, lParam);
	/*if (nCode == HC_ACTION)
	{
		switch (wParam)
		{
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
		case WM_KEYUP:
		case WM_SYSKEYUP:
			PKBDLLHOOKSTRUCT p = (PKBDLLHOOKSTRUCT)lParam;
			if (fEatKeystroke = (p->vkCode == 0x41)) {     //redirect a to b
				printf("Hello a\n");
				return -1;
				keybd_event('B', 0, 0, 0);
				keybd_event('B', 0, KEYEVENTF_KEYUP, 0);
				break;
			}
			break;
		}
	}*/
	return -1;
	return(fEatKeystroke ? 1 : CallNextHookEx(NULL, nCode, wParam, lParam));
}

hook::Callback hookThread::callback;

hookThread::hookThread()
{
}


hookThread::~hookThread()
{
}

void hookThread::run()
{
	this->hookKeyboard = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, 0, 0);
	MSG msg;
	while (!GetMessage(&msg, NULL, NULL, NULL) || this->close == false) {    //this while loop keeps the hook
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

void hookThread::stopHook()
{
	this->close = true;
	UnhookWindowsHookEx(this->hookKeyboard);
}

