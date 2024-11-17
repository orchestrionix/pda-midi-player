#ifndef __PSSYS_H__
#define __PSSYS_H__

class CPsSys  
{
public:
	static void* memset(void* p, int i, unsigned int len){
		return ::memset(p, i, len);
	}
	static void* memcpy(void* p, const void* ps, unsigned int len){
		return ::memcpy(p, ps, len);
	}
	static unsigned int GetTickInMS() {
		return ::GetTickCount();
	}
	static void* memmove(void* p, const void* ps, unsigned int len){
		return ::memmove(p, ps, len);
	}
};

#endif
