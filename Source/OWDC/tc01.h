#include <jde.h>

#define TC01_API    __declspec( dllexport )

TC01_API int  OWDCtc01(HENV hEnv,HUSER hUser, JCHAR LszUsrID[16],char cIniFlag);
