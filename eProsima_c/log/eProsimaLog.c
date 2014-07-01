#include "eProsimaLog.h"
#include "../macros/snprintf.h"

#include <stdarg.h>
#include <malloc.h>

#ifdef __linux
static const char* const EPROSIMA_LOG_COLOR[] = {"\033[0;31m", "\033[0;33m", "\033[0;34m", "\033[0;0m"};
#else
static const char* const EPROSIMA_LOG_COLOR[] = {"", "", "", ""};
#endif

static const char* const EPROSIMA_PRINT_MESSAGES[] = {"%sERROR<%s>: %s%s\n", "%sWARNING<%s>: %s%s\n", "%sINFO<%s>: %s%s\n"};

static const char* const EPROSIMA_LOG_MESSAGES[] = {"ERROR<%s>: ", "WARNING<%s>: ", "INFO<%s>: "};

static const int EPROSIMA_LOG_LAST_MESSAGE_TYPE = EPROSIMA_LOG_INFO + 1;

static EPROSIMA_LOG_VERBOSITY_LEVEL globalVerbosity = EPROSIMA_ERROR_VERBOSITY_LEVEL;

void eProsimaLog_setVerbosity(EPROSIMA_LOG_VERBOSITY_LEVEL level)
{
    globalVerbosity = level;
}

void eProsimaLog_print(EPROSIMA_LOG_MESSAGE_TYPE messageType, const char *method_text, const char *message)
{
    if((unsigned int)messageType < (unsigned int)globalVerbosity)
        printf(EPROSIMA_PRINT_MESSAGES[messageType], EPROSIMA_LOG_COLOR[messageType], method_text, message, EPROSIMA_LOG_COLOR[EPROSIMA_LOG_LAST_MESSAGE_TYPE]);
}

struct eProsima_Log* eProsimaLog_new(const char *filename)
{
    const char* const METHOD_NAME = "eProsimaLog_new";
    struct eProsima_Log *log = NULL;

    log = (struct eProsima_Log*)malloc(sizeof(struct eProsima_Log));

    if(log != NULL)
    {
        log->m_verbosity = EPROSIMA_ERROR_VERBOSITY_LEVEL;

        if(filename != NULL)
            log->m_logFile = fopen(filename, "a");

        if(log->m_logFile == NULL)
            log->m_logFile = stdout;
    }
    else
    {
        printError("Cannot create the eProsimaLog structure");
    }

    return log;
}

void eProsimaLog_delete(struct eProsima_Log *log)
{
    if(log != NULL)
    {
        if(log->m_logFile != NULL &&
                log->m_logFile != stdout)
            fclose(log->m_logFile);

        free(log);
    }
}

void eProsimaLog_setLogVerbosity(struct eProsima_Log *log, EPROSIMA_LOG_VERBOSITY_LEVEL level)
{
    if(log != NULL)
    {
        log->m_verbosity = level;
    }
}

void eProsimaLog_write(struct eProsima_Log *log, EPROSIMA_LOG_MESSAGE_TYPE messageType,
        const char *method_text, const char *message, ...)
{
    va_list arg_ptr;

    if(log != NULL)
    {
        if((unsigned int)messageType < (unsigned int)log->m_verbosity)
        {
            fprintf(log->m_logFile, EPROSIMA_LOG_MESSAGES[messageType], method_text);
            va_start(arg_ptr, message);
            vfprintf(log->m_logFile, message, arg_ptr);
            va_end(arg_ptr);

            fflush(log->m_logFile);
        }
    }
}
