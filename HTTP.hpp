#pragma once

#include <string>
#include <cassert>
#include <iostream>
#include <vector>

// networking libraries
#include <windows.h>
#include <wininet.h>
#pragma comment(lib, "wininet.lib")

namespace HTTP
{
    constexpr DWORD defaultNoCacheFlags = INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_RELOAD | INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_RELOAD;
    constexpr DWORD defaultBaseFlags = INTERNET_FLAG_NO_COOKIES | INTERNET_FLAG_NO_UI;

    extern bool debug;
    extern std::string userAgent;
    extern INTERNET_PORT port;
    extern DWORD requestFlags;
    extern std::string contentType;

    extern byte* Post(std::string URL, std::string input, DWORD* bytesRead);
};