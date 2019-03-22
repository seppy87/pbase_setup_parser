#include<iostream>
#include<functional>

extern "C"{
	__declspec(dllexport) const char* getCommand() {
		return "testme";
	}

	__declspec(dllexport) int getArgCount() {
		return 1;
	}

	__declspec(dllexport) const char* getDLLVersion() {
		return "1.0.0";
	}

	__declspec(dllexport) void* getMainFunc(const char* arg) {
		std::function<bool(const char*)> *func = new std::function<bool(const char*)>([](const char* arg) {std::cout << arg << '\n'; return true; });
		return (void*)func;
	}

}