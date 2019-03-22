#pragma once
#define POCO_STATIC
//#define POCO_NO_AUTOMATIC_LIBS
#pragma comment(lib, "IPHLPAPI.lib") 
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib, "Crypt32.lib")

#ifdef _DEBUG
#pragma comment(lib,"debug\\easyloggingpp.lib")
#else
#pragma comment(lib,"release\\easyloggingpp.lib")
#endif


//#pragma comment(lib,"C:\\Users\\User\\Downloads\\openssl.v140.windesktop.msvcstl.dyn.rt-dyn.x64.1.0.2.1\\lib\\native\\v140\\windesktop\\msvcstl\\dyn\\rt-dyn\\x64\\debug\\ssleay32.lib")
//#pragma comment(lib,"C:\\Users\\User\\Downloads\\openssl.v140.windesktop.msvcstl.dyn.rt-dyn.x64.1.0.2.1\\lib\\native\\v140\\windesktop\\msvcstl\\dyn\\rt-dyn\\x64\\debug\\libeay32.lib")
#include<Poco/Util/Application.h>
#include<Poco/Util/Option.h>
#include<Poco/Util/OptionCallback.h>
#include<Poco/Util/OptionSet.h>
#include<Poco/Zip/ZipArchive.h>
#include<Poco/SevenZip/Archive.h>
#include<Poco/DirectoryIterator.h>
#include "Poco/URIStreamOpener.h"
#include "Poco/StreamCopier.h"
#include "Poco/Path.h"
#include "Poco/URI.h"
#include "Poco/SharedPtr.h"
#include "Poco/Exception.h"
#include "Poco/Net/HTTPStreamFactory.h"
#include "Poco/Net/HTTPSStreamFactory.h"
#include "Poco/Net/FTPStreamFactory.h"
#include "Poco/Net/SSLManager.h"
#include "Poco/Net/KeyConsoleHandler.h"
#include "Poco/Net/ConsoleCertificateHandler.h"
#include<Poco/Platform.h>
#include<Poco/SharedLibrary.h>


//STL
#include <fstream>
#include<iterator>
#include<algorithm>
#include <memory>
#include<any>

//boost
#include<boost/algorithm/string.hpp>
#include<boost/algorithm/string/split.hpp>
#include<boost/assign.hpp>
#include"easylogging++.h"


//LRESULT CALLBACK myLowLevelKeyboardProc(int code, WPARAM wParam, LPARAM lParam);
bool InstallHook();


namespace buglogger {
	static ::el::Configurations conf("buglogger\\log.conf");
}

std::string getExtension(const std::string& path);

class MainWindow : public Poco::Util::Application {
protected:
	static std::map<std::string, UINT> OrderMap;
	bool insecure = false;
	std::map<std::string, std::any> vars;

	std::map<std::string, Poco::SharedPtr<Poco::SharedLibrary>> libs;
	void initialize(Poco::Util::Application& application) override;
	void uninitialize() override;
	void defineOptions(Poco::Util::OptionSet& optionSet) override;
	void handleCommand(const std::string& name, const std::string& value);
	void setInsecure(const std::string& name, const std::string& value);
	void setDefaults();


	//MAIN
	int main(const std::vector<std::string>& arguements) override;

	//Parser
	void parseLine(const std::string& line);

	void downloadFile(const std::string& url, const std::string& path = "");

	//load special dll
	void loadOSSpecific();
	void loadDLL(const std::string& name);
	void pass2DLL(std::vector<std::string>& args);
	void loadAllDlls();
	
	void createShortcut(const std::vector<std::string>& args);


	void loadWindowsNTModules();
	void loadLinuxModules();


private:
	Poco::SharedPtr<Poco::SharedLibrary> OSLib;
	Poco::SharedPtr<Poco::SharedLibrary> callbackLib;

	std::function<bool(const char*, const char*, int)> symlink;
	std::function<bool(const char*, const char*, const char*, const char*, const char*)> mklink;
	std::function<BOOL()> isElevated;
	std::function<void*()> enumProcess;
	std::function<void*()> deactivateKeyboard;
	std::function<bool(void*)> reactivateKeyboard;
	std::function<void*()> deactivateMouse;
	std::function<bool(void*)> reactivateMouse;
	std::function<void(void*)> setupKeyboardCallback;
	std::function<void(void*)> setupMouseCallback;
	bool kbHooked = false;
	bool mouseHooked = false;
};