#pragma once
#include"config.hpp"
#include<Windows.h>
#include<Poco/Runnable.h>


class DLL MouseHookThread : public Poco::Runnable
{
public:
	static hook::Callback callback;
	MouseHookThread();
	~MouseHookThread();
	void run();
	bool unhook();
	
private:
	HHOOK mouseHook;
	
	bool close = false;
};

