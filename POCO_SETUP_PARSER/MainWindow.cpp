#include"MainWindow.hpp"
#include<Poco/Net/NetException.h>
#include<iostream>



INITIALIZE_EASYLOGGINGPP;

std::map<std::string, UINT> MainWindow::OrderMap;



void MainWindow::initialize(Poco::Util::Application & application)
{
	
	::el::Loggers::reconfigureAllLoggers(buglogger::conf);
	LOG(INFO) << "INITIALIZATION";
	this->loadConfiguration();
	this->loadOSSpecific();
	if (!this->isElevated()) {
		std::cout << "WARNING: Process has no Administration/Root rights!\n";
	}
	MainWindow::OrderMap = boost::assign::map_list_of("write", 1)("pget",2)("pdll",3)("unzip",4)("un7z",5)("mkdir",6)("mklink",7)("symlink",8)("incdll",9)("loadall",10)("copy",11)("move",12)("rename",13)("delete",14)("loadModules",15)("enumproc",16)("hook",17)("sleep",18);
	Poco::Util::Application::initialize(application);

}

void MainWindow::uninitialize()
{
	LOG(INFO) << "UNINITIALIZE";
	for (auto iter : this->libs) {
		iter.second->unload();
	}
	if (this->kbHooked)
	{
		this->reactivateKeyboard(std::any_cast<Poco::Runnable*>(this->vars["kbhook"]));
	}
	if (this->mouseHooked) {
		this->reactivateMouse(std::any_cast<Poco::Runnable*>(this->vars["mousehook"]));
	}
	if (this->callbackLib->isLoaded())
		this->callbackLib->unload();
	Poco::Util::Application::uninitialize();
}

void MainWindow::defineOptions(Poco::Util::OptionSet & optionSet)
{
	using namespace Poco::Util;
	Poco::Util::Application::defineOptions(optionSet);
	optionSet.addOption(Option("file", "f", "Instruction File")
		.required(false)
		.repeatable(false)
		.argument("PathToFile", true)
		.callback(OptionCallback<MainWindow>(this,&MainWindow::handleCommand))
	);
	optionSet.addOption(
		Option("archive","a","Archive File")
		.required(false)
		.repeatable(false)
		.argument("PathToArchive",true)
		.callback(OptionCallback<MainWindow>(this,&MainWindow::handleCommand))
	);
	optionSet.addOption(
		Option("insecure","","Insecure Mode")
		.noArgument()
		.required(false)
		.repeatable(false)
		.callback(OptionCallback<MainWindow>(this,&MainWindow::setInsecure))
	);
}

void MainWindow::handleCommand(const std::string & name, const std::string & value)
{
	this->config().setString(name, value);
}

void MainWindow::setInsecure(const std::string & name, const std::string & value)
{
	std::cout << "[WARNING] DO YOU REALLY WANT TO SET THE PROGRAMM TO INSECURE MODE?(write \"yes\" for insecure mode!\n";
	std::string answer;
	std::getline(std::cin, answer);
	if (answer == "yes") {
		this->insecure = true;
	}
}

void MainWindow::setDefaults()
{
	if (!this->config().hasProperty("archive")) {
		this->config().setString("archive", "install.7z");
	}
	if (!this->config().hasProperty("file")) {
		this->config().setString("file", "install.psi");
	}
}

int MainWindow::main(const std::vector<std::string>& arguements)
{
	this->setDefaults();
	using std::ifstream;
	using std::copy;

	ifstream psi(this->config().getString("file"));
	std::vector<std::string> lines;
	//copy(std::istream_iterator<std::string>(psi), std::istream_iterator<std::string>(), std::back_inserter(lines));
	std::string temp;
	while (std::getline(psi, temp)) {
		lines.insert(lines.end(), temp);
	}
	try {
		for (auto line : lines) {
			if (line[0] == '/' && line[1] == '/') continue; // ignores Comments
			this->parseLine(line);
		}
	}
	catch (Poco::Exception& ex) {
		std::cout << ex.displayText() << '\n';
		//return -1;
	}
	system("pause");
	return 0;
}

