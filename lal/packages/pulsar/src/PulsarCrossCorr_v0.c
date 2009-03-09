/*
 *  Copyright (C) 2007 Badri Krishnan
 *  Copyright (C) 2008 Christine Chung, Badri Krishnan and John Whelan
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
 * \author Christine Chung, Badri Krishnan, John Whelan
 * \date 2008
 * \file 
 * \ingroup pulsar
 * \brief LAL routines for CW cross-correlation searches
 *
 * $Id$
 *
 */

#include <lal/DopplerScan.h>
#include <lal/PulsarCrossCorr_v0.h>
#include <gsl/gsl_permutation.h>



RCSID( "$Id$");

void LALCombineAllSFTs_v0 ( LALStatus *status,
			 SFTVector **outsfts,	   
			 MultiSFTVector *multiSFTs,  
			 REAL8 length)
{
  SFTVector *ret;
  SFTtype *tmpsft = NULL;
  INT4 j,k, counter = 0;


  INITSTATUS (status, "CombineAllSFTs", rcsid);
  ATTATCHSTATUSPTR (status);


  /*initialise return sftvector*/
  ret = (SFTVector *)LALCalloc(1, sizeof(SFTVector));
  ret->length = length;
  ret->data = LALCalloc(ret->length, sizeof(COMPLEX8FrequencySeries));
 
  for (j=0; j < (INT4)multiSFTs->length; j++) {
 	for (k=0; k < (INT4)multiSFTs->data[j]->length; k++) {

		TRY (LALCopySFT(status->statusPtr, &(ret->data[counter++]), &(multiSFTs->data[j]->data[k])), status);
  	}
  }

  /* now sort the sfts according to epoch using insertion sort... inefficient?*/

  /*initialise tmp SFT */

  for (j=1; j < length; j++) {
  TRY (LALCreateSFTtype(status->statusPtr, &tmpsft, 0), status);

	tmpsft->data = NULL;
 	TRY (LALCopySFT(status->statusPtr, tmpsft, &(ret->data[j])), status);
	k = j-1;
	while ( (k >= 0) && (XLALGPSDiff(&(ret->data[k].epoch), &(tmpsft->epoch)) > 0) ) {

		/*need to destroy the sfttype before copying otherwise will cause memory leak*/
		XLALDestroyCOMPLEX8Sequence(ret->data[k+1].data);
		ret->data[k+1].data = NULL;

		TRY (LALCopySFT(status->statusPtr, &(ret->data[k+1]), &(ret->data[k])), status);

		k = k-1;

		XLALDestroyCOMPLEX8Sequence(ret->data[k+1].data);
		ret->data[k+1].data = NULL;
		TRY (LALCopySFT(status->statusPtr, &(ret->data[k+1]), tmpsft), status);

    	} 
  TRY (LALDestroySFTtype(status->statusPtr, &tmpsft),status);

  }

  (*outsfts) = ret;



  DETATCHSTATUSPTR (status);
	
  /* normal exit */	
  RETURN (status);

}

void LALCreateSFTPairsIndicesFrom2SFTvectors_v0(LALStatus          *status,
					     INT4VectorSequence **out,
					     SFTVector          *in,
					     SFTPairParams_v0      *par,
					     INT4 		detChoice)
{
  
  UINT4 i, j, numsft, numPairs;
  INT4 thisPair;
  INT4 sameDet = 0;
  INT4Vector *List1, *List2;
  INT4VectorSequence *ret;
  COMPLEX8FrequencySeries  *thisSFT1, *thisSFT2;	       


  INITSTATUS (status, "CreateSFTPairsIndicesFrom2SFTvectors", rcsid);
  ATTATCHSTATUSPTR (status);

  ASSERT (in, status, PULSARCROSSCORR_V0_ENULL, PULSARCROSSCORR_V0_MSGENULL);
  ASSERT (par, status, PULSARCROSSCORR_V0_ENULL, PULSARCROSSCORR_V0_MSGENULL);

  /* number of SFTs */
  numsft = in->length;
  
  List1 = XLALCreateINT4Vector(numsft*numsft);
  List2 = XLALCreateINT4Vector(numsft*numsft);

  numPairs = 0; /* initialization */



  thisPair = 0;

  
  /* go over all sfts */
  for (i=0; i<numsft; i++) {

    thisSFT1 = in->data + i;
    
    /* go over all sfts again and check if it should be paired with thisSFT1 */
    /* this can be made more efficient if necessary */
    for (j=i; j<numsft; j++) {

      LIGOTimeGPS t1, t2;
      REAL8 timeDiff;

      thisSFT2 = in->data + j;

      /* calculate time difference */      
      t1 = thisSFT1->epoch;
      t2 = thisSFT2->epoch;
      timeDiff = XLALGPSDiff( &t1, &t2);
      
      sameDet = strcmp(thisSFT1->name, thisSFT2->name);

      if (sameDet != 0) { sameDet = 1; }
      
      if (detChoice == 2) { sameDet = detChoice; }

      /* decide whether to add this pair or not */
      if ((sameDet == detChoice) && (fabs(timeDiff) <= par->lag) && (i != j) ) {
	numPairs++;

	List1->data[thisPair] = i;
	List2->data[thisPair++] = j;
      } /* if ( numPairs < out->length)  */
	
      thisSFT2 = NULL;
    } /* end loop over second sft set */
     
    thisSFT1 = NULL;
  } /* end loop over first sft set */ 



  /* initialise pair list vector*/
  ret = (INT4VectorSequence *) LALCalloc(1, sizeof(INT4VectorSequence));
  ret->length = 2;
  ret->vectorLength = numPairs;
  ret->data = LALCalloc(ret->length * ret->vectorLength, sizeof(INT4));

  for (i=0; i < numPairs; i++) {

  ret->data[i] = List1->data[i];
  ret->data[i+numPairs] = List2->data[i];
  }

  (*out) = ret;

  XLALDestroyINT4Vector(List1);
  XLALDestroyINT4Vector(List2);
  thisSFT1 = NULL;
  thisSFT2 = NULL;  

  DETATCHSTATUSPTR (status);
	
  /* normal exit */	
  RETURN (status);

} /* CreateSFTPairsIndicesFrom2SFTvectors */


