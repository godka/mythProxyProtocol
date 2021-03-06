#pragma once
#include "MythConfig.hh"
#include "mythAvlist.hh"

class mythVirtualDecoder :
	public mythAvlist
{
public:
	static mythVirtualDecoder* CreateNew(void);
	void start(bool usethread = true);
	virtual void stop();
	void StopThread();
	static int MainLoopstatic(void* data);
	virtual int MainLoop();
	virtual ~mythVirtualDecoder(void);
	unsigned int GetTimeCount();
	static Uint32 TimerCallbackStatic(Uint32 interval, void *param);
protected:
	mythVirtualDecoder(void);
	Uint32 TimerCallback(Uint32 interval);
	int flag;
	unsigned int m_count;
	unsigned int ori_count;
	unsigned int ret_count;
	SDL_TimerID m_timeid;
	SDL_Thread* m_thread;
};

