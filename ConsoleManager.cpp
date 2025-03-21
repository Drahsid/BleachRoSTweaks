#include "ConsoleManager.hpp"

static bool consoleAllocated = false;

bool ConsoleManager::Allocate() {
    if (consoleAllocated) {
        return false;
    }

    try {
        if (!AllocConsole()) {
            throw std::runtime_error("Failed to AllocConsole");
        }

        // Redirect stdout
        FILE* fp_stdout;
        if (freopen_s(&fp_stdout, "CONOUT$", "w", stdout) != 0 || !fp_stdout) {
            throw std::runtime_error("Failed to redirect stdout");
        }

        // Redirect stderr
        FILE* fp_stderr;
        if (freopen_s(&fp_stderr, "CONOUT$", "w", stderr) != 0 || !fp_stderr) {
            throw std::runtime_error("Failed to redirect stderr");
        }

        // Redirect stdin
        FILE* fp_stdin;
        if (freopen_s(&fp_stdin, "CONIN$", "r", stdin) != 0 || !fp_stdin) {
            throw std::runtime_error("Failed to redirect stdin");
        }

        consoleAllocated = true;
        return true;
    }
    catch (const std::exception&) {
        FreeConsole();
        return false;
    }

    return false;
}

bool ConsoleManager::Free() {
    if (!consoleAllocated) {
        return false;
    }

    fclose(stdout);
    fclose(stderr);
    fclose(stdin);
    consoleAllocated = false;
    return FreeConsole();
}