/** Correlate a single pair of SFT at a parameter space point*/
void LALCorrelateSingleSFTPair_v0(LALStatus                *status,
			       COMPLEX16                *out,
			       COMPLEX8FrequencySeries  *sft1,
			       COMPLEX8FrequencySeries  *sft2,
			       REAL8FrequencySeries     *psd1,
			       REAL8FrequencySeries     *psd2,
			       REAL8                    *freq1,
			       REAL8                    *freq2)
{  
  INT4 bin1, bin2;
  REAL8 deltaF;
  REAL8 re1, re2, im1, im2;

  INITSTATUS (status, "CorrelateSingleSFTPair", rcsid);
  ATTATCHSTATUSPTR (status);
  ASSERT (sft1, status, PULSARCROSSCORR_V0_ENULL, PULSARCROSSCORR_V0_MSGENULL);
  ASSERT (sft2, status, PULSARCROSSCORR_V0_ENULL, PULSARCROSSCORR_V0_MSGENULL);
  ASSERT (psd1, status, PULSARCROSSCORR_V0_ENULL, PULSARCROSSCORR_V0_MSGENULL);
  ASSERT (psd2, status, PULSARCROSSCORR_V0_ENULL, PULSARCROSSCORR_V0_MSGENULL);


  /* assume both sfts have the same freq. resolution */
  deltaF = sft1->deltaF;
  bin1 = (INT4)ceil( ((*freq1 - sft1->f0) / (deltaF)) - 0.5);
  bin2 = (INT4)ceil( ((*freq2 - sft2->f0)/ (deltaF)) - 0.5);

  re1 = sft1->data->data[bin1].re;
  im1 = sft1->data->data[bin1].im;
  re2 = sft2->data->data[bin2].re;
  im2 = sft2->data->data[bin2].im;

  out->re = (deltaF * deltaF * sqrt(psd1->data->data[bin1] * psd2->data->data[bin2])) * (re1*re2 + im1*im2);
  out->im = (deltaF * deltaF * sqrt(psd1->data->data[bin1] * psd2->data->data[bin2])) * (re1*im2 - re2*im1);

  DETATCHSTATUSPTR (status);
	
  /* normal exit */	
  RETURN (status);

}




/** Calculate the frequency of the SFT at a given epoch */
void LALGetSignalFrequencyInSFT_v0(LALStatus                *status,
				REAL8                    *out,
				COMPLEX8FrequencySeries  *sft1,
				PulsarDopplerParams      *dopp,
				REAL8Vector              *vel)
{  
  UINT4 k;
  REAL8 alpha, delta;
  REAL8 vDotn, fhat, factor, timeDiff;
  
  INITSTATUS (status, "GetSignalFrequencyInSFT", rcsid);
  ATTATCHSTATUSPTR (status);

  alpha = dopp->Alpha;
  delta = dopp->Delta;


  vDotn = cos(delta) * cos(alpha) * vel->data[0]
    + cos(delta) * sin(alpha)  * vel->data[1]
    + sin(delta) * vel->data[2];

  /* now calculate the intrinsic signal frequency in the SFT */
  /* fhat = f_0 + f_1(t-t0) + f_2(t-t0)^2/2 + ... */

  /* this is the sft reference time  - the pulsar reference time */
  timeDiff = XLALGPSDiff( &(sft1->epoch), &(dopp->refTime));

  fhat = dopp->fkdot[0]; /* initialization */
  factor = 1.0;
  for (k = 1;  k < PULSAR_MAX_SPINS; k++) {
    factor *= timeDiff / k;  
    fhat += dopp->fkdot[k] * factor;
  }

  *out = fhat * (1 + vDotn);  


  DETATCHSTATUSPTR (status);
	
  /* normal exit */	
  RETURN (status);
}



