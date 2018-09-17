#include "HttpParser.h"
#include "http_parser.h"
#include <algorithm>
#include <iostream>

using namespace std;
HttpParser::HttpParser(enum http_parser_type type)
{
	mSettings.on_message_begin = &HttpParser::onMessageBegin;
	mSettings.on_url = &HttpParser::onUrl;
	mSettings.on_status = &HttpParser::onStatus;
	mSettings.on_header_field = &HttpParser::onHeaderField;
	mSettings.on_header_value = &HttpParser::onHeaderValue;
	mSettings.on_headers_complete = &HttpParser::onHeadersComplete;
	mSettings.on_body = &HttpParser::onBody;
	mSettings.on_message_complete = &HttpParser::onMessageComplete;
	mSettings.on_chunk_header = &HttpParser::onChunkHeader;
	mSettings.on_chunk_complete = &HttpParser::onChunkComplete;

	http_parser_init(&mParser, type);
	mParser.data = this;

	Reset();
}

HttpParser::~HttpParser()
{
	//handleMessageComplete();
}


int HttpParser::handleMessageBegin()
{
	return 0;
}

int HttpParser::handleUrl(const char * at, size_t length)
{
	return 0;
}

int HttpParser::handleStatus(const char * at, size_t length)
{
	return 0;
}

int HttpParser::handleHeaderField(const char* at, size_t length)
{
	std::string field(at, length);
	std::transform(field.begin(), field.end(), field.begin(), tolower);

	switch (last_on_header) {
	case NOTHING:
		// Allocate new buffer and copy callback data into it
		header_field = field;
		break;
	case VALUE:
		// New header started.
		// Copy current name,value buffers to headers
		// list and allocate new buffer for new name
		mHeanders[header_field] = header_value;
		header_field = field;
		break;
	case FIELD:
		// Previous name continues. Reallocate name
		// buffer and append callback data to it
		header_field.append(field);
		break;
	}
	last_on_header = FIELD;
	return 0;
}

int HttpParser::handleHeaderValue(const char* at, size_t length)
{
	const std::string value(at, length);
	switch (last_on_header) {
	case FIELD:
		//Value for current header started. Allocate
		//new buffer and copy callback data to it
		header_value = value;
		break;
	case VALUE:
		//Value continues. Reallocate value buffer
		//and append callback data to it
		header_value.append(value);
		break;
	case NOTHING:
		// this shouldn't happen
		//DEBUG(10)("Internal error in http-parser");
		break;
	}
	last_on_header = VALUE;

	return 0;
}

int HttpParser::handleHeadersComplete()
{
	/* Add the most recently read header to the map, if any */
	if (last_on_header == VALUE) {
		mHeanders[header_field] = header_value;
		header_field = "";
		header_value = "";
	}

	mHeadersComplete = true;

	return 0;
}

int HttpParser::handleBody(const char * at, size_t length)
{
	mBody.append(at, length); 
	return 0;
}


int HttpParser::handleMessageComplete()
{
	mMessageComplete = true;
	return 0;
}

int HttpParser::handleChunkHeader()
{
	return 0;
}

int HttpParser::handleChunkComplete()
{
	return 0;
}

int HttpParser::Execute(const char* buffer, size_t count)
{
	if (mMessageComplete) {
		Reset();
	}
	int n = http_parser_execute(&mParser, &mSettings, buffer, count);

	if (mParser.upgrade) //TODO
	{
		return 1000;
	}
	else if (n != count)
	{
		mErrorCode = (mParser.http_errno != 0 ? mParser.http_errno : 400);
		return mErrorCode;
	}
	return 0;
}

bool HttpParser::IsHeadersComplete()
{
	return mHeadersComplete;
}

bool HttpParser::IsReadComplete()
{
	return mMessageComplete;
}

int HttpParser::GetErrorCode()
{
	return mErrorCode;
}

std::string & HttpParser::GetResponseBody()
{
	return mBody;
}

bool HttpParser::GetHeader(std::string name, std::string & value)
{
	map<string,string>::iterator it =  mHeanders.find(name);
	if (it == mHeanders.end()) {
		return false;
	}

	value =  it->second;
	return true;
}

void HttpParser::Reset()
{
	mHeanders.clear();
	mBody.clear();
	header_field = "";
	header_value = "";
	last_on_header = NOTHING;
	mHeadersComplete = false;
	mMessageComplete = false;
}

int HttpParser::onMessageBegin(http_parser * parser)
{
	HttpParser* p = (HttpParser*)parser->data;
	return p->handleMessageBegin();
}

int HttpParser::onUrl(http_parser * parser, const char * at, size_t len)
{
	HttpParser* p = (HttpParser*)parser->data;
	return p->handleUrl(at,len);
}

int HttpParser::onStatus(http_parser *parser, const char * at, size_t len)
{
	HttpParser* p = (HttpParser*)parser->data;
	return p->handleStatus(at, len);
}

int HttpParser::onHeaderField(http_parser *parser, const char * at, size_t len)
{
	HttpParser* p = (HttpParser*)parser->data;
	return p->handleHeaderField(at, len);
}

int HttpParser::onHeaderValue(http_parser *parser, const char * at, size_t len)
{
	HttpParser* p = (HttpParser*)parser->data;
	return p->handleHeaderValue(at, len);
}

int HttpParser::onHeadersComplete(http_parser *parser)
{
	HttpParser* p = (HttpParser*)parser->data;
	return p->handleHeadersComplete();
}

int HttpParser::onBody(http_parser *parser, const char * at, size_t len)
{
	HttpParser* p = (HttpParser*)parser->data;
	return p->handleBody(at, len);
}

int HttpParser::onMessageComplete(http_parser * parser)
{
	HttpParser* p = (HttpParser*)parser->data;
	return p->handleMessageComplete();
}

int HttpParser::onChunkHeader(http_parser *parser)
{
	HttpParser* p = (HttpParser*)parser->data;
	return p->handleChunkHeader();
}

int HttpParser::onChunkComplete(http_parser *parser)
{
	HttpParser* p = (HttpParser*)parser->data;
	return p->handleChunkComplete();
}
