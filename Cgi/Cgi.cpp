#include "Cgi.hpp"
#include <string>
#include <sys/socket.h>
#include <signal.h>


CGI::CGI() {}
CGI::CGI(std::string ext, std::string cgi_path, std::string file, Config *conf, Request *req): 
    file(file), extention(ext), path(cgi_path), config(conf), request(req) 
{
    this->env = Env();
    this->reponse = "";
    find_abs_path(); // ./
    find_exec_file();
    set_env();
    cgi_handler();
}

void CGI::set_env()
{
    
}

void CGI::find_abs_path()
{
    // char cwd[256];

    // getcwd(cwd, 256);
    // this->path += std::string(cwd);
}

prop_map CGI::hasLocation()
{
    std::vector<Location>::iterator loc_iter = this->config->get_locations().begin();
    std::string plain_uri;
    prop_map props = config->get_map();

    while (loc_iter != this->config->get_locations().end())
    {
        plain_uri = this->request->get_uri().substr(this->request->get_uri().find_first_of('?'), this->request->get_uri().size() - this->request->get_uri().find_first_of('?'));
        if (plain_uri == loc_iter->get_path())
            props = loc_iter->get_props();
        ++loc_iter;
    }
    return props;
}

// std::string CGI::find_ext_file()
// {

// }

std::string CGI::full_file()
{
    std::cout << "CGI RESPONSE\n";
    std::vector<std::string> root = this->config->get_map().find("root")->second;
    std::string ffile;

    if (root.size() == 1)
    {
        if (root.at(0).at(0) == '/')
            ffile = root.at(0) + this->file;
        // else
            
    }
    else
    {
        // 500 to be returned
    }
}



std::string CGI::get_reponse()
{
    return this->reponse;
}

/** 
 *
 * 
 * find index file for uris
 * 
 * **/
void CGI::find_exec_file()
{
    std::vector<const std::string>::iterator index_iter;
    std::string temp_ext;
    std::string root = this->config->get_map().find("root")->second[0];

    if (this->file.empty() || this->file == "/")
    {
        if (this->config->get_map().count("index"))
        {
            index_iter = this->config->get_map().find("index")->second.begin();
            // std::cout << *index_iter << std::endl;
            while (index_iter != this->config->get_map().find("index")->second.end())
            {
                temp_ext = index_iter->substr(index_iter->find_last_of('.') + 1, (index_iter->size() - 1 - index_iter->find_last_of('.')));
                if (temp_ext == this->extention)
                {
                    this->file = *index_iter;
                    break ;
                }
                ++index_iter;
            }
        }
        else
        {
            ft_error(404, "find_exec_file()");
        }
    }
    // this->file = ;
    this->file = root + this->file;
    std::cout << "File : " << this->file << std::endl;
}

void CGI::handle_get()
{
    std::string query;

    if (request->get_uri().find_first_of('?') < request->get_uri().size())
    {
        // std::cout << request->get_uri() << std::endl;
        query = request->get_uri().substr(request->get_uri().find_first_of('?'), request->get_uri().size() - request->get_uri().find_first_of('?'));
    }
    // else
    //     query = 
    this->env.set_var("REQUEST_METHOD","GET");
    this->env.set_var("QUERY_STRING", query);
    this->env.set_var("CONTENT_LENGTH", to_string(query.size()));
    this->env.set_var("SCRIPT_FILENAME", this->file);
    this->env.set_var("PATH_INFO", this->path);
}

void CGI::handle_post(int fd)
{
    this->env.set_var("REQUEST_METHOD", "POST");
    this->env.set_var("CONTENT_LENGTH", to_string(request->get_body().size()));
    write(fd, request->get_body().c_str(), request->get_body().size());
    this->env.set_var("SCRIPT_FILENAME", this->file);
    this->env.set_var("PATH_INFO", this->path);
}

void CGI::cgi_handler()
{
    pid_t   pid;
    int     saveIn;
    int     saveOut;
    char **av;
    int fd[2];
    std::string newBody;

    saveIn = dup(STDIN_FILENO);
	saveOut = dup(STDOUT_FILENO);
    FILE	*fIn = tmpfile();
	FILE	*fOut = tmpfile();
	long	fdIn = fileno(fIn);
	long	fdOut = fileno(fOut);
	int		ret = 1;
	lseek(fdIn, 0, SEEK_SET);

    pipe(fd);    
    pid = fork();
    av = (char **)malloc(sizeof(char *) * 5);
    av[0] = (char *)this->path.c_str();
    av[1] = NULL;
    if (!pid)
    {
        if (request->get_method() == "GET")
            handle_get();
        else if (request->get_method() == "POST")
            handle_post(0);
        // dup2(fd[1], 1);
        // close(fd[0]);

        // execve(av[0], av, argv);
        // exit(1);
        char ** argv = this->env.to_matrix();
		dup2(fdIn, STDIN_FILENO);
		dup2(fdOut, STDOUT_FILENO);
		execve(av[0], av, argv);
		std::cerr << "Execve crashed." << std::endl;
		write(STDOUT_FILENO, "Status: 500\r\n\r\n", 15);
    }
    else 
    {
        //     close(fd[1]);
        //     waitpid(pid, 0, 0);

        //     std::cout << "CGI RESPONSE\n";
        //     std::cout << "<<<<<<<<<>>>>>>>>>\n";
        //     char *buf;

        //     buf = (char *)malloc(10000);
        //     memset(buf, 0, 10000);
        //     read(fd[0], buf, 10000);

        // }
    	char	buffer[65536] = {0};

		waitpid(-1, NULL, 0);
		lseek(fdOut, 0, SEEK_SET);

		ret = 1;
		while (ret > 0)
		{
			memset(buffer, 0, 65536);
			ret = read(fdOut, buffer, 65535);
			this->reponse += buffer;
		}
        dup2(saveIn, STDIN_FILENO);
	    dup2(saveOut, STDOUT_FILENO);
	    std::fclose(fIn);
	    std::fclose(fOut);
	    close(fdIn);
	    close(fdOut);
	    close(saveIn);
	    close(saveOut);
    }    

}




CGI::~CGI() {}