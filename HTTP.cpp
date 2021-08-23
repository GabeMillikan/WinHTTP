#include "HTTP.hpp"

namespace HTTP
{
    bool debug = true;
    std::string userAgent = "WinHTTP/1.0";
    INTERNET_PORT port = INTERNET_DEFAULT_HTTPS_PORT; // 443
    DWORD requestFlags = INTERNET_FLAG_SECURE | defaultNoCacheFlags | defaultBaseFlags;
    std::string contentType = "application/x-www-form-urlencoded";

    byte* Post(std::string URL, std::string input, DWORD* bytesRead)
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
            return nullptr;
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
            return nullptr;
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
            return nullptr;
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
            return nullptr;

        // dynamic buffer to hold file
        byte* fileBuffer = NULL;
        DWORD fileSize = 0;

        // buffer for each kb of the file
        byte* chunkBuffer = (byte*)malloc(1024);
        DWORD chunkSize = 1024;
        DWORD chunkBytesRead = 0;
        if (!chunkBuffer)
        {
            if (debug)
                std::cout << "failed to malloc for chunk buffer" << std::endl;
            free(fileBuffer);
            return nullptr;
        }

        do
        {
            chunkBytesRead = 0;
            InternetReadFile(hRequest, chunkBuffer, chunkSize, &chunkBytesRead);

            byte* re = (byte*)realloc(fileBuffer, fileSize + chunkBytesRead);
            if (!re)
            {
                if (debug)
                    std::cout << "failed to realloc dynamic buffer" << std::endl;
                free(fileBuffer);
                free(chunkBuffer);
                return nullptr;
            }
            else
            {
                fileBuffer = re;
            }
            memcpy(fileBuffer + fileSize, chunkBuffer, chunkBytesRead);
            fileSize += chunkBytesRead;

        } while (chunkBytesRead > 0);
        free(chunkBuffer);

        if (fileSize == 0)
        {
            if (debug)
                std::cout << "Got no bytes in response w/ error: " << GetLastError() << std::endl;
            free(fileBuffer);
            return nullptr;
        }
        if (bytesRead)
            *bytesRead = fileSize;

        return fileBuffer;
    }
}