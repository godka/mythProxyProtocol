#pragma once
//#include "SDL2/SDL.h"
#include "MythConfig.hh"
class mythAvlist
{
public:
	static mythAvlist* CreateNew(int BufferSize = 0);
	~mythAvlist(void);
	bool IsIframe(PacketQueue *pack);
	PacketQueue *get(int freePacket = 0);
	int get(unsigned char* buf, int len);
	int put(unsigned char* data, unsigned int length);
	//int put(unsigned char** dataline,unsigned int *datasize,unsigned int width,unsigned int height);
	int release(PacketQueue *pack);
	int free();
	int StopGet();
private:
	int InitalList();
	int mBufferSize;
	unsigned char* additionalpktbuffer;
	int additionalpktlen;
	int getDataFromBuf(unsigned char* buf, int len);
	int getSingleStep(unsigned char* buf, int len);
	int getDataFromStream(unsigned char* buf, int len);
	bool continueGet;
	bool firstFrame;
protected:
	bool startread;
	mythAvlist(void);
	mythAvlist(int BufferSize);
	//int InitalList(void);
	int abort_request;
	unsigned char* totalbuffer;
	unsigned int totalptr;
	unsigned int listcount;
	PacketQueue* ListPacket;
	int listwrite,listread;
	unsigned char* putcore(unsigned char* data,unsigned int datasize);
};

