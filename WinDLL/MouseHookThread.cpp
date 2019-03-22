#include "MouseHookThread.h"



MouseHookThread::MouseHookThread()
{
}

hook::Callback MouseHookThread::callback;

LRESULT CALLBACK mouseHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
	PMSLLHOOKSTRUCT p = (PMSLLHOOKSTRUCT)lParam;
	if (MouseHookThread::callback)
		return MouseHookThread::callback(nCode, wParam, lParam);
	//position.x = p->pt.x;
	//position.y = p->pt.y;
	return -1;
	//return CallNextHookEx(NULL, nCode, wParam, lParam);
}

MouseHookThread::~MouseHookThread()
{
}

void MouseHookThread::run()
{
	this->mouseHook = SetWindowsHookEx(WH_MOUSE_LL, mouseHookProc, 0, 0);
	MSG msg;
	while (!GetMessage(&msg, NULL, NULL, NULL) || this->close == false) {    //this while loop keeps the hook
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

bool MouseHookThread::unhook()
{
	this->close = true;
	return UnhookWindowsHookEx(this->mouseHook);
}

