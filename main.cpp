#include "HTTP.hpp"

struct pastebin
{
	char str[181];
};

int main()
{
	std::string out;
	bool success = HTTP::Post("https://www.example.com/api/example.php", "the data to post", out);
	std::cout << "output: " << out << std::endl;
}