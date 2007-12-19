/*
 * Copyright (C) 2006 Reinhard Prix
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with with program; see the file COPYING. If not, write to the 
 *  Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 *  MA  02111-1307  USA
 */

/**
 * \author Reinhard Prix
 * \date 2006
 * \file 
 * \ingroup pulsar
 * \brief Header-file defining the API for the LISA-specific functions
 *
 * $Id$
 *
 */

#ifndef _LISASPECIFICS_H  /* Double-include protection. */
#define _LISASPECIFICS_H

/* C++ protection. */
#ifdef  __cplusplus
extern "C" {
#endif

#include <lal/LALRCSID.h>
NRCSID( LISASPECIFICSH, "$Id$" );

/*---------- exported INCLUDES ----------*/
#include <lal/LALDetectors.h>
#include <lal/LALDatatypes.h>
#include <lal/DetectorStates.h>
#include <lal/ComplexAM.h>

/*---------- exported DEFINES ----------*/

#define LISA_TIME_ORIGIN		700000000; 	/* ad-hoc setting for MLDC 't=0' to GPS for Tue Mar 12 20:26:27 GMT 2002 */

/*---------- exported types ----------*/
/** Translate TDI arm indices to C-indexing */
typedef enum { 
  LISA_ARM1 = 0, 
  LISA_ARM2, 
  LISA_ARM3 
} LISAarmT;

/*---------- exported Global variables ----------*/
/* empty init-structs for the types defined in here */

/*---------- exported prototypes [API] ----------*/
int XLALcreateLISA ( LALDetector *Detector, CHAR channelNum );

int XLALprecomputeLISAarms ( DetectorState *detState );

int XLALgetLISADetectorTensorLWL ( DetectorTensor *detT, const Detector3Arms detArms, CHAR channelNum );
int XLALgetLISADetectorTensorRAA ( CmplxDetectorTensor *detT, const Detector3Arms detArms, PulsarDopplerParams doppler, CHAR channelNum );


#ifdef  __cplusplus
}
#endif
/* C++ protection. */

#endif  /* Double-include protection. */
