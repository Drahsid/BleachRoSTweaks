#pragma once

#include <windows.h>
#include <iostream>
#include <io.h>
#include <fcntl.h>

class ConsoleManager {
public:
    static bool Allocate();
    static bool Free();
};
