#include "HTTP.hpp"

int main()
{
	DWORD bytesRead = 0;
	byte* bResponse = HTTP::Post("www.google.com", "[insert post data here]", &bytesRead);

	// convert bytes to string
	std::string response(bResponse, bResponse + bytesRead);

	std::cout << "output: " << response << std::endl;
}