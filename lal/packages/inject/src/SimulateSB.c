/*************************** <lalVerbatim file="SimulateSBCV">
Author: Sukanta Bose (Adapted from a non-LAL code written by Bruce Allen)
$Id$
************************************* </lalVerbatim> */

/********************************************************** <lalLaTeX>
\subsection{Module \texttt{SimulateSB.c}}
\label{inject:ss:SimulateSB.c}

Simulates whitened time-domain signal in a pair 
of detectors that arises purely from  an isotropic and
unpolarized stochastic background of gravitational radiation with the
desired power spectrum, $\Omega_{\scriptstyle{\rm GW}}(f)$. This module 
will evolve beyond its present funtionality to produce only \texttt{real} 
time-series signal for a pair of interferometric detectors. 


\subsubsection*{Prototypes}
\input{SimulateSBCP}
\idx{LALSimulateSB()}

\subsubsection*{Description}

The frequency domain strains $\widetilde{h}_1(f_i)$ 
and $\widetilde{h}_2(f_j)$ caused by
the stochastic background in two detectors are random variables that have
zero mean and that obey  \cite{inject:Allen:1999}:
  \begin{equation}
    \langle\widetilde{h}_1^*(f_i)\widetilde{h}_1(f_j)\rangle
    = \frac{3H_0^2T}{20\pi^2}\delta_{ij}f_i^{-3}\gamma_{11}(f_i)
    \Omega_{\scriptstyle{\rm GW}}(|f_i|)
  \end{equation}
  and
   \begin{equation}
    \langle\widetilde{h}_2^*(f_i)\widetilde{h}_2(f_j)\rangle
    = \frac{3H_0^2T}{20\pi^2}\delta_{ij}f_i^{-3}\gamma_{22}(f_i)
    \Omega_{\scriptstyle{\rm GW}}(|f_i|)
  \end{equation}
  and
  \begin{equation}
    \langle\widetilde{h}_1^*(f_i)\widetilde{h}_2(f_j)\rangle
    = \frac{3H_0^2T}{20\pi^2}\delta_{ij}f_i^{-3}\gamma_{12}(f_i)
    \Omega_{\scriptstyle{\rm GW}}(|f_i|) \ ,
  \end{equation}
where $\langle\rangle$ denotes ensemble average, $T$ is the time of
observation, and $\gamma_{AB}$ is the overlap reduction function
\cite{inject:Flanagan:1993} of the detector pair comprising detectors $A$ 
and $B$. Above, $\widetilde{h}_1(f_i)$ and 
$\widetilde{h}_2(f_j)$ are the Fourier components of the gravitational strains
$h_1(t)$ and $h_2(t)$ at the two detectors. 

The Fourier components that 
obey the above relations are
  \begin{equation}
    \widetilde{h}_1(f_i) = \sqrt{3H_0^2T \over 40\pi^2}f_i^{-3/2}
    \Omega^{1/2}_{\scriptstyle{\rm GW}}(|f_i|) \sqrt{\gamma_{11}(f_i)}
(x_{1i} + i y_{1i})
    \,
  \end{equation}
  and
  \begin{equation}
    \widetilde{h}_2(f_i) = \widetilde{h}_1(f_i)\frac{\gamma_{12}(f_i)}
{\gamma_{11}(f_i)} +
    \sqrt{3H_0^2T \over 40\pi^2}f_i^{-3/2}
    \Omega^{1/2}_{\scriptstyle{\rm GW}}(|f_i|) 
    \sqrt{\gamma_{22}(f_i)-\frac{\gamma^2_{12}(f_i)}{\gamma_{11}(f_i)}} 
(x_{2i} + i y_{2i})
    \,
  \end{equation}
where $x_{1i}$, $y_{1i}$, $x_{2i}$, and $y_{2i}$ are statistically 
independent real Gaussian random variables, each of zero mean and unit 
variance. 

The routine assumes as inputs the data sample length, temporal spacing,
stochastic background characteristics, detector locations, the appropriate 
representations of the detector response function in each detector, etc.
The (frequency domain) response functions, $\widetilde{R}_1(f_i)$ and
$\widetilde{R}_2(f_i)$  are used to whiten the strains
$\widetilde{h}_1(f_i)$ and 
$\widetilde{h}_2(f_i)$, respectively, to obtain the whitened 
Fourier components:
  \begin{equation}
    \widetilde{o}_1(f_i) = \widetilde{R}_1(f_i)\widetilde{h}_1(f_i)
    \,
  \end{equation}
  and
  \begin{equation}
    \widetilde{o}_2(f_i) = \widetilde{R}_2(f_i)\widetilde{h}_2(f_i)
    \ .
  \end{equation}
To obtain the whitened (real) 
outputs $o_1(t_i)$ and $o_2(t_i)$ in the time domain, the inverse 
Fourier transforms of the above frequency series are taken.

\subsubsection*{Algorithm}

The routine \texttt{LALSSSimStochBGTimeSeries()} produces only \texttt{real} 
time-series signal for a pair of interferometric detectors. It
first inputs the frequency 
series describing the power spectrum of the stochastic background,
$\Omega_{\scriptstyle{\rm GW}}(|f|)$, which the simulated 
signal is required to represent. It also inputs two \texttt{COMPLEX8}
frequency series corresponding, respectively, to the two detector
response functions. As parameters, it takes the two \texttt{LALDetector}
structures corresponding to the two detectors in which the signal is to be 
mimicked. It also takes the time length (given in terms of the number of 
time data samples), the time spacing, a seed (for generating random
numbers), and a couple of \texttt{LALUnit} structures for specifying the
units of the two time-series signals that the routine outputs.

Using the specified power
spectrum for the stochastic background, and a random number generator (of
zero mean, unit variance Gaussian distributions), the routine produces 
$\widetilde{h}_1(f_i)$ and $\widetilde{h}_2(f_i)$. The 
response functions of the two detectors are then used to whiten the two 
strains in the Fourier domain. Their inverse transform is then taken to obtain
at each detector the whitened simulated signal in the time domain.

\subsubsection*{Uses}

\begin{verbatim}
LALStochasticOmegaGW()
LALReverseRealFFT()
LALNormalDeviates()
\end{verbatim}

\subsubsection*{Notes}

\begin{itemize}
\item This routine does not yet support non-zero heterodyning frequencies.
\end{itemize}

\vfill{\footnotesize\input{SimulateSBCV}}

******************************************************* </lalLaTeX> */ 

