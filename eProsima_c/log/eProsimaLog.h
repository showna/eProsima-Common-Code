#ifndef _EPROSIMA_C_LOG_EPROSIMALOG_H_
#define _EPROSIMA_C_LOG_EPROSIMALOG_H_

#include <stdio.h>

#define printError(message) eProsimaLog_print(EPROSIMA_LOG_ERROR, METHOD_NAME, message)

#define printWarning(message) eProsimaLog_print(EPROSIMA_LOG_WARNING, METHOD_NAME, message)

#define printInfo(message) eProsimaLog_print(EPROSIMA_LOG_INFO, METHOD_NAME, message)

#define logError(logObject, message, ...) eProsimaLog_write(logObject, EPROSIMA_LOG_ERROR, METHOD_NAME, message, ##__VA_ARGS__)

#define logWarning(logObject, message, ...) eProsimaLog_write(logObject, EPROSIMA_LOG_WARNING, METHOD_NAME, message, ##__VA_ARGS__)

#define logInfo(logObject, message, ...) eProsimaLog_write(logObject, EPROSIMA_LOG_INFO, METHOD_NAME, message, ##__VA_ARGS__)

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

    typedef enum EPROSIMA_LOG_VERBOSITY_LEVEL
    {
        EPROSIMA_QUIET_VERBOSITY_LEVEL = 0,
        EPROSIMA_ERROR_VERBOSITY_LEVEL,
        EPROSIMA_WARNING_VERBOSITY_LEVEL,
        EPROSIMA_INFO_VERBOSITY_LEVEL
    } EPROSIMA_LOG_VERBOSITY_LEVEL;

    typedef enum EPROSIMA_LOG_MESSAGE_TYPE
    {
        EPROSIMA_LOG_ERROR = 0,
        EPROSIMA_LOG_WARNING,
        EPROSIMA_LOG_INFO
    } EPROSIMA_LOG_MESSAGE_TYPE;

    void eProsimaLog_setVerbosity(EPROSIMA_LOG_VERBOSITY_LEVEL level);

    void eProsimaLog_print(EPROSIMA_LOG_MESSAGE_TYPE messageType, const char *method_text, const char *message);

    struct eProsima_Log
    {
        EPROSIMA_LOG_VERBOSITY_LEVEL m_verbosity;

        FILE *m_logFile;
    };

    struct eProsima_Log* eProsimaLog_new(const char *filename);

    void eProsimaLog_delete(struct eProsima_Log *log);

    void eProsimaLog_setLogVerbosity(struct eProsima_Log *log, EPROSIMA_LOG_VERBOSITY_LEVEL level);

    void eProsimaLog_write(struct eProsima_Log *log, EPROSIMA_LOG_MESSAGE_TYPE messageType, const char *method_text, const char *message, ...);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _EPROSIMA_C_LOG_EPROSIMALOG_H_
