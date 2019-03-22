#include"config.hpp"
//#pragma comment(lib,"C:\\Users\\User\\source\\repos\\POCO_SETUP_PARSER\\packages\\boost_filesystem-vc141.1.69.0.0\\lib\\nativelibboost_filesystem-vc141-mt-s-x64-1_69.lib")

#include "windows.h"
#include "winnls.h"
#include "shobjidl.h"
#include "objbase.h"
#include "objidl.h"
#include "shlguid.h"
#include<string>
#include<cwchar>
#include<cstdio>
#include"boost/filesystem.hpp"
#include <psapi.h>
#include<iostream>
#include<map>
#include<cstdio>
#include<tchar.h>

#include"hookThread.h"
#include"MouseHookThread.h"

std::pair<DWORD, std::wstring> PrintProcessNameAndID(DWORD processID)
{
	TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");

	// Get a handle to the process.

	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |
		PROCESS_VM_READ,
		FALSE, processID);

	// Get the process name.

	if (NULL != hProcess)
	{
		HMODULE hMod;
		DWORD cbNeeded;

		if (EnumProcessModules(hProcess, &hMod, sizeof(hMod),
			&cbNeeded))
		{
			GetModuleBaseName(hProcess, hMod, szProcessName,
				sizeof(szProcessName) / sizeof(TCHAR));
		}
	}

	// Print the process name and identifier.
	std::pair<DWORD, std::wstring> result{ processID,szProcessName };
	//_tprintf(TEXT("%s  (PID: %u)\n"), szProcessName, processID);

	// Release the handle to the process.

	CloseHandle(hProcess);
	return result;
}


extern "C" {
	DLL bool createSymLink(const char* target, const char* source, int flags) {
		return CreateSymbolicLinkA(target, source, (DWORD)flags);
	}

	DLL bool createNormalLink(const char* path, const char* linkname, const char* source, const char* workingDirectory = nullptr, const char* desc = nullptr) {
		HRESULT hres;
		IShellLinkA* psl;
		CoInitialize(nullptr);
		hres = CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_IShellLinkA, (LPVOID*)&psl);
		if (SUCCEEDED(hres)) {
			IPersistFile* ppf;
			std::string _target = path + std::string("\\") + std::string(linkname) + ".lnk";
			psl->SetPath(source);
			if (desc != nullptr)
				psl->SetDescription(desc);
			if (workingDirectory != nullptr)
				psl->SetWorkingDirectory(workingDirectory);
			else {
				boost::filesystem::path p(source);
				psl->SetWorkingDirectory(p.parent_path().string().c_str());
			}
			hres = psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf);
			if (SUCCEEDED(hres)) {
				WCHAR *wsz;
				int nChars = MultiByteToWideChar(CP_ACP, 0, _target.c_str(), -1, nullptr, 0);
				wsz = new WCHAR[nChars];
				MultiByteToWideChar(CP_ACP, 0, _target.c_str(), -1, (LPWSTR)wsz, nChars);

				hres = ppf->Save(wsz, true);
				ppf->Release();
			}
			psl->Release();
		}
		return hres;
	}


	DLL BOOL IsElevated() {
		BOOL fRet = FALSE;
		HANDLE hToken = NULL;
		if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
			TOKEN_ELEVATION Elevation;
			DWORD cbSize = sizeof(TOKEN_ELEVATION);
			if (GetTokenInformation(hToken, TokenElevation, &Elevation, sizeof(Elevation), &cbSize)) {
				fRet = Elevation.TokenIsElevated;
			}
		}
		if (hToken) {
			CloseHandle(hToken);
		}
		return fRet;
	}

	DLL bool ExecuteProcess(const char* program, const char* commandline) {
		STARTUPINFOA si;
		PROCESS_INFORMATION pi;
		return CreateProcessA(program, (LPSTR)commandline, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
	}

	DLL bool ExecuteAsRoot(const char* program, const char* commandline, unsigned int flags) {
		auto v = ShellExecuteA(NULL, "runas", program, commandline, NULL, SW_HIDE);
		if (v != 0) return true;
		return false;
	}

	DLL void* getProcesses() {
		DWORD aProcesses[1024], cbNeeded, cProcesses;
		//std::cout << "Inside getProcesses\n";
		auto result = new std::map<DWORD, std::wstring>();
		if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded))
		{
			std::cout << "ERROR ENUMERATING PROCESSES!\n";
		}
		cProcesses = cbNeeded / sizeof(DWORD);
		for (UINT i = 0; i < cProcesses; i++) {
			//std::cout << "Inside LOOP\n";
			if (aProcesses[i] == 0) continue;
			auto res = PrintProcessNameAndID(aProcesses[i]);
			result->insert(std::make_pair(res.first, res.second));
		}
		return (void*)result;
	}

	DLL bool terminateByPID(unsigned long pid) {
		auto hProc = OpenProcess(PROCESS_ALL_ACCESS, false, (DWORD)pid);
		return TerminateProcess(hProc, 1);
	}

	DLL bool terminateAllProcessesByName(const wchar_t* name) {
		auto res = (std::map<DWORD, std::wstring>*)getProcesses();
		bool succeeded = true;
		for (auto item : *res) {
			if (item.second == std::wstring(name)) {
				if (!terminateByPID(item.first))
				{
					succeeded = false;
					std::wcout << L"ERROR at terminating PROCESS " << item.second << " with PID: " << item.first << L'\n';
				}
			}
		}
		return succeeded;
	}
	
	HHOOK hkb = NULL;
	DLL bool UnHook(void* hookObj) {

		((hookThread*)hookObj)->stopHook();
		delete ((hookThread*)hookObj);
		return true;
	}
	

	DLL void* InstallHook()
	{
		//hkb = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, 0, 0);
		auto f = (new hookThread());
		return f;
	}

	DLL void setupKeyboardHookCallback(void* func) {
		hookThread::callback = *(hook::Callback*)func;
	}

	DLL void setupMouseHookCallback(void* func) {
		MouseHookThread::callback = *(hook::Callback*)func;
	}

	DLL bool UnHookMouse(void* hookObj) {
		((MouseHookThread*)hookObj)->unhook();
		delete ((MouseHookThread*)hookObj);
		return true;
	}

	DLL void* InstallMouseHook() {
		auto f = (new MouseHookThread());
		return f;
	}
	
}

BOOL APIENTRY DllMain(HANDLE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	hInst = (HINSTANCE)hModule;

	return TRUE;
}
