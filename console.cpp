#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <map>
#include <vector>
#include <utility>
#include <fstream>
#include <cstdlib>
#include <ctime> 
using namespace std;
using namespace boost::asio;

map<int,map<string, string>> info;

io_context ip_context;

/*void Delay(float time) 
{
    clock_t now = clock(); 
    while(clock() - now < time); 
} */


string escape(string data){
    string session = "" ;
    for ( auto&& ch : data) session += ( "&#" + to_string(int(ch)) + ";" );
    return session;
}

void OutputShell(string session , string c){
    string content=escape(c);
    cout<<"<script>document.getElementById('"<<session<<"').innerHTML += '"<<content<<"';</script>"<<endl;
}

void OutputCommand(string session , string c){
    string content=escape(c);
    cout<<"<script>document.getElementById('"<<session<<"').innerHTML += '<b>"<<content<<"</b>';</script>"<<endl;    
}

void CGIinterface()
{
    cout<<"Content-type: text/html"<<"\r\n\r\n";
    cout<<"<!DOCTYPE html>"<<endl;
    cout<<"<html lang=\"en\">"<<endl;
    cout<<"<head>"<<endl;
    cout<<"<meta charset=\"UTF-8\" />"<<endl;
    cout<<"<title>NP Project 3 Console</title>"<<endl;
    cout<<"<link"<<endl;
    cout<<"rel=\"stylesheet\""<<endl;
    cout<<"href=\"https://stackpath.bootstrapcdn.com/bootstrap/4.1.3/css/bootstrap.min.css\""<<endl;
    cout<<"integrity=\"sha384-MCw98/SFnGE8fJT3GXwEOngsV7Zt27NXFoaoApmYm81iuXoPkFOJwJ8ERdknLPMO\""<<endl;
    cout<<"crossorigin=\"anonymous\""<<endl;
    cout<<"/>"<<endl;
    cout<<"<link"<<endl;
    cout<<"href=\"https://fonts.googleapis.com/css?family=Source+Code+Pro\""<<endl;
    cout<<"rel=\"stylesheet\""<<endl;
    cout<<"/>"<<endl;
    cout<<"<link"<<endl;
    cout<<"rel=\"icon\""<<endl;
    cout<<"type=\"image/png\""<<endl;
    cout<<"href=\"https://cdn0.iconfinder.com/data/icons/small-n-flat/24/678068-terminal-512.png\""<<endl;
    cout<<"/>"<<endl;
    cout<<"<style>"<<endl;
    cout<<"* {"<<endl;
    cout<<"font-family: 'Source Code Pro', monospace;"<<endl;
    cout<<"font-size: 1rem !important;"<<endl;
    cout<<"}"<<endl;
    cout<<"body {"<<endl;
    cout<<"background-color: #212529;"<<endl;
    cout<<"}"<<endl;
    cout<<"pre {"<<endl;
    cout<<"color: #cccccc;"<<endl;
    cout<<"}"<<endl;
    cout<<"b {"<<endl;
    cout<<"color: #ffffff;"<<endl;
    cout<<"}"<<endl;
    cout<<"</style>"<<endl;
    cout<<"</head>"<<endl;
    cout<<"<body>"<<endl;
    cout<<"<table class=\"table table-dark table-bordered\">"<<endl;
    cout<<"<thead>"<<endl;
    cout<<"<tr>"<<endl;
    cout<<"<th scope=\"col\">"<<info[0]["IP"]<<":"<<info[0]["PORT"]<<"</th>"<<endl;
    cout<<"<th scope=\"col\">"<<info[1]["IP"]<<":"<<info[1]["PORT"]<<"</th>"<<endl;
    cout<<"<th scope=\"col\">"<<info[2]["IP"]<<":"<<info[2]["PORT"]<<"</th>"<<endl;
    cout<<"<th scope=\"col\">"<<info[3]["IP"]<<":"<<info[3]["PORT"]<<"</th>"<<endl;
    cout<<"<th scope=\"col\">"<<info[4]["IP"]<<":"<<info[4]["PORT"]<<"</th>"<<endl;
    cout<<"</tr>"<<endl;
    cout<<"</thead>"<<endl;
    cout<<"<tbody>"<<endl;
    cout<<"<tr>"<<endl;
    cout<<"<td><pre id=\"s0\" class=\"mb-0\"></pre></td>"<<endl;
    cout<<"<td><pre id=\"s1\" class=\"mb-0\"></pre></td>"<<endl;
    cout<<"<td><pre id=\"s2\" class=\"mb-0\"></pre></td>"<<endl;
    cout<<"<td><pre id=\"s3\" class=\"mb-0\"></pre></td>"<<endl;
    cout<<"<td><pre id=\"s4\" class=\"mb-0\"></pre></td>"<<endl;
    cout<<"</tr>"<<endl;
    cout<<"</tbody>"<<endl;
    cout<<"</table>"<<endl;
    cout<<"</body>"<<endl;
    cout<<"</html>"<<endl;
}

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

void GetInfo()
{
    string URL = GetEnv("QUERY_STRING");
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

class Session : public enable_shared_from_this<Session>
{
    private:
        enum { max_length = 1024 };
        ip::tcp::socket _socket;
        array<char, max_length> _data;
        ip::tcp::resolver _resolver;
        int CID;
        string msg;
        string session;
        ifstream testfile;
    public:
        Session(int id) : 
            _socket(ip_context), _resolver(ip_context),CID(id), session("s"+to_string(id)),testfile(info[id]["FILE"])
        {
        }
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
                        OutputShell(session,msg);
                        if(msg.find("%") != string::npos)
                        {
                            do_write();
                        }
                        do_read();
                    }
                });
        }

    void do_write()
    {
        auto self(shared_from_this());
        string line = "";
        getline(testfile, line);
        if(line=="exit")
        {
            testfile.close();
        }
        line += '\n';
        OutputCommand(session,line);
        async_write(_socket, buffer(line.c_str(), line.length()),
            [this, self](boost::system::error_code ec, size_t /*length*/)
            {
                if (!ec)
                {
                    do_read();
                }
            });
    }
};

int main()
{
    GetInfo();
    CGIinterface();
    try
    {
        for(int i = 0; i < 5; i ++)
        {
            make_shared<Session>(i)->start();
        }
        ip_context.run();
    }
    catch(exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    
}