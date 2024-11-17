#ifndef __PSCONFIG_H__
#define __PSCONFIG_H__

#if defined __PALMOS__
	#define OS_PALM
	#define COMPILER_GCC
#elif defined WIN32
	#define OS_WIN32
	#define COMPILER_VC
#elif defined _WIN32_WCE
	#define OS_WIN32
	#define COMPILER_VC
#else
	#define OS_SYMBIAN
#endif

#ifdef OS_WIN32
	#include <WINDOWS.H>
	#include <STDIO.H>
	#ifdef _DEBUG
//		#define _CRTDBG_MAP_ALLOC
//		#include <CRTDBG.H>
//		#define new new( _CLIENT_BLOCK, __FILE__, __LINE__)
	#endif
#endif

#ifdef __SYMBIAN32__
	#define OS_EPOC
	#ifdef __VC32__
		#define COMPILER_VC
	#else
		#define COMPILER_GCC
	#endif
#endif

#ifdef OS_PALM
	#include <PalmOS.h>
#endif

#ifdef OS_SYMBIAN
	#include <coeccntx.h>

	#include <eikenv.h>
	#include <qikdocument.h>
	#include <qikapplication.h>
	#include <qikappui.h>
	#include <eikmenup.h>

	#include <eikon.hrh>
#endif

#endif
