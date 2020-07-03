#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <map>
#include <vector>
#include <fstream>
#include <iostream>
#include <signal.h>
#include <utility>
#include <set>

using namespace std;
using namespace boost::asio;

io_context ip_context;
io_service global_io_service;

map<int,map<string, string>> info;
const int N_SERVERS = 5;
const string FormMethod = "GET";
const string FormAction = "console.cgi";
const string TestCaseDirectory = "test_case";
const string Domain = "cs.nctu.edu.tw";
typedef boost::shared_ptr<ip::tcp::socket> socket_ptr;

string escape(string data){
    string session = "" ;
    for ( auto&& ch : data) session += ( "&#" + to_string(int(ch)) + ";" );
    return session;
}

string OutputShell(string session , string c){
    string content=escape(c);
    return "<script>document.getElementById('" + session + "').innerHTML += '" + content + "';</script>\n";
}

string OutputCommand(string session , string c){
    string content=escape(c);
    return "<script>document.getElementById('" + session + "').innerHTML += '<b>" + content + "</b>';</script>\n";    
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

void GetInfo(string URL)
{
    vector<string> informat;
    informat = SplitStr(URL, "&");
    for(int i = 0; i < 15; i++)
    {
        vector<string> msg;
        msg = SplitStr(informat[i], "=");
        if(i % 3 == 0)
        {
            if(msg.size() == 2)
            {
                info[(i/3)]["IP"] = msg[1];
            }
            else
            {
                info[(i/3)]["IP"] = "";
            }
        }
        else if(i % 3 == 1)
        {
            if(msg.size() == 2)
            {
                info[(i/3)]["PORT"] = msg[1];
            }
            else
            {
                info[(i/3)]["PORT"] = "";
            }
        }
        else
        {
            if(msg.size() == 2)
            {
                info[(i/3)]["FILE"] = "test_case/" + msg[1];
            }
            else
            {
                info[(i/3)]["FILE"] = "";
            }
        }
    }
}
string ShowPanel()
{
    string panel = "";
	string test_case_menu = "";
	string host_menu = "";

	for (int i = 1; i < 11; i++) 
    {
		test_case_menu += ("<option value=t" + to_string(i) + ".txt>t" + to_string(i) + ".txt</option>");
	}

	for (int i = 1; i < 11; i++) 
    {
		host_menu += ("<option value=nplinux" + to_string(i) + "." + Domain + ">nplinux" + to_string(i) + "</option>");
	}
	panel += "HTTP / 1.1 200 OK\n";

	panel += "Content-type: text/html\r\n\r\n";
    panel += "<!DOCTYPE html>\n";
    panel += "<html lang=\"en\">\n";
    panel += "<head>\n";
    panel += "<title>NP Project 3 Panel</title>\n";
    panel += "<link\n";
    panel += "rel=\"stylesheet\"\n";
    panel += "href=\"https://stackpath.bootstrapcdn.com/bootstrap/4.1.3/css/bootstrap.min.css\"\n";
    panel += "integrity=\"sha384-MCw98/SFnGE8fJT3GXwEOngsV7Zt27NXFoaoApmYm81iuXoPkFOJwJ8ERdknLPMO\"\n";
    panel += "crossorigin=\"anonymous\"\n";
    panel += "/>\n";
    panel += "<link\n";
    panel += "href=\"https://fonts.googleapis.com/css?family=Source+Code+Pro\"\n";
    panel += "rel=\"stylesheet\"\n";
    panel += "/>\n";
    panel += "<link\n";
    panel += "rel=\"icon\"\n";
    panel += "type=\"image/png\"\n";
    panel += "href=\"https://cdn4.iconfinder.com/data/icons/iconsimple-setting-time/512/dashboard-512.png\"\n";
    panel += "/>\n";
    panel += "<style>\n";
    panel += "* {\n";
    panel += "font-family: 'Source Code Pro', monospace;\n";
    panel += "}\n";
    panel += "</style>\n";
    panel += "</head>\n";
    panel += "<body class=\"bg-secondary pt-5\">\n";
    panel += "<form action=\"" + FormAction +"\" method=\"" + FormMethod + "\">\n";
    panel += "<table class=\"table mx-auto bg-light\" style=\"width: inherit\">\n";
    panel += "<thead class=\"thead-dark\">\n";
    panel += "<tr>\n";
    panel += "<th scope=\"col\">#</th>\n";
    panel += "<th scope=\"col\">Host</th>\n";
    panel += "<th scope=\"col\">Port</th>\n";
    panel += "<th scope=\"col\">Input File</th>\n";
    panel += "</tr>\n";
    panel += "</thead>\n";
    panel += "<tbody>\n";
    for(int i = 0; i < N_SERVERS; i++)
    {
        panel += "<tr>\n";
        panel += "<th scope=\"row\" class=\"align-middle\">Session " + to_string(i + 1) + "</th>\n";
        panel += "<td>\n";
        panel += "<div class=\"input-group\">\n";
        panel += "<select name=\"h" + to_string(i) + "\" class=\"custom-select\">\n";
        panel += "<option></option>" + host_menu + "\n";
        panel += "</select>\n";
        panel += "<div class=\"input-group-append\">\n";
        panel += "<span class=\"input-group-text\">.cs.nctu.edu.tw</span>\n";
        panel += "</div>\n";
        panel += "</div>\n";
        panel += "</td>\n";
        panel += "<td>\n";
        panel += "<input name=\"p" + to_string(i) + "\" type=\"text\" class=\"form-control\" size=\"5\" />\n";
        panel += "</td>\n";
        panel += "<td>\n";
        panel += "<select name=\"f" + to_string(i) + "\" class=\"custom-select\">\n";
        panel += "<option></option>\n";
        panel += "" + test_case_menu + "\n";
        panel += "</select>\n";
        panel += "</td>\n";
        panel += "</tr>\n";
    }
    panel += "<tr>\n";
    panel += "<td colspan=\"3\"></td>\n";
    panel += "<td>\n";
    panel += "<button type=\"submit\" class=\"btn btn-info btn-block\">Run</button>\n";
    panel += "</td>\n";
    panel += "</tr>\n";
    panel += "</tbody>\n";
    panel += "</table>\n";
    panel += "</form>\n";
    panel += "</body>\n";
    panel += "</html>\n";

    return panel;
}

string ShowConsole()
{
    string console = "";
    console += "HTTP / 1.1 200 OK\n";
    console +=  "Content-type: text/html\r\n\r\n";
    console +=  "<!DOCTYPE html>\n";
    console +=  "<html lang=\"en\">\n";
    console +=  "<head>\n";
    console +=  "<meta charset=\"UTF-8\" />\n";
    console +=  "<title>NP Project 3 Console</title>\n";
    console +=  "<link\n";
    console +=  "rel=\"stylesheet\"\n";
    console +=  "href=\"https://stackpath.bootstrapcdn.com/bootstrap/4.1.3/css/bootstrap.min.css\"\n";
    console +=  "integrity=\"sha384-MCw98/SFnGE8fJT3GXwEOngsV7Zt27NXFoaoApmYm81iuXoPkFOJwJ8ERdknLPMO\"\n";
    console +=  "crossorigin=\"anonymous\"\n";
    console +=  "/>\n";
    console +=  "<link\n";
    console +=  "href=\"https://fonts.googleapis.com/css?family=Source+Code+Pro\"\n";
    console +=  "rel=\"stylesheet\"\n";
    console +=  "/>\n";
    console +=  "<link\n";
    console +=  "rel=\"icon\"\n";
    console +=  "type=\"image/png\"\n";
    console +=  "href=\"https://cdn0.iconfinder.com/data/icons/small-n-flat/24/678068-terminal-512.png\"\n";
    console +=  "/>\n";
    console +=  "<style>\n";
    console +=  "* {\n";
    console +=  "font-family: 'Source Code Pro', monospace;\n";
    console +=  "font-size: 1rem !important;\n";
    console +=  "}\n";
    console +=  "body {\n";
    console +=  "background-color: #212529;\n";
    console +=  "}\n";
    console +=  "pre {\n";
    console +=  "color: #cccccc;\n";
    console +=  "}\n";
    console +=  "b {\n";
    console +=  "color: #ffffff;\n";
    console +=  "}\n";
    console +=  "</style>\n";
    console +=  "</head>\n";
    console +=  "<body>\n";
    console +=  "<table class=\"table table-dark table-bordered\">\n";
    console +=  "<thead>\n";
    console +=  "<tr>\n";
    console +=  "<th scope=\"col\">" + info[0]["IP"] + ":" + info[0]["PORT"] + "</th>\n";
    console +=  "<th scope=\"col\">" + info[1]["IP"] + ":" + info[1]["PORT"] + "</th>\n";
    console +=  "<th scope=\"col\">" + info[2]["IP"] + ":" + info[2]["PORT"] + "</th>\n";
    console +=  "<th scope=\"col\">" + info[3]["IP"] + ":" + info[3]["PORT"] + "</th>\n";
    console +=  "<th scope=\"col\">" + info[4]["IP"] + ":" + info[4]["PORT"] + "</th>\n";
    console +=  "</tr>\n";
    console +=  "</thead>\n";
    console +=  "<tbody>\n";
    console +=  "<tr>\n";
    console +=  "<td><pre id=\"s0\" class=\"mb-0\"></pre></td>\n";
    console +=  "<td><pre id=\"s1\" class=\"mb-0\"></pre></td>\n";
    console +=  "<td><pre id=\"s2\" class=\"mb-0\"></pre></td>\n";
    console +=  "<td><pre id=\"s3\" class=\"mb-0\"></pre></td>\n";
    console +=  "<td><pre id=\"s4\" class=\"mb-0\"></pre></td>\n";
    console +=  "</tr>\n";
    console +=  "</tbody>\n";
    console +=  "</table>\n";
    console +=  "</body>\n";
    console +=  "</html>\n";
    
    return console;
}

class CGISession : public enable_shared_from_this<CGISession>
{
    private:
        enum { max_length = 1024 };
        ip::tcp::socket _socket;
        array<char, max_length> _data;
        ip::tcp::resolver _resolver;
        int CID;
        string line;
        string msg;
        string session;
        ifstream testfile;
        socket_ptr ServerSocket;
    public:
        CGISession(int id, socket_ptr socket) : 
            _socket(ip_context), _resolver(ip_context), ServerSocket(socket), CID(id), session("s" + to_string(id)), testfile(info[id]["FILE"]){}

        void start()
        {
            do_resolve();
        }
    private:
        void do_resolve()
        {
            auto self(shared_from_this());
            _resolver.async_resolve( ip::tcp::resolver::query(info[CID]["IP"],info[CID]["PORT"]),
                [this,self](boost::system::error_code ec,ip::tcp::resolver::iterator iterator) {
                    if (!ec) {
                        do_connect(iterator);
                    }
                }
            );
        }

        void do_connect(ip::tcp::resolver::iterator iterator) {
            auto self(shared_from_this());
            _socket.async_connect( *iterator,[this,self](boost::system::error_code ec) {
                if (!ec) {
                    do_read();
                }
            });
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
                        for(int i = 0; i < int(length); i++)
                        {
                            msg += _data[i];
                        }
                        string output = OutputShell(session,msg);
                        ServerSocket->async_write_some(
                            buffer(output.c_str(), output.length()),
                            [this, self](boost::system::error_code ec, std::size_t length) {}
                        );
                        if(msg.find("%") != string::npos)
                        {
                            do_write();
                        }
                        do_read();
                    }
                    else
                    {
                        msg = "";
                        for(int i = 0; i < int(length); i++)
                        {
                            msg += _data[i];
                        }
                        cout<<msg<<endl;
                    }
                    
                });
        }

    void do_write()
    {
        auto self(shared_from_this());
        line = "";
        getline(testfile, line);
        if(line=="exit")
        {
            testfile.close();
        }
        line += '\n';
        async_write(_socket,
			buffer(line.c_str(), line.length()),
			[this, self](boost::system::error_code ec, size_t length) 
            {
				if (!ec) 
                {
					string getcmd = OutputCommand(session, line);
					ServerSocket->async_write_some(
						buffer(getcmd.c_str(), getcmd.length()),
						[this, self](boost::system::error_code ec, size_t length) 
                        {
							if (!ec) 
                            {
								do_read();
							}
						}
					);
				}
			}
		);
    }
};