/** Get signal phase at a given epoch */
void LALGetSignalPhaseInSFT_v0(LALStatus               *status,
			    REAL8                   *out,
			    COMPLEX8FrequencySeries *sft1,
			    PulsarDopplerParams     *dopp,
			    REAL8Vector             *pos)
{  
  UINT4 k;
  REAL8 alpha, delta;
  REAL8 rDotn, phihat, factor, timeDiff;
  LIGOTimeGPS ssbt;
  
  INITSTATUS (status, "GetSignalPhaseInSFT", rcsid);
  ATTATCHSTATUSPTR (status);

  alpha = dopp->Alpha;
  delta = dopp->Delta;

  rDotn = cos(delta) * cos(alpha) * pos->data[0]
    + cos(delta) * sin(alpha) * pos->data[1]
    + sin(delta) * pos->data[2];


  /* now calculate the phase of the SFT */
  /* phi(t) = phi_0 + 2pi(f_0 t + 0.5 f_1 t^2) + 2pi (f_0 + f_1 t) r.n/c */
  /* this is an approximation... need to change in the future? */

  /* this is the sft reference time  - the pulsar reference time */
  XLALGPSSetREAL8(&ssbt, XLALGPSGetREAL8(&(sft1->epoch)) + rDotn);
/*  refTime = XLALGPSAdd(&(dopp->refTime), rDotn);*/

  timeDiff = XLALGPSDiff( &ssbt, &(dopp->refTime) );


/*  fhat = dopp->fkdot[0];  initialization */
  phihat = 0.0;

  factor = 1.0;

 for (k = 1;  k < PULSAR_MAX_SPINS; k++) {
    factor *= timeDiff / k;  
/*    fhat += dopp->fkdot[k] * factor;*/
    phihat += dopp->fkdot[k-1] * factor;

  }

    factor *= timeDiff / k;
    phihat += dopp->fkdot[k-1] * factor;


  *out = LAL_TWOPI * ( phihat );


  DETATCHSTATUSPTR (status);
	
  /* normal exit */	
  RETURN (status);
}

void LALCalculateSigmaAlphaSq_v0(LALStatus            *status,
			      REAL8                *out,
			      REAL8                freq1,
			      REAL8                freq2,
			      REAL8FrequencySeries *psd1,
			      REAL8FrequencySeries *psd2)
{			   
  INT8 bin1, bin2;
  REAL8 deltaF;

  INITSTATUS (status, "CalculateSigmaAlphaSq", rcsid);
  ATTATCHSTATUSPTR (status);
  deltaF = psd1->deltaF;

  bin1 = (INT8)ceil( ((freq1 - psd1->f0) / (deltaF)) - 0.5);
  bin2 = (INT8)ceil( ((freq2 - psd2->f0)/ (deltaF)) - 0.5);
  *out = pow(deltaF, 4) * psd1->data->data[bin1] * psd2->data->data[bin2];
  DETATCHSTATUSPTR (status);
	
  /* normal exit */	
  RETURN (status);

}

