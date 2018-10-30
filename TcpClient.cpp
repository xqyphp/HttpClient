//
// Created by likai_m on 2018/9/13.
//

#include "TcpClient.h"
#include <iostream>

using namespace std;

static int has_letter(const char *str) {
    long len = strlen(str);
    for (long i = 0; i < len; i++) {
        if (str[i] >= 'A' && str[i] <= 'Z')
            return 1;
        if (str[i] >= 'a' && str[i] <= 'z')
            return 1;
    }
    return 0;
}

#if false
static struct hostent *gethostbyname(const char *host) {
    struct hostent hostbuf, *hp;
    size_t hstbuflen;
    char tmphstbuf[1024];
    int res;
    int herr;

    hstbuflen = 1024;

    res = gethostbyname_r(host, &hostbuf, tmphstbuf, hstbuflen,
                          &hp, &herr);
    /*  Check for errors.  */
    if (res || hp == NULL)
        return NULL;
    return hp;
}
#endif // WIN32


static unsigned int get_hostip(const char *hname) {
    unsigned int ip = 0;

    struct hostent *pent = gethostbyname(hname);
    if (NULL == pent || NULL == pent->h_addr || NULL == pent->h_addr_list[0])
        return 0;
    return *(unsigned int *) pent->h_addr_list[0];
}


static const char *inttoip(unsigned int ip_num, char *ipaddr) {
    return inet_ntop(AF_INET, (const void *) &ip_num, ipaddr, (socklen_t) 32);
}

static string Host2IP(string hostname) {
    if (!has_letter(hostname.c_str()))
        return hostname;
    //LOGD("%s",hostname.c_str());
    unsigned int ip_num = get_hostip(hostname.c_str());
    if (ip_num == 0) {
        return hostname;
    }

    char sip[32] = "";
    inttoip(ip_num, sip);
    //LOGD("---%s---",sip);
    return sip;
}

TcpClient::TcpClient(const std::string &host, int port)
        : mPort(port), mErrorNo(0),
          mReadBlockSize(1024), mSocketFd(INVALID_SOCKET),
          mbConnected(false) {
    mHost = Host2IP(host);
}

TcpClient::~TcpClient() {
    DisConnect();
}

bool TcpClient::Connect() {
    if (mbConnected) {
        return true;
    }
    if (mSocketFd != INVALID_SOCKET) {
        if (IsConnected()) {
            return true;
        }
    }

    if (!CreateClientSocket()) {
        return false;
    }

    if (!ConnectToServer()) {
        CloseSocket();
        return false;
    }
    mbConnected = true;
    return true;
}

bool TcpClient::DisConnect() {

    CloseSocket();

    mSocketFd = INVALID_SOCKET;
    mbConnected = false;
    return false;
}

bool TcpClient::Send(const std::string &text) {
    Connect();
    size_t dataSend = 0;
    size_t dataLen = text.size();
    const char *dataPtr = text.c_str();
    while (dataSend < dataLen) {
        size_t toSendSize =
                dataLen - dataSend <= mReadBlockSize ? dataLen - dataSend : mReadBlockSize;
        int sendCnt = send(mSocketFd, dataPtr + dataSend, toSendSize, 0);
        if (sendCnt < 0) {
            std::string errStr = GetErrorString();
            printf("%s", errStr.c_str());
            //TODO 判断是否需要断开连接
            DisConnect();
            return false;
        }
        dataSend += sendCnt;
    }
    return true;
}

bool TcpClient::Read(std::string &text, int &len) {
    if (mReadBuffer.size() > 0) {
        text = mReadBuffer;
        mReadBuffer.clear();
        return true;
    }

    Connect();

    text.resize(mReadBlockSize, 0);
    len = recv(mSocketFd, &text[0], mReadBlockSize, 0);
    if (len < 0) {
        //TODO 接受数据错误处理
        return false;
    }
    if (len == 0) {//连接关闭
        return false;
    }

    text.resize(len);
    return true;
}

bool TcpClient::Read(std::string &text) {
    int code;
    return Read(text,code);
}

bool TcpClient::ReadUtil(std::string &text, const std::string &util) {
    size_t pos;
    std::string tmpBuffer;
    while (Read(tmpBuffer)) {
        mReadBuffer.append(tmpBuffer.c_str(), tmpBuffer.size());
        //TODO 非字符串可能有bug，待处理
        if ((pos = mReadBuffer.find(util)) != std::string::npos) {
            text.resize(pos + util.size());
            memcpy(&text[0], mReadBuffer.c_str(), text.size());
            mReadBuffer = mReadBuffer.substr(text.size());
            return true;
        }
    }
    return false;
}

