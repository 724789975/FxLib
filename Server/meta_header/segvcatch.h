/***************************************************************************
 *   Copyright (C) 2009 by VisualData                                      *
 *                                                                         *
 *   Redistributed under LGPL license terms.                               *
 ***************************************************************************/

#include <setjmp.h>

#ifndef _SEGVCATCH_H
#define	_SEGVCATCH_H

/*! \brief segvcatch namespace
*/

namespace segvcatch
{
/*! Signal handler, used to redefine standart exception throwing. */
typedef void (*handler)();
typedef void (*sig_handler)(int);

/*! Initialize segmentation violation handler.
    \param h (optional) - optional user's signal handler. By default used an internal signal handler to throw
 std::runtime_error.
   */

// void init_segv(sig_handler h = 0);

/*! Initialize floating point error handler.
    \param h - optional user's signal handler. By default used an internal signal handler to throw
 std::runtime_error.*/
// void init_fpe(sig_handler h = 0);

void init_sig(int sig, sig_handler h = 0);

jmp_buf& get_jmp_buff();

void long_jmp_env(jmp_buf& env, int sig);

}

#endif	/* _SEGVCATCH_H */
