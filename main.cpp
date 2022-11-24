# include "MAIN.hpp"
# include "Server/Headers.hpp"


bool isExist(std::string path)
{
	struct stat info;
	return (stat(path.c_str(), &info) == 0);
}

int main(int ac, char **av)
{
    // std::string str = "GET /hello.htm HTTP/1.1\r\nUser-Agent: Mozilla/4.0 (compatible; MSIE5.01; Windows NT)\r\nHost: www.tutorialspoint.com\r\n\r\nrandombody";
    // Request r = Request(str);
    
    std::vector<Config> configs;
    if (ac == 2)
        start_parse(av[1], configs);

    // Server s = Server();
    // std::cout << "in main" << configs.size() << std::endl;
    Webserv webserv = Webserv(configs);
    
    return 0;
}