#include "eProsimaDL.h"
#include "../log/eProsimaLog.h"

#if defined(RTI_WIN32)
#include <Windows.h>
#elif (defined(RTI_UNIX) || defined(RTI_LINUX))
#include <dlfcn.h>
#endif

void* eProsimaLoadLibrary(const char *filename)
{
    const char* const METHOD_NAME = "eProsimaLoadLibrary";
    void *libraryHandle = NULL;

    if(filename != NULL)
    {
#if defined(RTI_WIN32)
        libraryHandle = LoadLibrary(filename);
#elif (defined(RTI_UNIX) || defined(RTI_LINUX))
        libraryHandle = dlopen(filename, RTLD_LAZY);
#endif
    }
    else
    {
        printError("Bad parameter (filename)");
    }

    return libraryHandle;
}

void* eProsimaGetProcAddress(void *libraryHandle, const char *functionName)
{
    const char* const METHOD_NAME = "eProsimaGetProcAddress";
    void *functionPointer = NULL;

    if(libraryHandle != NULL && functionName != NULL)
    {
#if defined(RTI_WIN32)
        functionPointer = GetProcAddress(libraryHandle, functionName);
#elif (defined(RTI_UNIX) || defined(RTI_LINUX))
        functionPointer = dlsym(libraryHandle, functionName);
#endif
    }
    else
    {
        printError("Bad parameters");
    }

    return functionPointer;
}
