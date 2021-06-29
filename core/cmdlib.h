#ifndef CMDLIB_H
#define CMDLIB_H

typedef enum 
{
	SPEW_NORM,
	SPEW_INFO,
	SPEW_WARNING,
	SPEW_ERROR,
	SPEW_SUCCESS,
}SpewType_t;

typedef void (*SpewFunc_fn)(SpewType_t,const char*);

void SetSpewFunction(SpewFunc_fn newfunc);

void Install_ConsoleSpewFunction();

//---------------------------------------------------------------------------------------------------------------

// developer message output
void DevMsg(SpewType_t type, const char* fmt, ...);

// simple message output
void Msg(const char *fmt,...);

// Error messages
void MsgError(const char *fmt,...);

// Info messages
void MsgInfo(const char *fmt,...);

//Warning messages
void MsgWarning(const char *fmt,...);

// Good messages
void MsgAccept(const char *fmt,...);

#endif //CMDLIB_H