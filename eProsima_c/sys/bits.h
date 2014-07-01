#ifndef _EPROSIMA_C_SYS_BITS_H_
#define _SYS_BITS_H_

#ifndef BSWAP32
#if defined(_WIN32)
#define BSWAP32 _byteswap_ulong
#elif __linux
#define BSWAP32 __builtin_bswap32
#endif
#endif // BSWAP32

/* Work with bytes */
/* endianness = 1 => Litlle-endian | endianness = 0 => Big-endian */
#if defined(__LITTLE_ENDIAN__)
#define GET_INT_ENDIAN(endianness, buffer) \
    endianness ? ((unsigned int*)(buffer))[0] : \
ntohl(((unsigned int*)(buffer))[0])

#define SET_INT_ENDIAN(endianness, buffer, integer) \
    ((unsigned int*)(buffer))[0] = endianness ?  integer : BSWAP32(integer)
#elif defined(__BIG_ENDIAN__)
#else
#error Either __BIG_ENDIAN__ or __LITTLE_ENDIAN__ must be defined.
#endif

#endif // _EPROSIMA_C_SYS_BITS_H_