void MainWindow::parseLine(const std::string & line)
{
	std::vector<std::string> vec;
	boost::split(vec, line, boost::is_any_of("||"));
	for (auto iter = vec.begin(); iter != vec.end(); iter++) {
		if (iter->empty()) {
			iter = vec.erase(iter);
		}
	}
	switch (MainWindow::OrderMap[vec[0]]) {
	case 1:
		if (vec[1] == "cout")
		{
			std::cout << vec[2] << '\n';
			return;
		}
		return;
	case 2:
		try {
			if (vec.size() == 2) {
				this->downloadFile(vec[1], "test.txt");
			}
		}
		catch (Poco::Exception& ex) {
			std::cout << ex.displayText();
		}
		return;
	case 6:
	{
		if (vec.size() == 2) {
			Poco::File f(vec[1]);
			f.createDirectory();
		}
		if (vec.size() == 3) {
			if (vec[1] == "true") {
				Poco::File f(vec[2]);
				f.createDirectories();
			}
			else {
				Poco::File f(vec[2]);
				f.createDirectory();
			}
		}
	}
		return;
	case 7:
		this->createShortcut(vec);
		return;
	case 9:
		this->loadDLL(vec[1]);
		return;
	case 10:
		if (this->insecure) {
			this->loadAllDlls();
		}
		else {
			std::cout << "ERROR: Can't load all Modules if program is not set as insecure!\n";
			LOG(ERROR) << "INSECURE CODE IN SECURE MODE IS FORBIDDEN";
		}
		return;
	case 16:
		if (vec[1][0] == '$') {
			auto s = vec[1].substr(1, vec[1].size() - 1);
			auto m = (std::map<DWORD, std::wstring>*)this->enumProcess();
			std::cout << m->size()<<'\n';
			for (auto item : *m) {
				std::wcout << item.second << '\n';
			}
			this->vars.insert(std::make_pair(s, *m));
		}
		return;
	case 17:
		if (vec[1] == "kb") {
			if (vec[2] == "true") {
				this->kbHooked = true;
				auto erg = (Poco::Runnable*)this->deactivateKeyboard();
				Poco::ThreadPool::defaultPool().start(*erg);
				this->vars["kbhook"] = erg;
			}
			else {
				this->reactivateKeyboard(std::any_cast<Poco::Runnable*>(this->vars["kbhook"]));
			}
			return;
		}
		if (vec[1] == "ms") {
			if (vec[2] == "true") {
				auto mouseHook = (Poco::Runnable*)this->deactivateMouse();
				Poco::ThreadPool::defaultPool().start(*mouseHook);
				this->vars["mousehook"] = mouseHook;
			}
			else {
				this->reactivateMouse(std::any_cast<Poco::Runnable*>(this->vars["mousehook"]));
			}
			return;
		}
		using func_callback = LRESULT(*)(int, WPARAM, LPARAM);
		if (vec[1] == "kbfunc") {
			if(!this->callbackLib->isLoaded())
				this->callbackLib->load(vec[2]);
			
			auto f = new std::function<LRESULT(int, WPARAM, LPARAM)>([&](int nCode, WPARAM wParam, LPARAM lParam) {return ((func_callback)this->callbackLib->getSymbol(vec[3]))(nCode, wParam, lParam); });
			this->setupKeyboardCallback((void*)f);
			return;
		}
		if (vec[1] == "msfunc") {
			if (!this->callbackLib->isLoaded())
				this->callbackLib->load(vec[2]);
			auto f = new std::function<LRESULT(int, WPARAM, LPARAM)>([&](int nCode, WPARAM wParam, LPARAM lParam) {return ((func_callback)this->callbackLib->getSymbol(vec[3]))(nCode, wParam, lParam); });
			this->setupMouseCallback((void*)f);
			return;
		}
		return;
	case 18: 
		{
			if (vec.size() < 2) {
				LOG(ERROR) << "SLEEP NEEDS ONE ARGUEMENT";
				return;
			}
			auto time = std::stoi(vec[1]);
			Sleep(time);
		}
		return;
	default:
		if (this->libs.find(vec[0]) != this->libs.end()) {
			this->pass2DLL(vec);
		}
	}
}

void MainWindow::downloadFile(const std::string & url, const std::string & path)
{
	Poco::Net::initializeNetwork();
	Poco::Net::initializeSSL();
	Poco::Net::HTTPStreamFactory::registerFactory();
	Poco::Net::HTTPSStreamFactory::registerFactory();
	Poco::Net::FTPStreamFactory::registerFactory();
	Poco::SharedPtr<Poco::Net::InvalidCertificateHandler> ptrCert = new Poco::Net::ConsoleCertificateHandler(false);
	Poco::Net::Context::Ptr ptrContext = new Poco::Net::Context(Poco::Net::Context::CLIENT_USE, "");//(Poco::Net::Context::CLIENT_USE, "", "", "rootcert.pem", Poco::Net::Context::VERIFY_RELAXED, 9, false, "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH");
	Poco::Net::SSLManager::instance().initializeClient(0, ptrCert, ptrContext);
	try {
		Poco::URI uri(url);
		std::ofstream fout(path.c_str());
		std::unique_ptr<std::istream> pStr(Poco::URIStreamOpener::defaultOpener().open(uri));
		Poco::StreamCopier::copyStream(*pStr.get(), fout);
	}
	catch (Poco::Net::DNSException& ex) {
		std::cout << ex.message() << '\n';
		return;
	}
	Poco::Net::uninitializeSSL();
}

