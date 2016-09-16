/********************************************************************
  Created by MythKAst
  ©2013 MythKAst Some rights reserved.


  You can build it with vc2010,gcc.
  Anybody who gets this source code is able to modify or rebuild it anyway,
  but please keep this section when you want to spread a new version.
  It's strongly not recommended to change the original copyright. Adding new version
  information, however, is Allowed. Thanks.
  For the latest version, please be sure to check my website:
  Http://code.google.com/mythkast


  你可以用vc2010,gcc编译这些代码
  任何得到此代码的人都可以修改或者重新编译这段代码，但是请保留这段文字。
  请不要修改原始版权，但是可以添加新的版本信息。
  最新版本请留意：Http://code.google.com/mythkast
  B
 MythKAst(asdic182@sina.com), in 2013 June.
*********************************************************************/
#include "mythAvlist.hh"
#include <stdio.h>
//#include <memory.h>
mythAvlist::mythAvlist(void)
{
	listcount = 0;
	this->startread = false;
	this->totalptr = 0;
	mBufferSize = AVBUFFERSIZE;
	InitalList();
}
int mythAvlist::InitalList(){
	//inital list
	firstFrame = true;
	continueGet = true;
	totalbuffer = new unsigned char[mBufferSize * 1024 * 1024];
	ListPacket = new PacketQueue[AVFRAMECOUNT];
	for(int i = 0;i < AVFRAMECOUNT;i++){
		//ListPacket[i].YY = NULL;
		//ListPacket[i].UU = NULL;
		//ListPacket[i].VV = NULL;
		ListPacket[i].h264Packet = NULL;
	}
	listwrite = 0;
	listread = 0;
	additionalpktbuffer = new unsigned char[1 * 1024 * 1024];
	additionalpktlen = 0;
	return 0;
}
mythAvlist::mythAvlist(int BufferSize)
{
	listcount = 0;
	this->startread = false;
	mBufferSize = BufferSize;
	//totalbuffer = (unsigned char*)SDL_malloc(BufferSize * 1024 * 1024);
	totalptr = 0;
	InitalList();
}
mythAvlist *mythAvlist::CreateNew(int BufferSize){
	if(BufferSize == 0)
		return new mythAvlist();
	else
		return new mythAvlist(BufferSize);
}
mythAvlist::~mythAvlist(void)
{
	free();
}
bool mythAvlist::IsIframe(PacketQueue *pack)
{
	if (pack){
		if (pack->h264PacketLength > 4){
			unsigned char* tmp = pack->h264Packet;
			int Nalu = 0;
			for (int i = 0; i < pack->h264PacketLength - 4; i++){
				if (tmp[i] == 0 && tmp[i + 1] == 0 && tmp[i + 2] == 0 && tmp[i + 3] == 1){
					Nalu = tmp[i + 4];
				}
				else if (tmp[i + 0] == 0 && tmp[i + 1] == 0 && tmp[i + 2] == 1){
					Nalu = tmp[i + 3];
				}
				if (Nalu != 0)
					break;
			}
			int tmpnal = Nalu & 0x1f;
			if (tmpnal == 5 || tmpnal == 7 || tmpnal == 8){
				return true;
			}
		}
	}
	return false;
}
int mythAvlist::getDataFromStream(unsigned char* buf, int len){
	int retlen = 0;
	PacketQueue* pkt = NULL;
	while (1){
		SDL_PollEvent(NULL);
		pkt = get();
		if (pkt){
			if (firstFrame){
				if (IsIframe(pkt)){
					firstFrame = false;
					break;
				}else
					continue;
			}
			break;
		}
		SDL_Delay(1);
	}		
	if (pkt->h264PacketLength > len){
		SDL_memcpy(buf, pkt->h264Packet, len);
		SDL_memcpy(additionalpktbuffer, pkt->h264Packet + len, pkt->h264PacketLength - len);
		additionalpktlen = pkt->h264PacketLength - len;
		retlen = len;
	}
	else{
		SDL_memcpy(buf, pkt->h264Packet, pkt->h264PacketLength);
		additionalpktlen = 0;
		retlen = pkt->h264PacketLength;
	}
	return retlen;
}
int mythAvlist::getDataFromBuf(unsigned char* buf, int len){
	int retlen = 0;
	if (additionalpktlen > 0){
		if (additionalpktlen > len){
			SDL_memcpy(buf, additionalpktbuffer, len);
			SDL_memmove(additionalpktbuffer, additionalpktbuffer + len, additionalpktlen - len);
			additionalpktlen -= len;
			retlen = len;
		}
		else{
			SDL_memcpy(buf, additionalpktbuffer, additionalpktlen);
			retlen = additionalpktlen;
			additionalpktlen = 0;
		}
	}
	return retlen;
}
int mythAvlist::getSingleStep(unsigned char* buf, int len){
	int ret = getDataFromBuf(buf, len);
	if (ret < len){
		//说明buffer已经被取空，现在取avpacket的数据
		int tmplen = len - ret;
		int tmpindex = ret;
		int ret2 = getDataFromStream(buf + tmpindex, tmplen);
		tmpindex += ret2;
		tmplen -= ret2;
		return tmpindex;
	}
	else{
		//说明buffer没有被取空
		return ret;
	}
}
int mythAvlist::get(unsigned char* buf, int len){
	//说明buffer已经被取空，现在取avpacket的数据
	int tmplen = len;
	int tmpindex = 0;
	while (tmplen > 0){
		int ret2 = getSingleStep(buf + tmpindex, tmplen);
		tmpindex += ret2;
		tmplen -= ret2;
	}
	return tmpindex;
}

