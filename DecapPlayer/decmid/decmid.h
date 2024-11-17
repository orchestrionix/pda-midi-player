
// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the DECMID_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// DECMID_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifdef DECMID_EXPORTS
#define DECMID_API __declspec(dllexport)
#else
#define DECMID_API __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif

enum {
	DECMID_ERR_NONE = 0,
	DECMID_ERR_UNSUPPORTED_FORMAT = 1,
	DECMID_INVALID_USER_ID = 2
};

/*
pIn:        Pointer to the memory that holds the encrypted midi
inLenght:   Length of the encrypted midi
userId:     The user id used to encrypt the midi
pOut:       Pointer to the memory to hold the decrypted midi. Use NULL to get the decoded midi length and skip the decode process
pOutLength: Pointer to an int to hold the length of decrypted midi data
*/

DECMID_API int _stdcall DecapMidi_Decrypt(const void* pIn, unsigned int inLenght, int userId, void* pOut, unsigned int* pOutLength);

#ifdef  __cplusplus
}
#endif