void MainWindow::loadOSSpecific()
{
	auto os = Poco::Environment::os();
	switch (os)
	{
	case POCO_OS_WINDOWS_NT:
		this->loadWindowsNTModules();
		return;
	case POCO_OS_LINUX:
		this->loadLinuxModules();
		break;
	case POCO_OS_FREE_BSD:
		break;
	default:
		break;
	}
}

void MainWindow::loadDLL(const std::string & name)
{
	
//	typedef const char*(*funcGetCommand)();
	using funcGetCommand = const char*(*)();
	Poco::SharedPtr<Poco::SharedLibrary> dll(new Poco::SharedLibrary("Plugins\\" + name));
	if (!dll->hasSymbol("getCommand")) {
		std::cout << "DLL is not qualified as Plugin!\n";
		return;
	}
	funcGetCommand getCommand = static_cast<funcGetCommand>(dll->getSymbol("getCommand"));
	std::cout << "Adding Function: " << getCommand() << '\n';
	this->libs[getCommand()] = dll;

}

void MainWindow::pass2DLL(std::vector<std::string>& args)
{
	Poco::SharedPtr<Poco::SharedLibrary> dll = this->libs[args[0]];
	if (!dll->hasSymbol("getArgCount")) {
		std::cout << "Plugin does not contain Arguement Counter!\n";
		return;
	}
	//typedef int(*func_getArgCounter)();
	using func_getArgCounter = int(*)();
	//typedef void*(*func_getMainFunc)();
	using func_getMainFunc = void*(*)();
	func_getArgCounter getArgCount = static_cast<func_getArgCounter>(dll->getSymbol("getArgCount"));
	auto c = getArgCount();
	if (c < 0) {
		std::cout << "Function say it takes " << c << " Arguements. No negative Numbers allowed!\n";
		return;
	}
	if (!dll->hasSymbol("getMainFunc")) {
		std::cout << "Cannot extract Main Plugin Function!\n";
		return;
	}
	func_getMainFunc getMainFunc = static_cast<func_getMainFunc>(dll->getSymbol("getMainFunc"));
	switch (c) {
	case 0:
	{
		auto mfunc = static_cast<std::function<bool()>*>(getMainFunc());
		if (!(*mfunc)()) {
			std::cout << "Function call failed!\n";
			return;
		}
		std::cout << "Function call successfully executed!\n";
	}
		break;
	case 1:
	{
		auto mfunc = static_cast<std::function<bool(const char*)>*>(getMainFunc());
		if (!(*mfunc)(args[1].c_str())) {
			std::cout << "Function call failed!\n";
			return;
		}
		std::cout << "Function call successfully executed!\n";
	}
	break;
	case 3:
		if ((*static_cast<std::function<bool(const char*, const char*)>*>(getMainFunc()))(args[1].c_str(), args[2].c_str())) {
			std::cout << "Function successfully executed!\n";
		}
		else {
			std::cout << "Function failed!\n";
		}
		break;
	case 4:
		if ((*static_cast<std::function<bool(const char*, const char*, const char*, const char*)>*>(getMainFunc()))(args[1].c_str(), args[2].c_str(), args[3].c_str(), args[4].c_str())) {
			std::cout << "Function Successfully executed\n";
		}
		else {
			std::cout << "Function failed!\n";
		}
		break;
	default:
		std::cout << "Not implemented or not found!\n";
	}
}

void MainWindow::loadAllDlls()
{
	using namespace Poco;
	DirectoryIterator iter(std::string("Plugins"));
	DirectoryIterator end;
	for (; iter != end; iter++) {
		if (iter->isDirectory()) continue;
		if (iter->isFile()) {
			if (getExtension(iter->path()) == "dll") {
				Poco::SharedPtr<Poco::SharedLibrary> dll(new Poco::SharedLibrary(iter->path()));
				using func_getCommand = const char*(*)();
				if (!dll->hasSymbol("getCommand")) {
					std::cout << "The DLL " << iter->path() << " is not qualified to be a Plugin. getCommand Function is missing!\n";
					continue;
				}
				func_getCommand getCommand = static_cast<func_getCommand>(dll->getSymbol("getCommand"));
				std::string cmd = getCommand();
				if (this->libs.find(cmd) != this->libs.end()) {
					std::cout << "Command " << cmd << " has already been loaded. Do you want to overwrite?\n";
					std::string answer;
					std::getline(std::cin, answer);
					if (answer != "yes") continue;
					this->libs[cmd]->unload();
				}
				std::cout << "adding Function " << cmd << '\n';
				this->libs[cmd] = dll;
			}
		}
	}
}


