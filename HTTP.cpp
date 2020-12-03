#include "HTTP.hpp"

namespace HTTP
{
    bool debug = true;
    std::string userAgent = "WinHTTP/1.0";
    INTERNET_PORT port = INTERNET_DEFAULT_HTTPS_PORT; // 443
    DWORD requestFlags = INTERNET_FLAG_SECURE | defaultNoCacheFlags | defaultBaseFlags;
    std::string contentType = "application/x-www-form-urlencoded";

    bool Post(std::string URL, std::string input, std::string& output)
    {
        if (debug)
        {
            std::cout << "HTTP::Post to url " << URL << std::endl;
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

        // "Long pointer to a null-terminated array of string pointers indicating content types accepted by the client"
        LPCSTR acceptContentTypes[]{ "*/*", 0 };

        HINTERNET hRequest = HttpOpenRequest(hConnection,
            "POST",
            directory.c_str(),
            "HTTP/1.1",
            0,
            acceptContentTypes,
            requestFlags,
            0
        );
        if (!hRequest)
        {
            if (debug)
                std::cout << "Failed to open hRequest w/ error: " << GetLastError() << std::endl;
            return false;
        }

        bool headersAdded = HttpAddRequestHeaders(
            hRequest,
            ("Content-Type: " + contentType + "\r\n").c_str(),
            48,
            HTTP_ADDREQ_FLAG_REPLACE
        );

        bool requestSuccess = HttpSendRequest(hRequest,
            // headers and length, not supported yet
            NULL, 0,
            // post data and post length
            (void*)input.c_str(), input.size()
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

        // read response
        DWORD bytesRead = 0;
        size_t totalBytesRead = 0;
        char* buffer = (char*)malloc(257); // read 256 chars at a time
        if (!buffer)
        {
            if (debug)
                std::cout << "failed to malloc 257 bytes for response buffer" << std::endl;
            return false;
        }
        do {
            InternetReadFile(hRequest, buffer, 256, &bytesRead);
            buffer[bytesRead] = NULL;
            output += buffer;
            totalBytesRead += bytesRead;
        } while (bytesRead > 0);

        free(buffer);
        return true;
    }
}