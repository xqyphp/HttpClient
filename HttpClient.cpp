#include "HttpClient.h"
#include <iostream>
using namespace std;

HttpClient::HttpClient(const std::string & host, int port)
	:TcpClient(host, port), mHttpParser(HTTP_RESPONSE)
{
	mConstHeaders["Accept"] = " */*";
	mConstHeaders["Accept-Language"] = " zh-cn";
	mConstHeaders["Host:"] = mHost;
}

HttpClient::~HttpClient()
{
	TcpClient::~TcpClient();
}

bool HttpClient::SendRequest(const std::string & method, const std::string & path, std::map<std::string, std::string> headers, const std::string body)
{
	std::string requestLine;

	requestLine += method + " " + (path.size() == 0 ? "/" : path) + " HTTP/1.1\r\n";

	std::string headerStr;



	for (auto it = mConstHeaders.cbegin(); it != mConstHeaders.cend(); it++) {
		if (headers.find(it->first) == headers.cend()) {
			headerStr += it->first + ":" + it->second + "\r\n";
		}
	}

	for (auto it = headers.cbegin(); it != headers.cend(); it++) {
		headerStr += it->first + ":" + it->second + "\r\n";
	}

	if (body.size() != 0) {
		char c[64];
		int length = snprintf(c,sizeof(c), "Content - Length:%d\r\n", body.size());
		headerStr += c;
	}

	headerStr += "\r\n";

	if (!Send(requestLine + headerStr + body)) {
		return false;
	}
	if(!ReadResponse()){
		return false;
	}
	return mHttpParser.status_code  == 200;
}

bool HttpClient::GetHeader(std::string name, std::string & value)
{
	return mHttpParser.GetHeader(name, value);
}



bool HttpClient::GetResponseBody(std::string & body)
{
	body = mHttpParser.GetResponseBody();
	return true;
}

bool HttpClient::ReadResponse()
{
	mHttpParser.Reset();


	std::string tmpBuffer;
    int len;
	while (Read(tmpBuffer,len)) {
		int nCode = mHttpParser.Execute(tmpBuffer.c_str(), tmpBuffer.size());

		if (nCode != 0) {
			return false;
		}

		if (mHttpParser.IsReadComplete()) {
			return true;
		}
	}

    //没有 ContentLen 会返回ULLONG_MAX
    if(mHttpParser.content_length == 0 || mHttpParser.content_length== ULLONG_MAX){
        return true;
    }

	return false;
}

bool HttpClient::SendGet(const std::string & path)
{
	static const std::map<std::string, std::string> myMap;
	return SendRequest("GET", path, myMap, "");
}

bool HttpClient::SendPost(const std::string & path, const std::string& body)
{
	static const std::map<std::string, std::string> myMap;
	return SendRequest("POST", path, myMap, body);
}