void MainWindow::createShortcut(const std::vector<std::string>& args)
{
	bool ret = false;
	switch (args.size()) {
	case 6:
		ret = this->mklink(args[1].c_str(), args[2].c_str(), args[3].c_str(), args[4].c_str(), args[5].c_str());
		break;
	case 5:
		ret = this->mklink(args[1].c_str(), args[2].c_str(), args[3].c_str(), args[4].c_str(), NULL);
		break;
	case 4:
		ret = this->mklink(args[1].c_str(), args[2].c_str(), args[3].c_str(), NULL, NULL);
		break;
	default:
		std::cout << "Too Little Arguements!\n";
	}
	if (ret) {
		std::cout << "Creation of the Link succeeded!\n";
		return;
	}
	std::cout << "Creation failed\n";
}

void MainWindow::loadWindowsNTModules()
{
	try {
		this->OSLib = new Poco::SharedLibrary("Plugins\\WinDLL.dll");
		using func_createSymLink = bool(*)(const char*, const char*, int);
		using func_createShortcut = bool(*)(const char*, const char*, const char*, const char*, const char*);
		using func_isElevated = BOOL(*)();
		using func_enumProcess = void*(*)();
		using func_LockInput = void*(*)();
		using func_UnLockInput = bool(*)(void*);
		using func_setupCallback = void(*)(void*);
		//func_createSymLink createSymLink = (func_createSymLink)OSLib->getSymbol("createSymLink");
		this->symlink = [&](const char* target, const char* source, int flags) { return ((func_createSymLink)this->OSLib->getSymbol("createSymLink"))(target, source, flags); };
		this->mklink = [&](const char* path, const char* linkname, const char* source, const char* workingDirectory, const char* desc) {return ((func_createShortcut)this->OSLib->getSymbol("createNormalLink"))(path, linkname, source, workingDirectory, desc); };
		this->isElevated = [&]() {return ((func_isElevated)this->OSLib->getSymbol("IsElevated"))(); };
		this->enumProcess = [&]() {return ((func_enumProcess)this->OSLib->getSymbol("getProcesses"))(); };
		this->deactivateKeyboard = [&]() {this->kbHooked = true; return ((func_LockInput)this->OSLib->getSymbol("InstallHook"))(); };
		this->reactivateKeyboard = [&](void* hookObj) {this->kbHooked = false; return ((func_UnLockInput)this->OSLib->getSymbol("UnHook"))(hookObj); };
		this->deactivateMouse = [&]() { this->mouseHooked = true; return ((func_LockInput)this->OSLib->getSymbol("InstallMouseHook"))(); };
		this->reactivateMouse = [&](void* hookObj) {this->mouseHooked = false; return ((func_UnLockInput)this->OSLib->getSymbol("UnHookMouse"))(hookObj); };
		this->setupKeyboardCallback = [&](void* callback) {((func_setupCallback)this->OSLib->getSymbol("setupKeyboardHookCallback"))(callback); };
		this->setupMouseCallback = [&](void *callback) {((func_setupCallback)this->OSLib->getSymbol("setupMouseHookCallback"))(callback); };
	}
	catch (Poco::Exception& ex) {
		LOG(FATAL) << ex.displayText();
	}
}

void MainWindow::loadLinuxModules()
{
	this->OSLib = new Poco::SharedLibrary("Plugins/LinDLL");
	using func_createSymLink = bool(*)(const char*, const char*, int);
	using func_createShortcut = bool(*)(const char*, const char*, const char*, const char*, const char*);
	using func_isElevated = BOOL(*)();
	//func_createSymLink createSymLink = (func_createSymLink)OSLib->getSymbol("createSymLink");
	this->symlink = [&](const char* target, const char* source, int flags) { return ((func_createSymLink)this->OSLib->getSymbol("createSymLink"))(target, source, flags); };
	this->mklink = [&](const char* path, const char* linkname, const char* source, const char* workingDirectory, const char* desc) {return ((func_createShortcut)this->OSLib->getSymbol("createNormalLink"))(path, linkname, source, workingDirectory, desc); };
	this->isElevated = [&]() {return ((func_isElevated)this->OSLib->getSymbol("IsElevated"))(); };

}

POCO_APP_MAIN(MainWindow)