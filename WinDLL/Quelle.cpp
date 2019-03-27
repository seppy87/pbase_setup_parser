#include"config.hpp"
//#pragma comment(lib,"C:\\Users\\User\\source\\repos\\POCO_SETUP_PARSER\\packages\\boost_filesystem-vc141.1.69.0.0\\lib\\nativelibboost_filesystem-vc141-mt-s-x64-1_69.lib")
#pragma comment(lib,"setupapi.lib")
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
#include<SetupAPI.h>

#include"hookThread.h"
#include"MouseHookThread.h"
#include"thirdparty/WinReg.hpp"

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

	bool changeRealKeyValue(winreg::RegKey& oKey, const char* key, DWORD regType, const BYTE* data) {
		int size_needed = MultiByteToWideChar(CP_UTF8, 0, key, strlen(key), NULL, 0);
		std::wstring kstr(size_needed, 0);
		MultiByteToWideChar(CP_UTF8, 0, key, strlen(key), &kstr[0], size_needed);
		
		DWORD datasize;
		switch (regType) {
		case REG_SZ:
		{
			const char* str = (const char*)data;
			size_needed = MultiByteToWideChar(CP_UTF8, 0, str, strlen(str), NULL, 0);
			std::wstring vstr(size_needed, 0);
			MultiByteToWideChar(CP_UTF8, 0, str, strlen(str), &vstr[0], size_needed);
			oKey.SetStringValue(kstr, vstr);
			oKey.Close();
			return true;
		}
		case REG_DWORD:
			oKey.SetDwordValue(kstr, (*(DWORD*)data));
			oKey.Close();
			return true;
		case REG_BINARY:
			//std::string input(data);
		{
			std::vector<unsigned char> data_vec(data, data + sizeof(data));
			oKey.SetBinaryValue(kstr, data_vec);
			oKey.Close();
			return true;
		}
		case REG_QWORD:
			oKey.SetQwordValue(kstr, (const ULONGLONG)*data);
			oKey.Close();
			return true;
		default:
			std::cout << "ERROR THIS SHOULD NOT HAPPEN!\n";
				return false;
		}
		//LONG setRES = RegSetValueExA(hKey, key, 0, regType, data, datasize);
		
		return true;
	}


	DLL bool changeRegKeyValue(unsigned short main, const char* sub, const char* key,DWORD regType, const BYTE* data) {
		int size_needed = MultiByteToWideChar(CP_UTF8, 0, sub, strlen(sub), NULL, 0);
		std::wstring wstr(size_needed, 0);
		MultiByteToWideChar(CP_UTF8, 0, sub, strlen(sub), &wstr[0], size_needed);
		switch (main) {
		case 1:
		{
			
			winreg::RegKey k{ HKEY_CLASSES_ROOT,wstr };
			return changeRealKeyValue(k,key,regType, data);
		}
		case 2: {
			winreg::RegKey k{ HKEY_CURRENT_CONFIG,wstr };
			return changeRealKeyValue(k, key, regType, data);
		}
		case 3: {
			winreg::RegKey k{ HKEY_CURRENT_USER,wstr };
			return changeRealKeyValue(k, key, regType, data);
		}
		case 4: {
			winreg::RegKey k{ HKEY_LOCAL_MACHINE,wstr };
			return changeRealKeyValue(k, key, regType, data);
		}
		case 5: {
			winreg::RegKey k{ HKEY_USERS,wstr };
			return changeRealKeyValue(k, key, regType, data);
		}
		default:
			std::cout << "ERROR this should not happen!\n";
			return false;
		}
		
		return true;
	}

	void* getRealRegValue(winreg::RegKey& oKey, const char* key, DWORD type) {
		int size_needed = MultiByteToWideChar(CP_UTF8, 0, key, strlen(key), NULL, 0);
		std::wstring wkey(size_needed, 0);
		MultiByteToWideChar(CP_UTF8, 0, key, strlen(key), &wkey[0], size_needed);
		switch (type) {
		case REG_SZ:
		{
			auto val = oKey.GetStringValue(wkey);
			size_needed = WideCharToMultiByte(CP_UTF8, 0, &val[0], (int)val.size(), NULL, NULL, NULL, NULL);
			char* ret = new char[size_needed];
			WideCharToMultiByte(CP_UTF8, 0, &val[0], (int)val.size(), ret, size_needed, NULL, NULL);
			oKey.Close();
			return ret;
		}
		case REG_DWORD:
		{
			DWORD *val = new DWORD;
			*val = oKey.GetDwordValue(wkey);
			oKey.Close();
			return val;
		}
		case REG_BINARY:
		{
			std::vector<BYTE>* val = new std::vector<BYTE>();
			*val = oKey.GetBinaryValue(wkey);
			oKey.Close();
			return val;
		}
		case REG_QWORD:
		{
			ULONGLONG* val = new ULONGLONG;
			*val = oKey.GetQwordValue(wkey);
			oKey.Close();
			return val;
		}
		case REG_MULTI_SZ: {
			std::vector<std::wstring>* val = new std::vector<std::wstring>();
			*val = oKey.GetMultiStringValue(wkey);
			oKey.Close();
			return val;
		}
		default:
			std::cout << "This should not happen!\n";
			return nullptr;
		}
	}

	DLL void* getRegKeyValue(unsigned short main, const char* sub, const char* key, DWORD type) {
		int size_needed = MultiByteToWideChar(CP_UTF8, 0, sub, strlen(sub), NULL, 0);
		std::wstring wstr(size_needed, 0);
		MultiByteToWideChar(CP_UTF8, 0, sub, strlen(sub), &wstr[0], size_needed);
		winreg::RegKey* k;
		switch (main) {
		case 1: {
			k = new winreg::RegKey(HKEY_CLASSES_ROOT,wstr);
			break;
		}
		case 2: {
			k = new winreg::RegKey{ HKEY_CURRENT_CONFIG,wstr };
			break;
		}
		case 3: {
			k = new winreg::RegKey{ HKEY_CURRENT_USER,wstr };
			break;
		}
		case 4: {
			k= new winreg::RegKey { HKEY_CURRENT_USER,wstr };
			break;
		}
		case 5:
			k = new winreg::RegKey{ HKEY_USERS,wstr };
			break;
		default:
			std::cout << "This should not happen!\n";
			return nullptr;
		}
		auto res = getRealRegValue(*k, key, type);
		delete k;
		return res;
	}

	void createRealRegKey(winreg::RegKey& k, const char* key) {
		
	}
	
	DLL void createRegKey(unsigned int main, const char* sub, DWORD Type = 0, const char* value = NULL, void* data = NULL) {
		int size_needed = MultiByteToWideChar(CP_UTF8, 0, sub, strlen(sub), NULL, NULL);
		wchar_t* wstr = new wchar_t[size_needed];
		MultiByteToWideChar(CP_UTF8, 0, sub, strlen(sub), wstr, size_needed);
		winreg::RegKey k;
		winreg::RegKey* tKey = NULL;
		switch (main) {
		case 1:
			k.Create(HKEY_CLASSES_ROOT, wstr);
			k.Close();
			if (Type != 0) {
				tKey = new winreg::RegKey{ HKEY_CLASSES_ROOT,wstr };
			}
			break;
		case 2:
			k.Create(HKEY_CURRENT_CONFIG, wstr);
			k.Close();
			if (Type != 0) {
				tKey = new winreg::RegKey{ HKEY_CURRENT_CONFIG,wstr };
			}
			break;
		case 3:
			k.Create(HKEY_CURRENT_USER, wstr);
			k.Close();
			if (Type != 0) {
				tKey = new winreg::RegKey{ HKEY_CURRENT_USER,wstr };
			}
			break;
		case 4:
			k.Create(HKEY_LOCAL_MACHINE, wstr);
			k.Close();
			if (Type != 0) {
				tKey = new winreg::RegKey{ HKEY_LOCAL_MACHINE,wstr };
			}
			break;
		case 5:
			k.Create(HKEY_USERS, wstr);
			k.Close();
			if (Type != 0) {
				tKey = new winreg::RegKey{ HKEY_USERS,wstr };
			}
			break;
		default:
			std::cout << "This should not happen!\n";
			delete[] wstr;
			return;
		}
		changeRealKeyValue(*tKey, value, Type, (BYTE*)data);
		delete tKey;
		delete[] wstr;
	}

	DLL void removeRegKey(unsigned int main, const char* sub, const char* keyToDelete) {
		int size_needed = MultiByteToWideChar(CP_UTF8, 0, sub, strlen(sub), NULL, NULL);
		wchar_t* wstr = new wchar_t[size_needed];
		MultiByteToWideChar(CP_UTF8, 0, sub, strlen(sub), wstr, size_needed);
		winreg::RegKey* k;
		switch (main) {
		case 1:
			k = new winreg::RegKey{ HKEY_CLASSES_ROOT,wstr };
			break;
		case 2:
			k = new winreg::RegKey{ HKEY_CURRENT_CONFIG,wstr };
			break;
		case 3:
			k = new winreg::RegKey{ HKEY_CURRENT_USER,wstr };
			break;
		case 4:
			k = new winreg::RegKey{ HKEY_LOCAL_MACHINE,wstr };
			break;
		case 5:
			k = new winreg::RegKey{ HKEY_USERS,wstr };
			break;
		default:
			std::cout << "THIS SHOULD NOT HAPPEN!\n";
			delete[] wstr;
			return;
		}
		size_needed = MultiByteToWideChar(CP_UTF8, 0, keyToDelete, strlen(keyToDelete), NULL, NULL);
		wchar_t* wktd = new wchar_t[size_needed];
		MultiByteToWideChar(CP_UTF8, 0, keyToDelete, strlen(keyToDelete), wktd, size_needed);
		k->DeleteKey(wktd, KEY_ALL_ACCESS);
		delete[] wstr;
		delete[] wktd;
		delete k;
	}

	DLL void removeRegValue(unsigned int main, const char* sub, const char* value) {
		int size_needed = MultiByteToWideChar(CP_UTF8, 0, sub, strlen(sub), NULL, NULL);
		wchar_t* wstr = new wchar_t[size_needed];
		MultiByteToWideChar(CP_UTF8, 0, sub, strlen(sub), wstr, size_needed);
		winreg::RegKey* k;
		switch (main) {
		case 1:
			k = new winreg::RegKey{ HKEY_CLASSES_ROOT,wstr };
			break;
		case 2:
			k = new winreg::RegKey{ HKEY_CURRENT_CONFIG,wstr };
			break;
		case 3:
			k = new winreg::RegKey{ HKEY_CURRENT_USER,wstr };
			break;
		case 4:
			k = new winreg::RegKey{ HKEY_LOCAL_MACHINE,wstr };
			break;
		case 5:
			k = new winreg::RegKey{ HKEY_USERS,wstr };
			break;
		default:
			std::cout << "This should not happen!\n";
			return;
		}
		size_needed = MultiByteToWideChar(CP_UTF8, 0, value, strlen(value), NULL, NULL);
		wchar_t* wValue = new wchar_t[size_needed];
		MultiByteToWideChar(CP_UTF8, 0, value, strlen(value), wValue, size_needed);
		k->DeleteValue(wValue);
		delete[] wstr;
		delete[] wValue;
		delete k;
	}

	DLL void* enumRegValues(unsigned int main, const char* sub) {
		int size_needed = MultiByteToWideChar(CP_UTF8, 0, sub, strlen(sub), NULL, NULL);
		wchar_t* wstr = new wchar_t[size_needed];
		MultiByteToWideChar(CP_UTF8, 0, sub, strlen(sub), wstr, size_needed);
		winreg::RegKey* k;
		std::wstring mstr;
		switch (main) {
		case 1:
			k = new winreg::RegKey{ HKEY_CLASSES_ROOT,wstr };
			mstr = L"HKEY_CLASSES_ROOT\\"+std::wstring(wstr);
			break;
		case 2:
			k = new winreg::RegKey{ HKEY_CURRENT_CONFIG, wstr };
			mstr = L"HKEY_CURRENT_CONFIG\\" + std::wstring(wstr);
			break;
		case 3:
			k = new winreg::RegKey{ HKEY_CURRENT_USER,wstr };
			mstr = L"HKEY_CURRENT_USER\\" + std::wstring(wstr);
			break;
		case 4:
			k = new winreg::RegKey{ HKEY_LOCAL_MACHINE,wstr };
			mstr = L"HKEY_LOCAL_MACHINE\\" + std::wstring(wstr);
			break;
		case 5:
			k = new winreg::RegKey{ HKEY_USERS,wstr };
			mstr = L"HKEY_CLASSES_ROOT\\" + std::wstring(wstr);
			break;
		default:
			std::cout << "This should not happen!\n";
			return NULL;
		}
		auto kSubKey = k->EnumSubKeys();
		auto kValues = k->EnumValues(); 
		delete[] wstr;
		delete k;
		return new std::pair<std::vector<std::wstring>, std::vector<std::pair<std::wstring, DWORD>>>(std::make_pair(kSubKey, kValues));
	}

	DLL void installDeviceDriver(const char* path, unsigned int flag, const char* installType) {
		std::string insType(installType);
		std::string infpath(path);
		std::string strflag = std::to_string(flag);
		std::string cmd = insType + " " + strflag + " " + infpath;
		InstallHinfSectionA(NULL, NULL, cmd.c_str(), 0);
	}

	DLL void uninstallDeviceDriver() {

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
