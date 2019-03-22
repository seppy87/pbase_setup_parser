#pragma once

#define POCO_STATIC
#ifndef IMPORT
#define DLL __declspec(dllexport)
#else
#define DLL __declspec(dllimport)
#endif

#include<functional>
#include<Windows.h>

namespace hook {
	using Callback = std::function<LRESULT(int, WPARAM, LPARAM)>;
}