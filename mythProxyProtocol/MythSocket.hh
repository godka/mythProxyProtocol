#pragma once
#define CURL_STATICLIB  //HTTP_ONLY
#include "curl/curl.h"
#include "SDL2/SDL.h"

class MythSocket
{
public:
	int SendStr(const char* str,int length = -2);
	int ReceiveData(char* recvBuf,int recvLength);
	int CloseSocket();
	int ReceiveDataLn(char* recvBuf,int recvLength,char* lnstr);
	static MythSocket *CreateNew(const char* ip, u_short port);
	~MythSocket(void);
protected:
	char downbuffer[4097];
	int downlength;
	CURL *curl;
	CURLcode res;
	long sockextr;
	curl_socket_t sockfd;
	MythSocket();
	MythSocket(const char* ip, u_short port);
private:
	int BindAddressPort(const char* ip, u_short port);
	int cmp(char* buff, char*str, int length);
	int InitalSocket();
	int wait_on_socket(curl_socket_t sockfd, int for_recv, long timeout_ms);
};

