//
// Created by Homin Su on 2023/6/25.
//

#ifndef VUML_INCLUDE_VUML_VUML_H_
#define VUML_INCLUDE_VUML_VUML_H_

#ifndef VUML_ASSERT
#include <cassert>
#define VUML_ASSERT(x) assert(x)
#endif // VUML_ASSERT

#ifndef VUML_LENGTH
#define VUML_LENGTH(CONST_ARRAY) sizeof(CONST_ARRAY) / sizeof(CONST_ARRAY[0])
#endif // VUML_LENGTH

#ifndef VUML_STR_LENGTH
#if defined(_MSC_VER)
#define VUML_STR_LENGTH(CONST_STR) _countof(CONST_STR)
#else
#define VUML_STR_LENGTH(CONST_STR) sizeof(CONST_STR) / sizeof(CONST_STR[0])
#endif
#endif // VUML_STR_LENGTH

#if defined(_WIN64) || defined(WIN64) || defined(_WIN32) || defined(WIN32)
#if defined(_WIN64) || defined(WIN64)
#define VUML_ARCH_64 1
#else
#define VUML_ARCH_32 1
#endif
#define VUML_PLATFORM_STRING "windows"
#define VUML_WINDOWS 1
#elif defined(__linux__)
#define VUML_PLATFORM_STRING "linux"
#define VUML_LINUX 1
#ifdef _LP64
#define VUML_ARCH_64 1
#else /* _LP64 */
#define VUML_ARCH_32 1
#endif /* _LP64 */
#elif defined(__APPLE__)
#define VUML_PLATFORM_STRING "osx"
#define VUML_APPLE 1
#ifdef _LP64
#define VUML_ARCH_64 1
#else /* _LP64 */
#define VUML_ARCH_32 1
#endif /* _LP64 */
#endif

#ifndef VUML_WINDOWS
#define VUML_WINDOWS 0
#endif
#ifndef VUML_LINUX
#define VUML_LINUX 0
#endif
#ifndef VUML_APPLE
#define VUML_APPLE 0
#endif

#endif //VUML_INCLUDE_VUML_VUML_H_
