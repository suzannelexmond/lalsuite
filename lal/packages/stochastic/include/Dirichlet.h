/*-----------------------------------------------------------------------
 *
 * File Name: Dirichlet.h
 *
 * Author: UTB Relativity Group
 *
 * Revision: $Id$
 *
 *-----------------------------------------------------------------------
 *
 * NAME
 * Dirichlet.h
 *
 * SYNOPSIS
 * #include "Dirichlet.h"
 *
 * DESCRIPTION
 * Error codes, typedefs, and prototype for function Dirichlet()
 *
 * DIAGNOSTICS
 *
 *-----------------------------------------------------------------------
 *
 * REVISION HISTORY
 *
 * $Log$
 * Revision 1.1  2000/02/26 21:19:41  jolien
 * Overlap modified; Dirichlet added.
 *
 *
 *-----------------------------------------------------------------------
 */

#ifndef _DIRICHLET_H
#define _DIRICHLET_H

#ifndef _LALSTDLIB_H
#include "LALStdlib.h"
#ifndef _LALSTDLIB_H
#define _LALSTDLIB_H
#endif
#endif

static volatile const char *DIRICHLETH = "$Id$";

#define DIRICHLET_ENULLIP     1
#define DIRICHLET_ENVALUE     2
#define DIRICHLET_ESIZE       4
#define DIRICHLET_EDELTAX     8
#define DIRICHLET_ENULLOP     16
#define DIRICHLET_ESIZEMM     32
#define DIRICHLET_ENULLD      64

#define DIRICHLET_MSGENULLIP  "Pointer to input parameters must be non-null"
#define DIRICHLET_MSGENVALUE  "Dirichlet parameter N must be > 0"
#define DIRICHLET_MSGESIZE    "Specified length of output vector must be > 0"
#define DIRICHLET_MSGEDELTAX  "Spacing of x values must be > 0"
#define DIRICHLET_MSGENULLOP  "Pointer to ouput vector must be non-null"
#define DIRICHLET_MSGESIZEMM  "Length of output vector must agree with length specified in input parameters"
#define DIRICHLET_MSGENULLD   "Pointer to data member of output vector must be non-null"

typedef struct tagDirichletParameters{
  INT4	 n;       /* Dirichlet parameter N */
  INT4	 length;  /* specified length of output vector */
  REAL8	 deltaX;  /* spacing of x values */
} DirichletParameters; 

void 
Dirichlet ( Status*, REAL4Vector*, DirichletParameters* );

#endif /* _DIRICHLET_H */

