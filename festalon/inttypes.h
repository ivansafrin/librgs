#ifndef __INTTYPES_H_
#define __INTTYPES_H_

#ifdef _WINDOWS
typedef signed __int8		int8_t;
typedef unsigned __int8		uint8_t;
typedef signed __int16		int16_t;
typedef unsigned __int16	uint16_t;
typedef signed __int32		int32_t;
typedef unsigned __int32	uint32_t;
typedef signed __int64		int64_t;
typedef unsigned __int64	uint64_t;

//typedef int16_t intptr_t;
//typedef uint16_t uintptr_t;
#endif

#endif