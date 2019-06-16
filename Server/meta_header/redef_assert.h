#ifndef __REDEF_ASSERT_H__
#define __REDEF_ASSERT_H__

#include <assert.h>

#ifdef NDEBUG
#define Assert(expression) (void)(!!(expression))
#else
#define Assert assert
#endif // NDEBUG

#endif