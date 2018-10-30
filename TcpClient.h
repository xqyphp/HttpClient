//
// Created by likai_m on 2018/9/13.
//

#ifndef CYAP_ALL_TCPCLIENT_H
#define CYAP_ALL_TCPCLIENT_H

#include <string>
#include <cstddef>
#include "SocketDef.h"


class TcpClient {

public:
	TcpClient(const std::string &host, int port);
	virtual ~TcpClient();

	bool Connect();
	bool DisConnect();

	bool Send(const std::string& text);
	bool Read(std::string& text);
    bool Read(std::string& text,int &code);
	bool ReadUtil(std::string& text, const std::string& util);
	bool ReadAll(std::string& text, size_t len);

	bool IsConnected();
	int  GetErrorNo();
	std::string GetErrorString();
	int  GetTimeOut() { return mTimeOut; }


	void SetReadBlockSize(int blockSize);

	bool SetTimeOut(int timeOut);

protected:
	bool CreateClientSocket();
	bool ConnectToServer();
	bool CloseSocket();
	static bool GetSockAddrByHostName(const std::string& host, int port, struct sockaddr_in& sockIn);
protected:
	std::string mHost;
	int  mPort;
	int  mErrorNo;
	int  mTimeOut;
	size_t  mReadBlockSize;
	int	 mSocketFd;
	bool mbConnected;
	std::string mReadBuffer;
};


#endif //CYAP_ALL_TCPCLIENT_H
