#include "Response.hpp"
#include "MAIN.hpp"


Response::Response(Request *request, Config &conf)
{
    // this->status_code needs to be taken from request as well
    this->version = "HTTP/1.1 ";
    this->request = request;
    this->config = conf;
    std::string uri = "";
    prop_map map = this->config.get_map();
    const std::string prop = "listen";
    std::vector<std::string>::iterator vect = map[prop].begin();

    std::cout << *vect << std::endl << std::endl << std::endl << std::endl << std::endl << std::endl << std::endl << std::endl << std::endl << std::endl;

    fill_phrase();
    fill_status_line();
    if (!hasExt(uri))
    {
        if (!check_autoindex())
        {

            if (is_non_static(uri))
                serve_non_static(uri);
            else 
                ft_error(404, "hasExt()");
        }
        else 
        {
            std::cout <<"create\n";
            create_autoindex();
        }
    }
    
    fill_headers();

    // fill_body();
}

void Response::create_autoindex()
{
	std::string body = "";
	body += "<!DOCTYPE html>\n";
	body += "<html>\n";
	body += "<head>\n";
	body += "</head>\n";
	body += "<body>";
	body += "autoindex page\n\n";
	body += "<br><br>";
	makeFileList(body);
	body += "</body>\n";
	body += "</html>\n";
	this->content += body;
}

void Response::makeFileList(std::string &body)
{
    DIR *dir = NULL;
	struct dirent *directory = NULL;
    prop_map::const_iterator map = this->config.get_map().find("root");

    std::cout << this->request->get_uri() << std::endl;
	if ((dir = opendir((*map->second.begin() + this->request->get_uri()).c_str())) == NULL)
		ft_error(500, "autoindex");
	while ((directory = readdir(dir)))
	{
		std::string file_name(directory->d_name);
		if (file_name != ".")
		{
			body += "<a href=http://" + this->request->get_headers().find("Host")->second + "/" + file_name + " >" + file_name + "</a><br>";
		}
	}
	closedir(dir);
}

int Response::check_autoindex()
{
    prop_map::const_iterator map = this->config.get_map().find("index");
    int flag = 0;
    std::vector<const std::string>::iterator indexs;
    std::string root;

    if (this->config.get_map().count("root"))
        root = this->config.get_map().find("root")->second[0];
    if (map != this->config.get_map().end())
        indexs = map->second.begin();
    else
        flag = 1;
    if ((this->config.get_map().find("autoindex") != this->config.get_map().end()) && this->config.get_map().find("autoindex")->second[0] == "on")
    {
        if (this->config.get_map().count("index"))
            return 1;
        while ( flag != 1 && indexs != this->config.get_map().find("index")->second.end())
        {
            std::cout << "mtav\n";
            std::cout << (root + "/" + *indexs) <<  "incg=vor ban\n";
            if (isExist(root + "/" + *indexs))
                return 0;
            indexs++;
        }
        return 1;
    }
    return 0;
}

Response::~Response()
{
}

char    *Response::get_c_response()
{
    return this->c_response;
}

void    Response::fill_phrase()
{
    if (this->status_code == 200)
        this->phrase = "OK";
    //consider other status codes
}

void    Response::fill_status_line()
{
    this->status_line += this->version + std::to_string(this->status_code) + " " + this->phrase + "\r\n";
    this->raw_response += this->status_line;
}

void    Response::fill_headers()
{
    std::cout << "->" << raw_response << "<- "<<  std::endl;
    this->raw_response += "Content-Type: text/html\nContent-Length: ";//<html>\n<body>\n<h1>Hello from server</h1>\n</body>\n</html>";
    this->raw_response += std::to_string(this->content.size()) + "\n\n";
    this->raw_response += this->content;
    // std::cout << this->raw_response << std::endl;
    this->c_response = (char *)this->raw_response.c_str(); // consider casting into non-const again
}

// void    Response::fill_body()
// {

// }

void    Response::serve_non_static(std::string file_uri)
{

    std::vector<std::string> cgi_pass = config.get_map().find("cgi_pass")->second;

    std::cout << "CGI STUFF\n";
    std::string cgi_ext = "";
    std::string cgi_path = "";
    std::string temp = this->request->get_uri();

    if (cgi_pass.size() == 2)
    {
        cgi_ext = cgi_pass.at(0);
        cgi_path = cgi_pass.at(1);
        // if ()
        CGI cgi = CGI(cgi_ext, cgi_path, file_uri, &config, request);
        this->content = cgi.get_reponse();
    }
    else
    {
        // 500 to be returned
    } 
}

void   Response::open_static(std::string file)
{
    std::cout << "\n\nIN_OPEN\n";
    std::string line;
    std::string path = "./www" + file;
    std::cout << "PATH: " << path << "\n";
    std::ifstream static_file(path);

    if (static_file.is_open())
    {
        while (static_file)
        {
            std::getline(static_file, line);
            this->content += line;
        }
        // while (static_file.good())
        //     static_file >> static_file_content;
        // std::cout << "CONTENT: " << static_file_content << "\n";
        static_file.close();
    }
    else
    {
        this->status_code = 404;
        std::cout << "COULDNT OPEN FILE\n";
    }
}

int Response::is_non_static(std::string uri)
{
    std::vector<const std::string>::iterator temp = this->config.get_map().find("cgi_pass")->second.begin();
    std::string ext = uri.substr(uri.find_first_of(".", 0) + 1, uri.size() - uri.find_first_of(".", 0));
    std::vector<Location> locs = this->config.get_locations();
    std::vector<Location>::iterator locs_iter = locs.begin();

    if (*temp == ext)
        return 1;
    while (locs_iter != locs.end())
    {
        std::cout << locs_iter->get_path() << std::endl;
        if (locs_iter->get_path() == uri)
        {
            if (locs_iter->get_props().count("cgi_pass"))
                return (1);
        }
        locs_iter++;
    }
    return 0;
}


int Response::hasExt(std::string &uri)
{
    std::string temp;
    size_t match = 0;
    
    uri = this->request->get_uri();
    if (((match = uri.find_first_of("?", 0)) < uri.size()))
        uri.erase(match, uri.size() - match);
    if ((match = uri.find_last_of(".")) > uri.find_last_of("/"))
    {
        temp = uri.substr(match + 1, uri.size() - match);
        if (temp == "html" || temp == "htm")
        {
            std::cout << "URI: " << uri << "\n";
            open_static(uri);
        }
            return (1);
    }
    return 0;
}