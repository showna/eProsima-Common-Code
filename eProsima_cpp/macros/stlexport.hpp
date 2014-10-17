#ifndef _EPROSIMA_CPP_MACROS_STLEXPORT_HPP_
#define _EPROSIMA_CPP_MACROS_STLEXPORT_HPP_

#ifdef _WIN32

#define STL_STRING_EXPORT(dllexport) \
    template class dllexport std::allocator<char>; \
    template class dllexport std::basic_string<char, std::char_traits<char>, std::allocator<char> >;

#else

#define STL_STRING_EXPORT(dllexport)

#endif //_WIN32

#endif //_EPROSIMA_CPP_MACROS_STLEXPORT_HPP_