class ServerSession : public enable_shared_from_this<ServerSession>
{
    private:
        enum { max_length = 1024 };
        ip::tcp::socket _socket;
        array<char, max_length> _data;
        string msg;
    public:
            ServerSession(ip::tcp::socket socket) : _socket(move(socket)) {}
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
                        for(int i = 0; i < int(length); i++)
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
            if(msg.find("GET /panel.cgi") != string::npos)
            {
                string panel = ShowPanel();
			    _socket.async_send(buffer(panel.c_str(), panel.length()),
				[this, self](boost::system::error_code ec, size_t /* length */) 
                {
				    if (!ec) 
                    {
					    do_read();
				    }
			    });
            }
            else if(msg.find("GET /console.cgi") != string::npos)
            {
                vector <string> URL = SplitStr(msg,"\r\n");
                vector <string> req = SplitStr(URL[0]," ");//切出REQUEST_METHOD,REQUEST_URI,SERVER_PROTOCOL
                vector <string> QS = SplitStr(req[1],"?"); //切出QUERY_STRING
                GetInfo(QS[1]);
                string console = ShowConsole();
                _socket.async_send(buffer(console.c_str(), console.length()),
                    [this, self](boost::system::error_code ec, size_t /* length */) 
                {
                    if (!ec) 
                    {
                        socket_ptr sock(&_socket);
                        for(int i = 0; i < 5; i ++)
                        {
                            make_shared<CGISession>(i, sock)->start();
                        }
                        ip_context.run();
                        do_read();
                    }
                });
            }
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
                    make_shared<ServerSession>(move(_socket))->start();
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
        system("pause");
        return 1;
    }
    try 
    {
        short port = atoi(argv[1]);
        Server server(port);
        global_io_service.run();
    } 
    catch (exception& e) 
    {
        cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}