/** Calculate pair weights (U_alpha) for an average over Psi and cos(iota) **/
void LALCalculateAveUalpha_v0(LALStatus *status,
			COMPLEX16 *out,
			REAL8     *phiI,
			REAL8     *phiJ,
			CrossCorrBeamFn_v0 beamfnsI,
			CrossCorrBeamFn_v0 beamfnsJ,
			REAL8     *sigmasq)
{
  REAL8 deltaPhi;
  REAL8 re, im;
  INITSTATUS (status, "CalculateAveUalpha", rcsid);
  ATTATCHSTATUSPTR (status);

  deltaPhi = *phiI - *phiJ;

  /*calculate G_IJ. In this case, we have <G_IJ> = 0.1*(-exp^(delta phi)) * (aIaJ + bIbJ)*/
  re = 0.1 * cos(deltaPhi) * ((beamfnsI.Fplus_or_a * beamfnsJ.Fplus_or_a) + (beamfnsI.Fcross_or_b * beamfnsJ.Fcross_or_b));
  im = 0.1 * sin(-deltaPhi) * ((beamfnsI.Fplus_or_a * beamfnsJ.Fplus_or_a) + (beamfnsI.Fcross_or_b * beamfnsJ.Fcross_or_b));

  /*calculate Ualpha*/
  out->re = re/(*sigmasq);
  out->im = -im/(*sigmasq);


  DETATCHSTATUSPTR (status);
	
  /* normal exit */	
  RETURN (status);


}
/** Calculate pair weights (U_alpha) for the general case **/
void LALCalculateUalpha_v0(LALStatus *status,
			COMPLEX16 *out,
			CrossCorrAmps_v0 amplitudes,
			REAL8     *phiI,
			REAL8     *phiJ,
			CrossCorrBeamFn_v0 beamfnsI,
			CrossCorrBeamFn_v0 beamfnsJ,
			REAL8     *sigmasq)
{
  REAL8 deltaPhi;
  REAL8 re, im;
  REAL8 FplusIFplusJ, FcrossIFcrossJ, FplusIFcrossJ, FcrossIFplusJ;
	    
  INITSTATUS (status, "CalculateUalpha", rcsid);
  ATTATCHSTATUSPTR (status);

  deltaPhi = *phiI - *phiJ;

  FplusIFplusJ = beamfnsI.Fplus_or_a * (beamfnsJ.Fplus_or_a);
  FcrossIFcrossJ = beamfnsI.Fcross_or_b * (beamfnsJ.Fcross_or_b);
  FplusIFcrossJ = beamfnsI.Fplus_or_a * (beamfnsJ.Fcross_or_b);	
  FcrossIFplusJ = beamfnsI.Fcross_or_b * (beamfnsJ.Fplus_or_a);

  /*calculate G_IJ*/
  re = 0.25 * ( cos(deltaPhi)*((FplusIFplusJ * amplitudes.Aplussq) + (FcrossIFcrossJ * amplitudes.Acrosssq)) 
		-sin(deltaPhi)*((FplusIFcrossJ - FcrossIFplusJ) * amplitudes.AplusAcross) );


  im = 0.25 * (-cos(deltaPhi) * ((FplusIFcrossJ - FcrossIFplusJ)*amplitudes.AplusAcross)
	       -sin(deltaPhi) * ((FplusIFplusJ * amplitudes.Aplussq) + (FcrossIFcrossJ * amplitudes.Acrosssq))); 

  /*calculate Ualpha*/
  out->re = re/(*sigmasq);
  out->im = -im/(*sigmasq);


  DETATCHSTATUSPTR (status);
	
  /* normal exit */	
  RETURN (status);

}



void LALCalculateCrossCorrPower_v0(LALStatus       *status,
				REAL8	        *out,
				COMPLEX16Vector *yalpha,
				COMPLEX16Vector *ualpha)
{
  INT4 i;

  INITSTATUS (status, "CalculateCrossCorrPower", rcsid);
  ATTATCHSTATUSPTR (status);

  ASSERT (out, status, PULSARCROSSCORR_V0_ENULL, PULSARCROSSCORR_V0_MSGENULL);
  ASSERT (yalpha, status, PULSARCROSSCORR_V0_ENULL, PULSARCROSSCORR_V0_MSGENULL);
  ASSERT (ualpha, status, PULSARCROSSCORR_V0_ENULL, PULSARCROSSCORR_V0_MSGENULL);

  *out = 0;
  
  for (i=0; i < (INT4)yalpha->length; i++) {

  *out += 2.0 * ((yalpha->data[i].re * ualpha->data[i].re) - (yalpha->data[i].im * ualpha->data[i].im));


  }

  DETATCHSTATUSPTR (status);
	
  /* normal exit */	
  RETURN (status);

}

void LALNormaliseCrossCorrPower_v0(LALStatus        *status,
				REAL8		 *out,
				COMPLEX16Vector  *ualpha,
				REAL8Vector      *sigmaAlphasq)
{
  INT4 i;
  REAL8 variance = 0;

  INITSTATUS (status, "NormaliseCrossCorrPower", rcsid);
  ATTATCHSTATUSPTR (status);

  ASSERT (ualpha, status, PULSARCROSSCORR_V0_ENULL, PULSARCROSSCORR_V0_MSGENULL);
  ASSERT (sigmaAlphasq, status, PULSARCROSSCORR_V0_ENULL, PULSARCROSSCORR_V0_MSGENULL);


  for (i=0; i < (INT4)ualpha->length; i++) {
	variance += (pow(ualpha->data[i].re, 2) + pow(ualpha->data[i].im, 2)) * sigmaAlphasq->data[i];

  }
  
  variance *= 2.0;
  *out = sqrt(variance);

  DETATCHSTATUSPTR (status);
	
  /* normal exit */	
  RETURN (status);


}


