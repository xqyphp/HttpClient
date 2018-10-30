#pragma once
#include "http_parser.h"

#include <map>
#include <string>

class HttpParser :
public http_parser
{
public:
	HttpParser(enum http_parser_type type);
	~HttpParser();


	int Execute(const char * buffer, size_t count);

    //uint64_t  GetContentLen(){return mParser.content_length;}
    //无法判断非指定长度，在HttpClient中判断
	bool IsReadComplete();
	int GetErrorCode();
	std::string& GetResponseBody();
	bool  GetHeader(std::string name, std::string& value);
	void Reset();

	int handleMessageBegin();
	int handleUrl(const char * at, size_t length);
	int handleStatus(const char * at, size_t length);
	int handleHeaderField(const char * at, size_t length);
	int handleHeaderValue(const char * at, size_t length);
	int handleHeadersComplete();
	int handleBody(const char * at, size_t length);
	int handleMessageComplete();
	int handleChunkHeader();
	int handleChunkComplete();

	static int onMessageBegin(struct http_parser*);
	static int onUrl(struct http_parser*, const char*, size_t);
	static int onStatus(struct http_parser*, const char*, size_t);
	static int onHeaderField(struct http_parser*, const char*, size_t);
	static int onHeaderValue(struct http_parser*, const char*, size_t);
	static int onHeadersComplete(struct http_parser*);
	static int onBody(struct http_parser*, const char*, size_t);
	static int onMessageComplete(struct http_parser*);
	static int onChunkHeader(struct http_parser*);
	static int onChunkComplete(struct http_parser*);

private:

	typedef enum { NOTHING, FIELD, VALUE } last_on_header_t;

   	//http_parser mParser;
	http_parser_settings mSettings;

	std::map<std::string, std::string> mHeaders;
	std::string mBody;
	int mErrorCode;
	bool mHeadersComplete;
	bool mMessageComplete;

	std::string header_value;
	std::string header_field;
	last_on_header_t last_on_header;

	enum http_parser_type mType;
};

