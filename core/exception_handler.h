#ifndef EXCEPTION_HANLDER_H
#define EXCEPTION_HANLDER_H

using UnhandledExceptionCallback = void(*)();
void set_signal_handler(UnhandledExceptionCallback callback);

#endif // EXCEPTION_HANLDER_H