#pragma once
#include "TcpClient.h"
#include "HttpParser.h"
#include <map>

class HttpClient :
	public TcpClient
{
public:
	HttpClient(const std::string &host, int port);
	~HttpClient();


	bool SendRequest(const std::string& method,const std::string& path,std::map<std::string,std::string> headers,const std::string body);
	bool SendGet(const std::string& path);
	bool SendPost(const std::string& path, const std::string body);
	bool  GetHeader(std::string name, std::string& value);
	bool  GetResponseBody(std::string& body);
protected:
	bool ReadResponse();
private:
	HttpParser mHttpParser;
	std::map<std::string, std::string> mConstHeaders;
};