/**************************** <lalLaTeX file="SimulateSBCB">
\bibitem{inject:Allen:1999}
  B.~Allen and J.~D.~Romano, ``Detecting a stochastic background of
  gravitational radiation: Signal processing strategies and
  sensitivities''
  Phys.\ Rev.\ D {\bf 59}, 102001 (1999);
  \href{http://www.arXiv.org/abs/gr-qc/9710117}{gr-qc/9710117}
  \bibitem{inject:Flanagan:1993}
  E.~Flanagan, ``The sensitivity of the laser interferometer gravitational 
  wave observatory (LIGO) to a stochastic background, and its dependence on
  the detector orientations''
  Phys.\ Rev.\ D {\bf 48}, 2389 (1993);
  \href{http://www.arXiv.org/abs/astro-ph/9305029}{astro-ph/9305029}
******************************************************* </lalLaTeX> */

#include <math.h>
#include <string.h>
#include <stdio.h>
#include <lal/LALStdlib.h>
#include <lal/LALConstants.h>
#include <lal/StochasticCrossCorrelation.h> 
#include <lal/AVFactories.h>
#include <lal/RealFFT.h>
#include <lal/ComplexFFT.h>
#include <lal/Units.h>
#include <lal/Random.h>
#include <lal/SimulateSB.h>
#include <lal/DetectorSite.h>

NRCSID (SIMULATESBC, "$Id$");

