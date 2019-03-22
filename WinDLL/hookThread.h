#pragma once
#include"config.hpp"
#include<Poco/Runnable.h>
#include<functional>
#include<Windows.h>
static HINSTANCE hInst = NULL;
//LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);



class DLL hookThread : public Poco::Runnable
{
public:
	static hook::Callback callback;
	hookThread();
	~hookThread();

	void run() override;
	void stopHook();
	
private:
	HHOOK hookKeyboard;
	
	bool close = false;
};