bool TcpClient::ReadAll(std::string &text, size_t len) {
    if (len <= 0) {
        return false;
    }

    std::string tmpBuffer;
    text.resize(len, 0);

    char *dataPtr = &text[0];
    const char *dataEnd = dataPtr + len;
    while (Read(tmpBuffer)) {
        size_t dataLast = dataEnd - dataPtr;
        size_t readLen = tmpBuffer.size();
        if (dataLast > readLen) {
            memcpy(dataPtr, tmpBuffer.c_str(), tmpBuffer.size());
            dataPtr += readLen;
        } else {
            memcpy(dataPtr, tmpBuffer.c_str(), dataLast);
            mReadBuffer = tmpBuffer.substr(readLen - dataLast);
            return true;
        }
    }

    return false;
}

void TcpClient::SetReadBlockSize(int blockSize) {
    mReadBlockSize = blockSize;
}

bool TcpClient::SetTimeOut(int timeOut) {
    if (mSocketFd == INVALID_SOCKET) {
        return false;
    }
    mTimeOut = timeOut;
    struct timeval tv;
    tv.tv_sec = timeOut / 1000;
    tv.tv_usec = timeOut % 1000;
    bool bSuccess = true;
    if (0 != setsockopt(mSocketFd, SOL_SOCKET, SO_SNDTIMEO, (const char *) &tv, sizeof(tv))) {
        bSuccess = false;
    }
    if (0 != setsockopt(mSocketFd, SOL_SOCKET, SO_RCVTIMEO, (const char *) &tv, sizeof(tv))) {
        bSuccess = false;
    }

    return bSuccess;
}

bool TcpClient::CreateClientSocket() {
    struct sockaddr_in client_addr;
    bzero(&client_addr, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = htons(INADDR_ANY);
    client_addr.sin_port = htons(0);

    mSocketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (mSocketFd < 0) {
        mSocketFd = INVALID_SOCKET;
        return false;
    }

    if (::bind(mSocketFd, (struct sockaddr *) &client_addr, sizeof(client_addr))) {
        CloseSocket();
        return false;
    }

    return true;
}


bool TcpClient::ConnectToServer() {
    if (mSocketFd == INVALID_SOCKET) {
        return false;
    }
    struct sockaddr_in server_addr;

    if (!GetSockAddrByHostName(mHost, mPort, server_addr)) {
        return false;
    }

    socklen_t server_addr_length = (socklen_t) sizeof(server_addr);

    if (connect(mSocketFd, (struct sockaddr *) &server_addr, server_addr_length) < 0) {
        //LOGV("Can Not Connect To %s:%d->%d!\n", hostname,portnumber,socket_geterr());
        return false;
    }
    return true;
}

bool TcpClient::CloseSocket() {
    if (mSocketFd == INVALID_SOCKET) {
        return true;
    }
    shutdown(mSocketFd, SHUT_RDWR);
    int r = close(mSocketFd);
    if (0 == r) {
        mSocketFd = INVALID_SOCKET;
        return true;
    }
    return false;
}

bool TcpClient::GetSockAddrByHostName(const std::string &host, int port,
                                      struct sockaddr_in &sockIn) {

    bzero(&sockIn, sizeof(sockIn));
    sockIn.sin_family = AF_INET;

    if (inet_aton(host.c_str(), &sockIn.sin_addr) == 0) {
        return false;
    }
    sockIn.sin_port = htons(port);
    return true;
}

bool TcpClient::IsConnected() {

    if (!mbConnected) {
        return false;
    }

    if (mSocketFd == INVALID_SOCKET) {
        mbConnected = false;
        return false;
    }
#ifdef WIN32
    bool ret = true;
    HANDLE closeEvent = WSACreateEvent();
    WSAEventSelect(mSocketFd, closeEvent, FD_CLOSE);

    DWORD dwRet = WaitForSingleObject(closeEvent, 0);

    if (dwRet == WSA_WAIT_EVENT_0) {
        mSocketFd = INVALID_SOCKET;
        mbConnected = false;
        ret = false;
    }
    else if (dwRet == WSA_WAIT_TIMEOUT) {
        ret = true;
    }
    WSACloseEvent(closeEvent);
    return ret;

#else
    struct tcp_info info;
    int len = sizeof(info);
    getsockopt(mSocketFd, IPPROTO_TCP, TCP_INFO, &info, &len);
    if (info.tcpi_state == TCP_ESTABLISHED) { //TODO SOCK无效，根据情况断开?
        return true;
    } else if (info.tcpi_state == TCP_CLOSE || info.tcpi_state == TCP_CLOSING) {
        mSocketFd = INVALID_SOCKET;
        mbConnected = false;
    } else {
        DisConnect();
        return false;
    }
#endif // WIN32
    return false;
}

int TcpClient::GetErrorNo() {
#ifdef WIN32
    return ::WSAGetLastError();
#else
    return errno;
#endif

}

std::string TcpClient::GetErrorString() {
    return strerror(errno);
}