/* <lalVerbatim file="SimulateSBCP"> */
void
LALSSSimStochBGTimeSeries( LALStatus                    *status,
			   SSSimStochBGOutput           *output,
			   SSSimStochBGInput            *input,
			   SSSimStochBGParams           *params 
	       )
     /* </lalVerbatim> */
{
  /* parameters */
  UINT4             length;   /* (time) length of output vector data samples */
  UINT4             freqlen;
  REAL8             deltaT;   /* time spacing */
  REAL8             f0;       /* start frequency */
  
  /* counters */ 
  UINT4             i;
  
  /* other variables used */
  REAL8             deltaF;
  RandomParams     *randParams=NULL;
  
  /* vector for storing random numbers */ 
  REAL4Vector      *gaussdevsX1=NULL;
  REAL4Vector      *gaussdevsY1=NULL;
  REAL4Vector      *gaussdevsX2=NULL;
  REAL4Vector      *gaussdevsY2=NULL;
  
  /* LAL structure needed as input/output for computing overlap 
     reduction function */
  LALDetectorPair                    detectors;
  REAL4FrequencySeries               overlap11,overlap12,overlap22;
  OverlapReductionFunctionParameters ORFparameters;
  
  
  /* IFO output counts in freq domain : */
  COMPLEX8Vector   *ccounts[2]={NULL,NULL};
  COMPLEX8Vector   *ccountsTmp[2]={NULL,NULL};
  
  /* Plan for reverse FFTs */ 
  RealFFTPlan      *invPlan=NULL;  
  
  /* initialize status pointer */
  INITSTATUS(status, "LALSSSimStochBGTimeSeries", SIMULATESBC);
  ATTATCHSTATUSPTR(status);
  
  
  /*
   *
   *ERROR CHECKING
   *
   */
  
  /***** check input/output structures exist *****/

  /* output structure */
  ASSERT(output !=NULL, status, 
         SIMULATESBH_ENULLP,
         SIMULATESBH_MSGENULLP);
  ASSERT(output->SSimStochBG1->data !=NULL, status, 
         SIMULATESBH_ENULLP,
         SIMULATESBH_MSGENULLP);
  ASSERT(output->SSimStochBG2->data !=NULL, status, 
         SIMULATESBH_ENULLP,
         SIMULATESBH_MSGENULLP);
  ASSERT(output->SSimStochBG1->data->data !=NULL, status, 
         SIMULATESBH_ENULLP,
         SIMULATESBH_MSGENULLP);
  ASSERT(output->SSimStochBG2->data->data !=NULL, status, 
         SIMULATESBH_ENULLP,
         SIMULATESBH_MSGENULLP);
  
  /* input structure */
  ASSERT(input != NULL, status, 
         SIMULATESBH_ENULLP,
         SIMULATESBH_MSGENULLP);
  
  /* omega member of input */
  ASSERT(input->omegaGW != NULL, status, 
         SIMULATESBH_ENULLP,
         SIMULATESBH_MSGENULLP);
  
  /* First detector's complex response (whitening filter) part of input */
  ASSERT(input->whiteningFilter1 != NULL, status, 
         SIMULATESBH_ENULLP,
         SIMULATESBH_MSGENULLP);

  /* Second detector's complex response (whitening filter) part of input */
  ASSERT(input->whiteningFilter2 != NULL, status, 
         SIMULATESBH_ENULLP,
         SIMULATESBH_MSGENULLP);

  /* data member of omega */
  ASSERT(input->omegaGW->data != NULL, status, 
         SIMULATESBH_ENULLP,
         SIMULATESBH_MSGENULLP);

  /* data member of first detector's response (whitening filter) */
  ASSERT(input->whiteningFilter1->data != NULL, status, 
         SIMULATESBH_ENULLP,
         SIMULATESBH_MSGENULLP);

  /* data member of second detector's response (whitening filter) */
  ASSERT(input->whiteningFilter2->data != NULL, status, 
         SIMULATESBH_ENULLP,
         SIMULATESBH_MSGENULLP);

  /* data-data member of first detector's response (whitening filter) */
  ASSERT(input->whiteningFilter1->data->data != NULL, status, 
         SIMULATESBH_ENULLP,
         SIMULATESBH_MSGENULLP);

  /* data-data member of second detector's response (whitening filter) */
  ASSERT(input->whiteningFilter2->data->data != NULL, status, 
         SIMULATESBH_ENULLP,
         SIMULATESBH_MSGENULLP);

  /************* check parameter structures ***********/

  /*No. of discrete time samples (length) is non-zero in each detector output*/
  ASSERT(params->length > 0, status, 
         SIMULATESBH_ENONPOSLEN,
         SIMULATESBH_MSGENONPOSLEN);

  /* time-interval between successive samples is non-zero in each output */
  ASSERT(params->deltaT > 0, status, 
         SIMULATESBH_ENONPOSLEN,
         SIMULATESBH_MSGENONPOSLEN);

  /************* done with null pointers *****************/
  
  
  /**** check for legality ****/
  
  /* start frequency must not be negative */
  f0 = input->omegaGW->f0;
  if (f0 < 0)
    {
      ABORT( status,
	     SIMULATESBH_ENEGFMIN,
	     SIMULATESBH_MSGENEGFMIN );
    }
  
  
  /** check for mismatches **/
  /* frequency length = length/2 +1  */
  length = params->length;
  freqlen = length/2 +1;

  if (input->omegaGW->data->length != (length/2 +1)) 
    {
      ABORT(status,
	    SIMULATESBH_EMMLEN,
	    SIMULATESBH_MSGEMMLEN);
    }
  if (input->whiteningFilter1->data->length != (length/2 +1)) 
    {
      ABORT(status,
	    SIMULATESBH_EMMLEN,
	    SIMULATESBH_MSGEMMLEN);
    }
  if (input->whiteningFilter2->data->length != (length/2 +1)) 
    {
      ABORT(status,
	    SIMULATESBH_EMMLEN,
	    SIMULATESBH_MSGEMMLEN);
    }
  if (input->whiteningFilter1->f0 != f0) 
    {
      ABORT(status,
	    SIMULATESBH_EMMFMIN,
	    SIMULATESBH_MSGEMMFMIN);
    }
  if (input->whiteningFilter2->f0 != f0) 
    {
      ABORT(status,
	    SIMULATESBH_EMMFMIN,
	    SIMULATESBH_MSGEMMFMIN);
    }

  /* frequency spacing */
  deltaT = params->deltaT;
  deltaF = 1/(deltaT*length);
  if (input->whiteningFilter1->deltaF != deltaF) 
    {
      ABORT(status,
	    SIMULATESBH_EMMDELTAF,
	    SIMULATESBH_MSGEMMDELTAF);
    }
  if (input->whiteningFilter2->deltaF != deltaF) 
    {
      ABORT(status,
	    SIMULATESBH_EMMDELTAF,
	    SIMULATESBH_MSGEMMDELTAF);
    }
  
  
  /*
   *
   *EVERYHTING OKAY HERE 
   *
   */
  

  /******** create fft plans and workspace vectors *****/

  LALCreateReverseRealFFTPlan(status->statusPtr,&invPlan,length,0);
  CHECKSTATUSPTR( status ); 

  LALSCreateVector( status->statusPtr, 
		    &gaussdevsX1, freqlen ); 
  BEGINFAIL( status )
    {
      TRY( LALDestroyRealFFTPlan( status->statusPtr, 
				  &invPlan ), status );
    }
  ENDFAIL( status );
  

  LALSCreateVector( status->statusPtr, 
		    &gaussdevsY1, freqlen ); 
  BEGINFAIL( status )
    {
      TRY( LALSDestroyVector( status->statusPtr, 
			      &gaussdevsX1), status ); 
      TRY( LALDestroyRealFFTPlan( status->statusPtr, 
				  &invPlan ), status );
    }
  ENDFAIL( status );
  
  LALSCreateVector( status->statusPtr, 
		    &gaussdevsX2, freqlen ); 
  BEGINFAIL( status )
    {
      TRY( LALSDestroyVector( status->statusPtr, 
			      &gaussdevsY1), status ); 
      TRY( LALSDestroyVector( status->statusPtr, 
			      &gaussdevsX1), status ); 
      TRY( LALDestroyRealFFTPlan( status->statusPtr, 
				  &invPlan ), status );
    }
  ENDFAIL( status );
  
  LALSCreateVector( status->statusPtr, 
		    &gaussdevsY2, freqlen ); 
  BEGINFAIL( status )
    {
      TRY( LALSDestroyVector( status->statusPtr, 
			      &gaussdevsX2), status ); 
      TRY( LALSDestroyVector( status->statusPtr, 
			      &gaussdevsY1), status ); 
      TRY( LALSDestroyVector( status->statusPtr, 
			      &gaussdevsX1), status ); 
      TRY( LALDestroyRealFFTPlan( status->statusPtr, 
				  &invPlan ), status );
    }
  ENDFAIL( status );
  
  /* create parameters for generating random numbers from seed */
  LALCreateRandomParams( status->statusPtr, 
			 &randParams, params->seed ); 
  BEGINFAIL( status )
    {
      TRY( LALSDestroyVector( status->statusPtr, 
			      &gaussdevsY2), status ); 
      TRY( LALSDestroyVector( status->statusPtr, 
			      &gaussdevsX2), status ); 
      TRY( LALSDestroyVector( status->statusPtr, 
			      &gaussdevsY1), status ); 
      TRY( LALSDestroyVector( status->statusPtr, 
			      &gaussdevsX1), status ); 
      TRY( LALDestroyRealFFTPlan( status->statusPtr, 
				  &invPlan ), status );
    }
  ENDFAIL( status );
  
  LALCCreateVector(status->statusPtr, &ccounts[0],freqlen);
  BEGINFAIL( status )
    {
      TRY( LALDestroyRandomParams( status->statusPtr, 
				   &randParams), status ); 
      TRY( LALSDestroyVector( status->statusPtr, 
			      &gaussdevsY2), status ); 
      TRY( LALSDestroyVector( status->statusPtr, 
			      &gaussdevsX2), status ); 
      TRY( LALSDestroyVector( status->statusPtr, 
			      &gaussdevsY1), status ); 
      TRY( LALSDestroyVector( status->statusPtr, 
			      &gaussdevsX1), status ); 
      TRY( LALDestroyRealFFTPlan( status->statusPtr, 
				  &invPlan ), status );
    }  
  ENDFAIL( status );
  
  LALCCreateVector(status->statusPtr, &ccounts[1],freqlen);
  BEGINFAIL( status )
    {
      TRY( LALCDestroyVector(status->statusPtr, &ccounts[0]), status);
      TRY( LALDestroyRandomParams( status->statusPtr, 
				   &randParams), status ); 
      TRY( LALSDestroyVector( status->statusPtr, 
			      &gaussdevsY2), status ); 
      TRY( LALSDestroyVector( status->statusPtr, 
			      &gaussdevsX2), status ); 
      TRY( LALSDestroyVector( status->statusPtr, 
			      &gaussdevsY1), status ); 
      TRY( LALSDestroyVector( status->statusPtr, 
			      &gaussdevsX1), status ); 
      TRY( LALDestroyRealFFTPlan( status->statusPtr, 
				  &invPlan ), status );
    }  
  ENDFAIL( status );
  
  LALCCreateVector(status->statusPtr, &ccountsTmp[0],freqlen);
  BEGINFAIL( status )
    {
      TRY( LALCDestroyVector(status->statusPtr, &ccounts[1]), status);
      TRY( LALCDestroyVector(status->statusPtr, &ccounts[0]), status);
      TRY( LALDestroyRandomParams( status->statusPtr, 
				   &randParams), status ); 
      TRY( LALSDestroyVector( status->statusPtr, 
			      &gaussdevsY2), status ); 
      TRY( LALSDestroyVector( status->statusPtr, 
			      &gaussdevsX2), status ); 
      TRY( LALSDestroyVector( status->statusPtr, 
			      &gaussdevsY1), status ); 
      TRY( LALSDestroyVector( status->statusPtr, 
			      &gaussdevsX1), status ); 
      TRY( LALDestroyRealFFTPlan( status->statusPtr, 
				  &invPlan ), status );
    }  
  ENDFAIL( status );
  
  LALCCreateVector(status->statusPtr, &ccountsTmp[1],freqlen);
  BEGINFAIL( status )
    {
      TRY( LALCDestroyVector(status->statusPtr, &ccountsTmp[0]), status);
      TRY( LALCDestroyVector(status->statusPtr, &ccounts[1]), status);
      TRY( LALCDestroyVector(status->statusPtr, &ccounts[0]), status);
      TRY( LALDestroyRandomParams( status->statusPtr, 
				   &randParams), status ); 
      TRY( LALSDestroyVector( status->statusPtr, 
			      &gaussdevsY2), status ); 
      TRY( LALSDestroyVector( status->statusPtr, 
			      &gaussdevsX2), status ); 
      TRY( LALSDestroyVector( status->statusPtr, 
			      &gaussdevsY1), status ); 
      TRY( LALSDestroyVector( status->statusPtr, 
			      &gaussdevsX1), status ); 
      TRY( LALDestroyRealFFTPlan( status->statusPtr, 
				  &invPlan ), status );
    }  
  ENDFAIL( status );

  overlap11.data = NULL;
  LALSCreateVector(status->statusPtr, &(overlap11.data),freqlen);
  BEGINFAIL( status )
    {
      TRY( LALCDestroyVector(status->statusPtr, &ccountsTmp[1]), status);
      TRY( LALCDestroyVector(status->statusPtr, &ccountsTmp[0]), status);
      TRY( LALCDestroyVector(status->statusPtr, &ccounts[1]), status);
      TRY( LALCDestroyVector(status->statusPtr, &ccounts[0]), status);
      TRY( LALDestroyRandomParams( status->statusPtr, 
				   &randParams), status ); 
      TRY( LALSDestroyVector( status->statusPtr, 
			      &gaussdevsY2), status ); 
      TRY( LALSDestroyVector( status->statusPtr, 
			      &gaussdevsX2), status ); 
      TRY( LALSDestroyVector( status->statusPtr, 
			      &gaussdevsY1), status ); 
      TRY( LALSDestroyVector( status->statusPtr, 
			      &gaussdevsX1), status ); 
      TRY( LALDestroyRealFFTPlan( status->statusPtr, 
				  &invPlan ), status );
    }  
  ENDFAIL( status );
    
  overlap12.data = NULL;
  LALSCreateVector(status->statusPtr, &(overlap12.data),freqlen);
  BEGINFAIL( status )
    {
      TRY( LALSDestroyVector(status->statusPtr, &(overlap11.data)), status);
      TRY( LALCDestroyVector(status->statusPtr, &ccountsTmp[1]), status);
      TRY( LALCDestroyVector(status->statusPtr, &ccountsTmp[0]), status);
      TRY( LALCDestroyVector(status->statusPtr, &ccounts[1]), status);
      TRY( LALCDestroyVector(status->statusPtr, &ccounts[0]), status);
      TRY( LALDestroyRandomParams( status->statusPtr, 
				   &randParams), status ); 
      TRY( LALSDestroyVector( status->statusPtr, 
			      &gaussdevsY2), status ); 
      TRY( LALSDestroyVector( status->statusPtr, 
			      &gaussdevsX2), status ); 
      TRY( LALSDestroyVector( status->statusPtr, 
			      &gaussdevsY1), status ); 
      TRY( LALSDestroyVector( status->statusPtr, 
			      &gaussdevsX1), status ); 
      TRY( LALDestroyRealFFTPlan( status->statusPtr, 
				  &invPlan ), status );
    }  
  ENDFAIL( status );
    
  overlap22.data = NULL;
  LALSCreateVector(status->statusPtr, &(overlap22.data),freqlen);
  BEGINFAIL( status )
    {
      TRY( LALSDestroyVector(status->statusPtr, &(overlap12.data)), status);
      TRY( LALSDestroyVector(status->statusPtr, &(overlap11.data)), status);
      TRY( LALCDestroyVector(status->statusPtr, &ccountsTmp[1]), status);
      TRY( LALCDestroyVector(status->statusPtr, &ccountsTmp[0]), status);
      TRY( LALCDestroyVector(status->statusPtr, &ccounts[1]), status);
      TRY( LALCDestroyVector(status->statusPtr, &ccounts[0]), status);
      TRY( LALDestroyRandomParams( status->statusPtr, 
				   &randParams), status ); 
      TRY( LALSDestroyVector( status->statusPtr, 
			      &gaussdevsY2), status ); 
      TRY( LALSDestroyVector( status->statusPtr, 
			      &gaussdevsX2), status ); 
      TRY( LALSDestroyVector( status->statusPtr, 
			      &gaussdevsY1), status ); 
      TRY( LALSDestroyVector( status->statusPtr, 
			      &gaussdevsX1), status ); 
      TRY( LALDestroyRealFFTPlan( status->statusPtr, 
				  &invPlan ), status );
    }  
  ENDFAIL( status );
    
  /* create random numbers from parameters */
  LALNormalDeviates( status->statusPtr, 
		     gaussdevsX1, randParams ); 
  CHECKSTATUSPTR( status);

  LALDestroyRandomParams(status->statusPtr,&randParams);

  LALCreateRandomParams(status->statusPtr,&randParams, params->seed +1);
  
  LALNormalDeviates( status->statusPtr, 
		     gaussdevsY1, randParams ); 
  CHECKSTATUSPTR( status);
  
  LALDestroyRandomParams(status->statusPtr,&randParams);
  
  LALCreateRandomParams(status->statusPtr,&randParams, params->seed +2);
  
  LALNormalDeviates( status->statusPtr, 
		     gaussdevsX2, randParams ); 
  CHECKSTATUSPTR( status);
 
  LALDestroyRandomParams(status->statusPtr,&randParams);
  
  LALCreateRandomParams(status->statusPtr,&randParams, params->seed +3);
  
  LALNormalDeviates( status->statusPtr, 
		     gaussdevsY2, randParams ); 
  CHECKSTATUSPTR( status);
  
  LALDestroyRandomParams(status->statusPtr,&randParams);

  ORFparameters.length   = length/2 + 1;
  ORFparameters.f0       = f0;
  ORFparameters.deltaF   = deltaF;
  detectors.detectorOne  = params->detectorOne;
  detectors.detectorTwo  = params->detectorOne;

  LALOverlapReductionFunction( status->statusPtr, &overlap11, 
			       &detectors, &ORFparameters);
  CHECKSTATUSPTR( status);

  detectors.detectorOne  = params->detectorOne;
  detectors.detectorTwo  = params->detectorTwo;

  LALOverlapReductionFunction( status->statusPtr, &overlap12, 
			       &detectors, &ORFparameters);
  CHECKSTATUSPTR( status);

  detectors.detectorOne  = params->detectorTwo;
  detectors.detectorTwo  = params->detectorTwo;

  LALOverlapReductionFunction( status->statusPtr, &overlap22, 
			       &detectors, &ORFparameters);
  CHECKSTATUSPTR( status);
  
  if (f0 == 0) 
    {
      REAL4    gamma11,gamma12,gamma22;
      REAL4    omega;
      REAL8    freq;
      REAL8    factor;
      REAL8    factor2,factor3;
      COMPLEX8 wFilter1;
      COMPLEX8 wFilter2;
       
      /* loop over frequencies; will do DC and Nyquist below */
      for (i = 1; i < freqlen; ++i)
	{
	  freq  = i*deltaF;	  

	  gamma11 = overlap11.data->data[i];
	  gamma12 = overlap12.data->data[i];
	  gamma22 = overlap22.data->data[i];

	  omega = input->omegaGW->data->data[i];
	  
	  factor = deltaF * sqrt(3.0L * length * deltaT * omega / 
				 (40.0L *freq*freq*freq)
				 )* LAL_H0FAC_SI / LAL_PI;
	  factor2 = sqrt(gamma22-gamma12*gamma12/gamma11)*factor;
	  factor3 = sqrt(gamma11)*factor;

	  wFilter1 = input->whiteningFilter1->data->data[i];
	  wFilter2 = input->whiteningFilter2->data->data[i];
	  
	  ccountsTmp[0]->data[i].re=factor3*gaussdevsX1->data[i];
	  ccountsTmp[0]->data[i].im=factor3*gaussdevsY1->data[i];
	  ccountsTmp[1]->data[i].re=ccountsTmp[0]->data[i].re*gamma12/gamma11+factor2*gaussdevsX2->data[i];
	  ccountsTmp[1]->data[i].im=ccountsTmp[0]->data[i].im*gamma12/gamma11+factor2*gaussdevsY2->data[i];
	  
	  ccounts[0]->data[i].re = wFilter1.re * ccountsTmp[0]->data[i].re -
	    wFilter1.im * ccountsTmp[0]->data[i].im;
	  ccounts[0]->data[i].im = wFilter1.re * ccountsTmp[0]->data[i].im +
	    wFilter1.im * ccountsTmp[0]->data[i].re;
	  ccounts[1]->data[i].re = wFilter2.re * ccountsTmp[1]->data[i].re -
	    wFilter2.im * ccountsTmp[1]->data[i].im;
	  ccounts[1]->data[i].im = wFilter2.re * ccountsTmp[1]->data[i].im +
	    wFilter2.im * ccountsTmp[1]->data[i].re;
	}
      
      /* Set DC, Nyquist (imaginary) components to zero */
      for (i=0;i<2;++i)
	{
	  ccounts[i]->data[0].re=0.0;
	  ccounts[i]->data[0].im=0.0;
	  ccountsTmp[i]->data[length/2].im=0.0;
	}
      
      /* Compute the whitened Nyquist (real) component */
      gamma11 = overlap11.data->data[length/2];
      gamma12 = overlap12.data->data[length/2];
      gamma22 = overlap22.data->data[length/2];
      
      omega = input->omegaGW->data->data[length/2];
      freq = deltaF*length/2;
      
      wFilter1 = input->whiteningFilter1->data->data[length/2];
      wFilter2 = input->whiteningFilter2->data->data[length/2];
      
      /* check that whitening filter is real in time domain */
      if (wFilter1.im != 0) 
	{
	  ABORT(status,
		SIMULATESBH_ECOMPTIME,
		SIMULATESBH_MSGECOMPTIME);
	};
      if (wFilter2.im != 0) 
	{
	  ABORT(status,
		SIMULATESBH_ECOMPTIME,
		SIMULATESBH_MSGECOMPTIME);
	};
      
      factor = deltaF * sqrt(3.0L * length * deltaT * omega / 
			     (40.0L *freq*freq*freq)
			     )* LAL_H0FAC_SI / LAL_PI;
      factor2 = sqrt(gamma22-gamma12*gamma12/gamma11)*factor;
      factor3 = sqrt(gamma11)*factor;
      
      ccountsTmp[0]->data[length/2].re=factor3*gaussdevsX1->data[length/2];
      
      ccountsTmp[1]->data[length/2].re=
	(ccountsTmp[0]->data[length/2].re*gamma12/gamma11 + 
	 factor2*gaussdevsX2->data[length/2]);
      
      ccounts[0]->data[length/2].re = 
	(wFilter1.re * ccountsTmp[0]->data[length/2].re - 
	 wFilter1.im * ccountsTmp[0]->data[length/2].im);
      ccounts[0]->data[length/2].im = 0;
      
      
      ccounts[1]->data[length/2].re = 
	(wFilter2.re * ccountsTmp[1]->data[length/2].re - 
	 wFilter2.im * ccountsTmp[1]->data[length/2].im);
      ccounts[1]->data[length/2].im = 0;
      
      LALSDestroyVector(status->statusPtr, &(overlap11.data));
      LALSDestroyVector(status->statusPtr, &(overlap12.data));      
      LALSDestroyVector(status->statusPtr, &(overlap22.data));
      
      LALSDestroyVector(status->statusPtr, &gaussdevsX1);
      LALSDestroyVector(status->statusPtr, &gaussdevsY1);
      LALSDestroyVector(status->statusPtr, &gaussdevsX2);
      LALSDestroyVector(status->statusPtr, &gaussdevsY2);
      
      /*
       * 
       * assign parameters and data to output 
       *
       */
      
      
      /*ReverseFFT from freq to time domain & get output (no detector noise)*/ 
      LALReverseRealFFT(status->statusPtr,output->SSimStochBG1->data,
			ccounts[0],invPlan); 
      LALReverseRealFFT(status->statusPtr,output->SSimStochBG2->data,
			ccounts[1],invPlan); 
      
      LALDestroyRealFFTPlan(status->statusPtr,&invPlan);
  
      for (i=0;i<2;i++){
	LALCDestroyVector(status->statusPtr, &ccountsTmp[i]);
	LALCDestroyVector(status->statusPtr, &ccounts[i]);
      }
      
      /*
       * 
       * assign parameters and data to output 
       *
       */
      
      output->SSimStochBG1->f0                   = f0;
      output->SSimStochBG1->deltaT               = deltaT;
      output->SSimStochBG1->epoch.gpsSeconds     = 0;
      output->SSimStochBG1->epoch.gpsNanoSeconds = 0;
      output->SSimStochBG1->sampleUnits          = params->SSimStochBGTimeSeries1Unit;
      strncpy( output->SSimStochBG1->name, 
	       "Whitened-SimulatedSBOne", LALNameLength );

      output->SSimStochBG2->f0                   = f0;
      output->SSimStochBG2->deltaT               = deltaT;
      output->SSimStochBG2->epoch.gpsSeconds     = 0;
      output->SSimStochBG2->epoch.gpsNanoSeconds = 0;
      output->SSimStochBG2->sampleUnits          = params->SSimStochBGTimeSeries2Unit;
      strncpy( output->SSimStochBG2->name, 
	       "Whitened-SimulatedSBTwo", LALNameLength );
    } /* if (f0 == 0) */
  else
    {
      
      /*****This abort should be replaced with the correct *****/
      /***** non-zero heterodyne frequency procedure******/
      ABORT(status, SIMULATESBH_ENOTYETHETERO,
	    SIMULATESBH_MSGENOTYETHETERO); 
    }
  
  /* clean up and exit */

  DETATCHSTATUSPTR(status);
  RETURN(status);
  
}
