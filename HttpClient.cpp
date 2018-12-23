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
	const char *BLANK_SPACE = " ";
	const char *NEW_LINE = "\r\n";
	//请求行
	std::string requestContent;
	requestContent.append(method).append(BLANK_SPACE);
	if (path.size() == 0 || path[0] != '/') {
		requestContent.append("/");
	}
	requestContent.append(path);
	requestContent.append(BLANK_SPACE).append("HTTP/1.1").append(NEW_LINE);
	//请求头
	for (auto it = mConstHeaders.begin(); it != mConstHeaders.end(); it++) {
		if (headers.find(it->first) == headers.end()) {
			requestContent.append(it->first).append(":").append(it->second).append(NEW_LINE);
		}
	}
	for (auto it = headers.begin(); it != headers.end(); it++) {
		if (headers.find(it->first) == headers.end()) {
			requestContent.append(it->first).append(":").append(it->second).append(NEW_LINE);
		}
	}
	//请求体长度
	if (body.size() != 0) {
		char c[64];
		int length = snprintf(c, sizeof(c), "Content - Length: %d\r\n", (int)body.size());
		requestContent.append(c);
	}
	//回车换行
	requestContent.append(NEW_LINE);
	//请求体
	requestContent.append(body);
	//发送请求
	if (!Send(requestContent)) {
		return false;
	}
	//读取结果
	return ReadResponse();
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







