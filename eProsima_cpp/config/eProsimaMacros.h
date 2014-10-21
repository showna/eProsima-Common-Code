fndef _EPROSIMA_CPP_EPROSIMAMACROS_H_
#define _EPROSIMA_CPP_EPROSIMAMACROS_H_
 
/* Useful macros */
 
#ifndef SNPRINTF
#if defined(_WIN32)
#define SNPRINTF _snprintf
#elif defined(__linux)
#define SNPRINTF snprintf
#endif
#endif // SNPRINTF
 
#if defined(__linux)
#define EPROSIMA_USE_NOEXCEPT _GLIBCXX_USE_NOEXCEPT
#else
#define EPROSIMA_USE_NOEXCEPT
#endif
 
#endif // _EPROSIMA_CPP_EPROSIMAMACROS_H_
