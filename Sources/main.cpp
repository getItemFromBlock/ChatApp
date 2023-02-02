#include <iostream>

#include "Core/App.hpp"

#ifdef _DEBUG
#include <crtdbg.h>
#else
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif

int main()
{
#ifdef _DEBUG
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    //_CrtSetBreakAlloc(4698);
#endif
    {
        Core::App app;
        if (app.Init()) return -1;
        app.Run();
    }
}