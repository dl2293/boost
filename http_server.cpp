#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <signal.h>
#include <utility>
#include <set>
#include <sys/wait.h> //waitpid()
#include <sys/types.h> //pid_t
#include <vector>


using namespace std;
using namespace boost::asio;

const string httpok = "HTTP/1.1 200 OK";

io_service global_io_service;

string GetEnv(string name)
{
    char* locat;
	locat = getenv(name.c_str());
	if(locat != NULL)
	{
		return locat;
	}
    return "";
}

void Setenv(string name, string locat)
{
    setenv(name.c_str(),locat.c_str(),1);
}

vector<string> SplitStr(string line, string findstr)
{
    int start = 0;
    vector<string> cmd; //command in line
    size_t found = line.find(findstr);
    if(found == string::npos)
	{
		cmd.push_back(line);
	}
	else
 	{
		while(1)
		{
			string cmd_content;
			if(found != string::npos)
			{
				cmd_content = line.substr(start , found - start);
				cmd.push_back(cmd_content);
				start = found + 1;
				found=line.find(findstr, start);
				if(cmd.back() == "")
				{
					cmd.pop_back();
				}
			}
			else
			{
				cmd_content = line.substr(start, line.length() - start);
				cmd.push_back(cmd_content);
				if(cmd.back() == "")
				{
					cmd.pop_back();
				}
				break;
			}
		}
	}
    return cmd;
}

void ChildHandler(int signo){
    int status;
    while(waitpid(-1,&status,WNOHANG)>0){}
}

class Session : public enable_shared_from_this<Session>
{
    private:
        enum { max_length = 1024 };
        ip::tcp::socket _socket;
        array<char, max_length> _data;
        string msg;
        string cgi;
    public:
            Session(ip::tcp::socket socket) : _socket(move(socket)) {}
            void start()
            {
                do_read();
            }
    private:
        void do_read()
        {
            auto self(shared_from_this());
            _socket.async_read_some(buffer(_data, max_length),
                [this, self](boost::system::error_code ec, size_t length)
                {
                    if (!ec)
                    {
                        msg = "";
                        for(int i = 0; i < length; i++)
                        {
                            msg += _data[i];
                        }
                        cout<<msg<<endl;
                        do_write();
                    }
                });
        }

        void do_write() 
        {
            auto self(shared_from_this());
            vector <string> URL = SplitStr(msg,"\r\n");
            vector <string> req = SplitStr(URL[0]," ");//切出REQUEST_METHOD,REQUEST_URI,SERVER_PROTOCOL
            vector <string> QS = SplitStr(req[1],"?"); //切出QUERY_STRING
            vector <string> HH = SplitStr(URL[1],":");//切出HTTP_HOST
            Setenv("REQUEST_METHOD",req[0]);
            Setenv("REQUEST_URI",req[1]);
            cgi=QS[0].substr(1);
            Setenv("QUERY_STRING",QS[1]);
            Setenv("SERVER_PROTOCOL",req[2]);
            string Host = HH[1].substr(1);
            Setenv("HTTP_HOST",Host);//前面會有一個空白
            boost::system::error_code ec;
            ip::tcp::endpoint Lendpoint = _socket.local_endpoint(ec);
            if(!ec)
            {
                    Setenv("SERVER_ADDR",Lendpoint.address().to_string());
                    Setenv("SERVER_PORT",to_string(Lendpoint.port()));
            }
            else
            {
                    cout<<"local: "<<ec.message()<<endl;
            }
            ip::tcp::endpoint Rendpoint = _socket.remote_endpoint(ec);
            if(!ec)
            {
                Setenv("REMOTE_ADDR",Rendpoint.address().to_string());
                Setenv("REMOTE_PORT",to_string(Rendpoint.port()));
            }
            else
            {
                cout<<"remote: "<<ec.message()<<endl;
            }
            _socket.async_send(buffer(httpok.c_str(), httpok.length()),
                [this, self](boost::system::error_code ec, size_t /* length */) 
            {
                if (!ec) 
                {
                    if(fork() == 0)
                    {
                        char** arg = new char* [2];
                        cout<<"=="<<cgi<<"=="<<endl;
                        arg[0]=strdup(cgi.c_str());
                        arg[1]=NULL;
                        dup2(_socket.native_handle(),0);
                        dup2(_socket.native_handle(),1);
                        dup2(_socket.native_handle(),2);
                        if(execv(arg[0],arg)<0){
                            cerr<<strerror(errno)<<endl;
                            exit(EXIT_FAILURE);
                        }
                    }
                    _socket.close();
                }
            });
        }
};

class Server 
{
    private:
        ip::tcp::acceptor _acceptor;
        ip::tcp::socket _socket;

    public:
        Server(short port)
            : _acceptor(global_io_service, ip::tcp::endpoint(ip::tcp::v4(), port)),
                _socket(global_io_service) 
                {
                    do_accept();
                }

    private:
        void do_accept() 
        {
            _acceptor.async_accept(_socket, [this](boost::system::error_code ec) 
            {
                if (!ec) 
                {
                    make_shared<Session>(move(_socket))->start();
                }
                do_accept();
            });
        }
};

int main(int argc, char* const argv[]) 
{
    if (argc != 2) 
    {
        cerr << "Usage:" << argv[0] << " [port]" << endl;
        return 1;
    }
    try 
    {
        unsigned short port = atoi(argv[1]);
        Server server(port);
        global_io_service.run();
    } 
    catch (exception& e) 
    {
        cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}