/************************************ <lalVerbatim file="StackSlideIsolatedHV">
Author:  Mendell, G.
$Id$
************************************* </lalVerbatim> */

/* REVISIONS: */
/* 04/12/05 gam; Move from using StackSlideOld to using StackSlide function. */
/* 04/12/05 gam; add RunStackSlideIsolatedMonteCarloSimulation to StackSlideIsolated.c */
/* 05/24/05 gam; make maxPower and totalEventCount part of params; change finishSUMs to finishPeriodicTable; end xml in FinalizeSearch */

#ifndef _STACKSLIDEISOLATED_H
#define _STACKSLIDEISOLATED_H

/*********************************************/
/*                                           */
/* START SECTION: include header files       */
/*                                           */
/*********************************************/
#include <stdio.h>
#include <math.h>
#include <lal/LALStdlib.h>
#include <lal/AVFactories.h>
#include <lal/LALConstants.h>
#include <lal/LALDemod.h>
/* next two are for xml I/O */
#include <lal/LIGOLwXML.h>
#include <lal/LIGOLwXMLHeaders.h>
/* next is needed for tables defined in LAL */
#include <lal/LIGOMetadataTables.h>
/* 04/12/05 gam; next two are needed to inject signals for Monte Carlo simulations. */
#include <lal/GeneratePulsarSignal.h>
#include <lal/Random.h>
#include "DriveStackSlide.h"
#include "StackSlide.h"
/* #include <lal/LALStackSlide.h> Will need to switch to this version when StackSlide is in LAL. */
/*********************************************/
/*                                           */
/* END SECTION: include header files         */
/*                                           */
/*********************************************/

#ifdef __cplusplus
extern "C" {
#endif
  
NRCSID (STACKSLIDEISOLATEDH, "$Id$");

/*********************************************/
/*                                           */
/* START SECTION: define constants           */
/*                                           */
/*********************************************/
#define STACKSLIDEISOLATEDH_ENULL 1
#define STACKSLIDEISOLATEDH_ENNUL 2
#define STACKSLIDEISOLATEDH_ENEGA 4
#define STACKSLIDEISOLATEDH_EBADRESULTSFILE 4
#define STACKSLIDEISOLATEDH_MSGENULL "Null Pointer"
#define STACKSLIDEISOLATEDH_MSGENNUL "Non-Null Pointer"
#define STACKSLIDEISOLATEDH_MSGENEGA "Bad Negative Value"
#define STACKSLIDEISOLATEDH_MSGEBADRESULTSFILE "Could not open priorResultsFile"
/*********************************************/
/*                                           */
/* END SECTION: define constants             */
/*                                           */
/*********************************************/

void StackSlideIsolated (
    LALStatus                        *status,
    SnglStackSlidePeriodicTable      *loudestPeaksArray,
    LALFindStackSlidePeakOutputs     *pLALFindStackSlidePeakOutputs,
    LALFindStackSlidePeakParams      *pLALFindStackSlidePeakParams,
    LALUpdateLoudestStackSlideParams *pLALUpdateLoudestStackSlideParams,
    LALDetector                      *cachedDetector,
    StackSlideParams                 *stksldParams,
    StackSlideSearchParams           *params
);

/* 04/12/05 gam */
void RunStackSlideIsolatedMonteCarloSimulation(LALStatus *status, StackSlideSearchParams *params, INT4 nSamples);

/* 05/24/05 gam; Function that reads results from previous jobs in the pipeline */
void getStackSlidePriorResults(LALStatus *status,
                               REAL4 *priorLoudestEvent,
                               REAL8 *priorStartFreq,
                               REAL8 *priorBand,
                               REAL8 *priorConfidence,
                               REAL8 *priorUL,
                               REAL8 *priorUncertainty,
                               CHAR  *priorResultsFile);

#ifdef __cplusplus
}
#endif

#endif /* _STACKSLIDEISOLATED_H */