PacketQueue *mythAvlist::get(int freePacket){
	PacketQueue *tmp;
	if (this->listwrite - this->listread == 1 ||
		this->listwrite - this->listread == 0 ||
		(this->listwrite == 0 && this->listread == AVFRAMECOUNT)){
		tmp = NULL;
	}else{
		tmp = &this->ListPacket[this->listread];
		if(tmp->h264Packet == NULL){
			tmp = NULL;
		}else{
			if(freePacket == 0){
				if(listwrite - listread > 10){
					LOGE("skip frames");
					LOGE(" read = %d,write = %d,minus = %d\n",listread,listwrite,listwrite - listread);
					listread += 9;
				}else
					listread++;
			}
		}
	}
	listread %= AVFRAMECOUNT;
	return tmp;
}
unsigned char* mythAvlist::putcore(unsigned char* data,unsigned int datasize){
	if (totalptr + datasize > (unsigned int) (mBufferSize * 1024 * 1024)){
		totalptr = 0;
	}
	SDL_memcpy(totalbuffer + totalptr, data, datasize);
	totalptr += datasize;
	//printf("totalptr = %d\n",totalptr);
	return (totalbuffer + totalptr - datasize);
}
/*
int mythAvlist::put(unsigned char** dataline,unsigned int *datasize,unsigned int width,unsigned int height){
	SDL_LockMutex(this->mutex);
	if(listwrite >= AVFRAMECOUNT)listwrite = 0;
	PacketQueue *tmp = &ListPacket[listwrite];
	tmp->h264Packet = NULL;
	tmp->width = width;
	tmp->height = height;

	tmp->YY = (unsigned char*)putcore(dataline[0],datasize[0] * height);
	tmp->Ydatasize = datasize[0];
	
	tmp->UU = (unsigned char*)this->putcore(dataline[1], datasize[1] * height / 2);
	tmp->Udatasize = datasize[1];
	
	tmp->VV = (unsigned char*)this->putcore(dataline[2], datasize[2] * height / 2);
	tmp->Vdatasize = datasize[2];

	listwrite++;
	//LOGE("YUVlistcount=%d\n",listwrite);
	SDL_UnlockMutex(this->mutex);
	return 0;
}
*/
int mythAvlist::release(PacketQueue *pack)
{
	return 0;
}
int mythAvlist::put(unsigned char* data,unsigned int length){	
	if(listwrite >= AVFRAMECOUNT)listwrite = 0;
	PacketQueue *tmp = &ListPacket[listwrite];

	//tmp->YY = NULL;
	//tmp->UU = NULL;
	//tmp->VV = NULL;

	tmp->h264PacketLength = length;
	tmp->h264Packet = putcore(data, length);
	listwrite++;
	//LOGE("H264listcount=%d\n",listwrite);
	return 0;
}
int mythAvlist::free(){
	if (ListPacket)
		delete [] ListPacket;
	ListPacket = NULL;
	if (totalbuffer)
		delete [] totalbuffer;
	totalbuffer = NULL;
	return 0;
}

int mythAvlist::StopGet()
{
	return 0;
}
