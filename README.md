# pbase_setup_parser
Setup Parser based on POCO Framework

A Parser to parse commands which will be executed.

This is in early stage. It shall support Windows and Linux and maybe FreeBSD.

The Folder Structure:

-Plugins\ {All DLLs in hier}
-Executable

Command to start:

file.exe /f=Path/to/INSTRUCTIONS.psi

COMMANDs:

Windows   ||    LINUX    || Description
/f=path         -f:path       Path to instructions File
/insecure       --insecure    Sets Programm to insecure. It enables the loading Modules from Internet and loading all Modules

Structure of PSI FILE:

"//" <-- It means Comment and will be ignored by the parser

First Word is the Command and will be followed but subcommand (if existant) and/or Arguemnets. Everything is seperated by "||".

COMMANDS in INSTRUCTION FILE:

1 WRITE - DISPLAYS Text in Console or file (File not SUPPORTED)
1.1. <target> - Specifies target can be cout or file
1.1.1 <text> - the text which will be written
Format:write||TARGET||TEXT
example:write||cout||TEXT TO DISPLAY

1. pget - Downloads File from the Internet
1.1. <outputtarget><optional> filename and path
1.1.1. <required> URL
FORMAT:pget||outputtarget||url
example:pget||c:\temp\myfile.7z||https://test.test/myfile.7z
or
FORMAT:pget||url
example:pget||https://test.test/test.txt

15. loadModules - Loads Modules so the Parser can use these commands (which are not part of the standard commands
15.1. <required> first Module to load
15.1.x. all other Modules to load sperated by ||
FORMAT:loadModules||Module1||Module2||...
example:loadModules||modbus||modusb

17. hook - Hooks Functions (Mouse and Keyboard support atm). Has subcommands
hook||<required subcommand>||<required first argument>||<required second argument if subcommand is either kbfunc or msfunc>
17.1. kb - Subcommand for hooking Keyboard
17.1.1. - boolean: Hook(true) or Unhook(false) the Keyboard
17.2. ms - Subcommand for hooking mouse
17.3. kbfunc - sets hook Callback. If no Callback is set the command only blocks the input of the keyboard until it is unhooked or program closed
17.3.1. Path to DLL with HOOKCALLBACK Function
17.3.1.1. Symbolname for dynamic runtime loading
17.4. msfun . set hook Callback for Mouse. If no Callback is set the command only blocks the input of Mouse until it is unhooked or program closed.
17.4.1. Path to DLL with HOOKCALLBACK Function
17.4.1.1. Symbolname for dynamic runtime loading

