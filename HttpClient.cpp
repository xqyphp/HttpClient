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

	for (map<string, string>::iterator it = mConstHeaders.begin(); it != mConstHeaders.end(); it++) {
		if (headers.find(it->first) == headers.end()) {
			headerStr += it->first + ":" + it->second + "\r\n";
		}
	}

	for (map<string, string>::iterator it = headers.begin(); it != headers.end(); it++) {
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

	return ReadResponse();
}

bool HttpClient::GetHeader(std::string name, std::string & value)
{
	if (!mHttpParser.IsReadComplete()) {
		return false;
	}

	return mHttpParser.GetHeader(name, value);
}



bool HttpClient::GetResponseBody(std::string & body)
{
	if (!mHttpParser.IsReadComplete()) {
		return false;
	}
	body = mHttpParser.GetResponseBody();
	return true;
}

bool HttpClient::ReadResponse()
{
	mHttpParser.Reset();

	std::string tmpBuffer;
	while (Read(tmpBuffer)) {
		int nCode = mHttpParser.Execute(tmpBuffer.c_str(), tmpBuffer.size());
		if (nCode != 0) {
			return false;
		}

		if (mHttpParser.IsReadComplete()) {
			return true;
		}

	}

	return false;
}

bool HttpClient::SendGet(const std::string & path)
{
	static const std::map<std::string, std::string> myMap;
	return SendRequest("GET", path, myMap, "");
}

bool HttpClient::SendPost(const std::string & path, const std::string body)
{
	static const std::map<std::string, std::string> myMap;
	return SendRequest("POST", path, myMap, body);
}





