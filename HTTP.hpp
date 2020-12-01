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

    /*
        A post function used when the response is known,
        for example, an api where you supply a 4 byte int, 
        and it responds with a 4 byte int which is the input + 1
    */
    template <class PostData, class ResponseFormat>
    bool StructuredPost(std::string URL, PostData postData, ResponseFormat* output)
    {
        size_t responseSize = output ? sizeof(ResponseFormat) : 0;

        if (debug)
        {
            std::cout << "HTTP::Post to url " << URL << std::endl;
            if (!output)
                std::cout << "(ignoring any response)" << std::endl;
        }

        // split string into host and directory
        // [ garbage ][     host part    ][     directory part     ]
        //   https://    www.example.com     /subdirectory/api.php

        // remove protocol from url
        if (URL.rfind("http://") == 0)
        {
            URL = URL.substr(7);
        }
        else if (URL.rfind("https://") == 0)
        {
            URL = URL.substr(8);
        }

        // split host and directory
        int directoryIndex = URL.find("/");
        std::string host = URL;
        std::string directory = "";
        if (directoryIndex >= 0)
        {
            host = URL.substr(0, directoryIndex);
            directory = URL.substr(directoryIndex + 1);
        }

        if (debug)
        {
            std::cout << "Determined that host = \"" << host << "\", and directory = \"" << directory << "\"" << std::endl;
        }

        HINTERNET hInternet = InternetOpen(
            userAgent.c_str(),
            INTERNET_OPEN_TYPE_PRECONFIG,
            NULL, NULL,
            0
        );
        if (!hInternet)
        {
            if (debug)
                std::cout << "Failed to open hInternet w/ error: " << GetLastError() << std::endl;
            return false;
        }

        HINTERNET hConnection = InternetConnect(
            hInternet,
            host.c_str(),
            INTERNET_DEFAULT_HTTPS_PORT,
            NULL, NULL,
            INTERNET_SERVICE_HTTP,
            0, 0
        );
        if (!hConnection)
        {
            if (debug)
                std::cout << "Failed to open hConnection w/ error: " << GetLastError() << std::endl;
            return false;
        }

        LPCSTR acceptedContentTypes[] = { "application/octet-stream", NULL };
        HINTERNET hRequest = HttpOpenRequest(hConnection,
            "POST",
            directory.c_str(),
            "HTTP/1.1",
            0,
            // Long pointer to a null-terminated array of string pointers indicating content types accepted by the client
            acceptedContentTypes,
            requestFlags,
            0
        );
        if (!hRequest)
        {
            if (debug)
                std::cout << "Failed to open hRequest w/ error: " << GetLastError() << std::endl;
            return false;
        }

        bool requestSuccess = HttpSendRequestA(hRequest,
            // headers and length, not supported yet
            NULL, 0,
            // post data and post length
            (void*)&postData, sizeof(postData)
        );

        if (debug)
        {
            if (requestSuccess)
                std::cout << "Request completed successfully" << std::endl;
            else
                std::cout << "Request failed, error: " << GetLastError() << std::endl;
        }

        if (!requestSuccess)
            return false;

        if (!output)
            return true;

        DWORD bytesRead = 0;
        char* buffer = (char*)malloc(responseSize + 1); // add one just to check if server has more to say after ResponseFormat
        if (!buffer)
        {
            if (debug)
                std::cout << "failed to malloc " << responseSize + 1 << " bytes for response buffer" << std::endl;
            return false;
        }
        bool readSuccess = InternetReadFile(hRequest, buffer, responseSize + 1, &bytesRead);

        if (debug)
        {
            if (readSuccess)
                std::cout << "InternetReadFile completed successfully" << std::endl;
            else
                std::cout << "InternetReadFile failed, error: " << GetLastError() << std::endl;
        }

        if (!readSuccess)
        {
            free(buffer);
            return false;
        }

        if (debug)
        {
            if (bytesRead == responseSize)
                std::cout << "Server response is valid, copying to output" << std::endl;
            else if (bytesRead > responseSize)
                std::cout << "WARNING: Server responded with " << bytesRead << " or more bytes, but only expected " << responseSize << " bytes" << std::endl;
            else
                std::cout << "Server response too short, expected " << responseSize << " bytes, but only got " << bytesRead << " bytes" << std::endl;
        }

        if (bytesRead < responseSize)
        {
            free(buffer);
            return false;
        }

        memcpy(output, buffer, responseSize);
        free(buffer);
        return true;
    }

    /*
        a universal post function that works with strings
    */
    extern bool Post(std::string URL, std::string input, std::string& output);
};