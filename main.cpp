#include "HTTP.hpp"

struct pastebin
{
	char str[181];
};

int main()
{
	std::string out = "";
	bool success = HTTP::Post(
		"https://pastebin.com/api/api_post.php",
	"api_dev_key=glerPifRdWtMqYW5FIUMwqn6lZrir5CC&api_option=paste&api_paste_private=0&api_paste_name=myname.js&api_paste_expire_date=10M&api_paste_format=javascript&api_paste_code=ayoo", &out);

	std::cout << success << ": " << out << std::endl;
}