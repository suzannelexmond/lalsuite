/****************************** <lalVerbatim file="StackMetricTestCV">
Author: Creighton, T. D.
$Id$
**************************************************** </lalVerbatim> */

/********************************************************** <lalLaTeX>

\subsection{Program \texttt{StackMetricTest.c}}
\label{ss:StackMetricTest.c}

Computes the parameter space metric for a coherent pulsar search.

\subsubsection*{Usage}
\begin{verbatim}
StackMetricTest [-p n dt t0] [-l lat lon] [-d debuglevel]
                [ra dec f0 [f1 [...]]]
\end{verbatim}

\subsubsection*{Description}

This test program computes the stack search metric for a particular
location in parameter space.  The following option flags are accepted:
\begin{itemize}
\item[\texttt{-p}] Sets the search parameters: the number of stacks
\verb@n@, the length of each stack \verb@dt@ (in seconds), and the
start time of the first stack \verb@t0@ (in seconds of GPS time).
\item[\texttt{-l}] Sets the detector latitude to \verb@lat@ (in
radians north from the equator) and longitude to \verb@lon@ (in
radians east of the prime meridian).
\item[\texttt{-d}] Sets the debug level to \verb@debuglevel@.
\end{itemize}
Once any of the above options are processed, any remaining
command-line options are taken to be the parameter-space location of
the source: its right ascension \verb@ra@ (in radians), its
declination \verb@dec@ (in radians), its frequency \verb@f0@ (in Hz),
and zero or more spindown parameters \verb@f@$k$ (in $\mathrm{Hz}^k$),
all evaluated at the start time of the search \verb@t0@.  If any (or
all) of the command-line arguments are missing, they will be set from
\verb@#define@d defaults.

\subsubsection*{Exit codes}
****************************************** </lalLaTeX><lalErrTable> */
#define STACKMETRICTESTC_ENORM 0
#define STACKMETRICTESTC_ESUB  1
#define STACKMETRICTESTC_EARG  2
#define STACKMETRICTESTC_EVAL  3

#define STACKMETRICTESTC_MSGENORM "Normal exit"
#define STACKMETRICTESTC_MSGESUB  "Subroutine failed"
#define STACKMETRICTESTC_MSGEARG  "Error parsing arguments"
#define STACKMETRICTESTC_MSGEVAL  "Input argument out of valid range"
/******************************************** </lalErrTable><lalLaTeX>

\subsubsection*{Algorithm}

\subsubsection*{Uses}
\begin{verbatim}
lalDebugLevel
LALPrintError()                 LALCheckMemoryLeaks()
LALMalloc()                     LALFree()
LALDCreateVector()              LALDDestroyVector()
LALStackMetric()                LALProjectMetric()
LALTBaryPtolemaic()             LALDTBaryPtolemaic()
LALTSpin()                      LALDTSpin()
LALDTComp()
LALGetEarthTimes()
\end{verbatim}

\subsubsection*{Notes}

\vfill{\footnotesize\input{StackMetricTestCV}}

******************************************************* </lalLaTeX> */

#include <math.h>
#include <stdlib.h>
#include <lal/LALStdlib.h>
#include <lal/LALConstants.h>
#include <lal/AVFactories.h>
#include <lal/StackMetric.h>
#include <lal/PulsarTimes.h>

NRCSID(STACKMETRICTESTC,"$Id$");

/* Default parameter settings. */
int lalDebugLevel=0;
#define NSTACKS 1
#define STACKLENGTH 100000.0  /* arbitrary */
#define STARTTIME 0.0         /* arbitrary */
#define LATITUDE 0.91188      /* GEO600 location */
#define LONGITUDE 0.17142     /* GEO600 location */
#define RIGHTASCENSION 5.0309 /* a known pulsar */
#define DECLINATION 0.27925   /* a known pulsar */
#define FREQUENCY 1000.0      /* arbitrary */

/* Usage format string. */
#define USAGE "Usage: %s [-p n dt t0] [-l lat lon] [-d debuglevel]\n\
                       [ra dec f0 [f1 [...]]]\n"

/* Input error checking: Some accepted parameter ranges. */
#define NMAX  10000 /* 1 <= number of stacks <= NMAX */
#define DTMAX  3e10 /* 1/f_0 < stack length <= DTMAX */
#define F0MAX  1e4  /* 0 < f_0 <= FOMAX */
#define TAUMIN 1e4  /* |f_k| <= TAUMIN^(-k) */
/* |latitude| and |declination| are <= LAL_PI,
   |longitude| and |right ascension| are <= LAL_TWOPI */

/* Macros for printing errors and testing subroutines. */
#define ERROR( code, msg, statement )                                \
do                                                                   \
if ( lalDebugLevel & LALERROR )                                      \
{                                                                    \
  LALPrintError( "Error[0] %d: program %s, file %s, line %d, %s\n"   \
		 "        %s %s\n", (code), *argv, __FILE__,         \
		 __LINE__, STACKMETRICTESTC, statement ? statement : \
                 "", (msg) );                                        \
}                                                                    \
while (0)

#define INFO( statement )                                            \
do                                                                   \
if ( lalDebugLevel & LALINFO )                                       \
{                                                                    \
  LALPrintError( "Info[0]: program %s, file %s, line %d, %s\n"       \
		 "        %s\n", *argv, __FILE__, __LINE__,          \
		 STACKMETRICTESTC, (statement) );                    \
}                                                                    \
while (0)

#define SUB( func, statusptr )                                       \
do                                                                   \
if ( (func), (statusptr)->statusCode )                               \
{                                                                    \
  ERROR( STACKMETRICTESTC_ESUB, STACKMETRICTESTC_MSGESUB,            \
         "Function call \"" #func "\" failed:" );                    \
  return STACKMETRICTESTC_ESUB;                                      \
}                                                                    \
while (0)

#define CHECKVAL( val, lower, upper )                                \
do                                                                   \
if ( ( (val) <= (lower) ) || ( (val) > (upper) ) )                   \
{                                                                    \
  ERROR( STACKMETRICTESTC_EVAL, STACKMETRICTESTC_MSGESUB,            \
         "Value of " #val " out of range:" );                        \
  LALPrintError( #val " = %f, range = (%f,%f]\n", (REAL8)(val),      \
                 (REAL8)(lower), (REAL8)(upper) );                   \
  return STACKMETRICTESTC_EVAL;                                      \
}                                                                    \
while (0)

/* A global pointer for debugging. */
#ifndef NDEBUG
char *lalWatch;
#endif

int
main(int argc, char **argv)
{
  static LALStatus stat;
  INT4  arg;
  UINT4 i, j, k;
  UINT4 nSpin = 0;
  UINT4 nSky = 2;
  UINT4 n = NSTACKS;
  REAL8 dt = STACKLENGTH;
  REAL8 t0 = STARTTIME;
  REAL8 lat = LATITUDE;
  REAL8 lon = LONGITUDE;
  REAL8 ra = RIGHTASCENSION;
  REAL8 dec = DECLINATION;
  REAL8 f0 = FREQUENCY;
  REAL8 *spin = NULL;
  REAL8Vector *lambda = NULL;
  REAL8Vector *metric = NULL;
  static LIGOTimeGPS start;
  static PulsarTimesParamStruc spinParams;
  static PulsarTimesParamStruc baryParams;
  static PulsarTimesParamStruc compParams;
  static MetricParamStruc params;

  /* Parse argument list.  arg stores the current position. */
  arg = 1;
  while ( arg < argc ) {
    /* Parse search parameter option. */
    if ( !strcmp( argv[arg], "-p" ) ) {
      if ( argc > arg + 3 ) {
	arg++;
	n = atoi( argv[arg++] );
	dt = atof( argv[arg++] );
	t0 = atof( argv[arg++] );
      }else{
	ERROR( STACKMETRICTESTC_EARG, STACKMETRICTESTC_MSGEARG, 0 );
        LALPrintError( USAGE, *argv );
        return STACKMETRICTESTC_EARG;
      }
    }
    /* Parse detector position option. */
    else if ( !strcmp( argv[arg], "-l" ) ) {
      if ( argc > arg + 2 ) {
	arg++;
	lat = atof( argv[arg++] );
	lon = atof( argv[arg++] );
      }else{
	ERROR( STACKMETRICTESTC_EARG, STACKMETRICTESTC_MSGEARG, 0 );
        LALPrintError( USAGE, *argv );
        return STACKMETRICTESTC_EARG;
      }
    }
    /* Parse debug level option. */
    else if ( !strcmp( argv[arg], "-d" ) ) {
      if ( argc > arg + 1 ) {
	arg++;
	lalDebugLevel = atoi( argv[arg++] );
      }else{
	ERROR( STACKMETRICTESTC_EARG, STACKMETRICTESTC_MSGEARG, 0 );
        LALPrintError( USAGE, *argv );
        return STACKMETRICTESTC_EARG;
      }
    }
    /* Check for unrecognized options. */
    else if ( argv[arg][0] == '-' ) {
      ERROR( STACKMETRICTESTC_EARG, STACKMETRICTESTC_MSGEARG, 0 );
      LALPrintError( USAGE, *argv );
      return STACKMETRICTESTC_EARG;
    }
    /* Parse remaining options. */
    else {
      if ( argc > arg + 2 ) {
	ra = atof( argv[arg++] );
	dec = atof( argv[arg++] );
	f0 = atof( argv[arg++] );
	nSpin = argc - arg;
	if ( nSpin )
	  spin = (REAL8 *)LALMalloc( nSpin*sizeof(REAL8) );
	j = 0;
	while ( arg < argc )
	  spin[j++] = atof( argv[arg++] );
      }else{
	ERROR( STACKMETRICTESTC_EARG, STACKMETRICTESTC_MSGEARG, 0 );
        LALPrintError( USAGE, *argv );
        return STACKMETRICTESTC_EARG;
      }
    }
  } /* End of argument parsing loop. */

  /* Do error trapping on input parameters. */
  if ( lalDebugLevel & LALERROR ) {
    CHECKVAL( n, 0, NMAX );
    CHECKVAL( dt, 1.0/f0, DTMAX );
    CHECKVAL( lat, -LAL_PI, LAL_PI );
    CHECKVAL( lon, -LAL_TWOPI, LAL_TWOPI );
    CHECKVAL( ra, -LAL_TWOPI, LAL_TWOPI );
    CHECKVAL( dec, -LAL_PI, LAL_PI );
    CHECKVAL( f0, 0.0, F0MAX );
    for ( j = 0; j < nSpin; j++ ) {
      REAL4 inverseAge = pow( spin[j], 1.0/( j + 1 ) );
      CHECKVAL( inverseAge, -1.0/TAUMIN, 1.0/TAUMIN );
    }
  }

  /* Set up start time. */
  start.gpsSeconds = (INT4)t0;
  start.gpsNanoSeconds = (INT4)( 1.0e9*( t0 - start.gpsSeconds ) );
  t0 = 0.0;

  /* Set up constant parameters for barycentre transformation. */
  baryParams.epoch = start;
  baryParams.latitude = lat;
  baryParams.longitude = lon;
  SUB( LALGetEarthTimes( &stat, &baryParams ), &stat );

  /* Set up constant parameters for spindown transformation. */
  spinParams.epoch = start;
  spinParams.t0 = t0;

  /* Set up constant parameters for composed transformation. */
  compParams.epoch = start;
  compParams.t1 = LALTBaryPtolemaic;
  compParams.t2 = LALTSpin;
  compParams.dt1 = LALDTBaryPtolemaic;
  compParams.dt2 = LALDTSpin;
  compParams.constants1 = &baryParams;
  compParams.constants2 = &spinParams;
  compParams.nArgs = 2;

  /* Set up constant parameters for metric calculation. */
  params.dtCanon = LALDTComp;
  params.constants = &compParams;
  /* To ignore spindown, uncomment the following:
  params.dtCanon = LALDTBaryPtolemaic;
  params.constants = &baryParams;
  nSpin = 0; */
  /* To ignore detector motion, uncomment the following:
  params.dtCanon = LALDTSpin;
  params.constants = &spinParams;
  nSky = 0; */

  params.start = t0;
  params.deltaT = dt;
  params.n = n;
  params.errors = 1;

  /* Set up the parameter list. */
  SUB( LALDCreateVector( &stat, &lambda, nSpin + nSky + 1 ), &stat );
  lambda->data[0] = f0;
  if ( nSky ) {
    lambda->data[1] = ra;
    lambda->data[2] = dec;
  }
  if ( nSpin ) {
    memcpy( lambda->data + nSky + 1, spin, nSpin*sizeof(REAL8) );
    LALFree( spin );
  }

  /* Compute the metric, and free lambda. */
  n = nSpin + nSky + 1;
  if ( params.errors )
    SUB( LALDCreateVector( &stat, &metric, n*(n+1) ), &stat );
  else
    SUB( LALDCreateVector( &stat, &metric, (n*(n+1)) >> 1 ), &stat );
  SUB( LALStackMetric( &stat, metric, lambda, &params ), &stat );
  SUB( LALDDestroyVector( &stat, &lambda ), &stat );

  /* Print the metric, project it, and print it again. */
  for ( i=0, k=0; i<n; i++ ){
    if ( params.errors )
      for ( j=0; j<=i; j++, k+=2 )
	fprintf( stdout, "%8.1e(%7.1e) ", metric->data[k],
		 metric->data[k+1] );
    else
      for ( j=0; j<=i; j++, k++ )
	fprintf( stdout, "%10.3e ", metric->data[k] );
    fprintf( stdout, "\n" );
  }
  SUB( LALProjectMetric( &stat, metric, params.errors ), &stat );
  fprintf( stdout, "\n" );
  for ( i=0, k=0; i<n; i++ ){
    if ( params.errors )
      for ( j=0; j<=i; j++, k+=2 )
	fprintf( stdout, "%8.1e(%7.1e) ", metric->data[k],
		 metric->data[k+1] );
    else
      for ( j=0; j<=i; j++, k++ )
	fprintf( stdout, "%10.3e ", metric->data[k] );
    fprintf( stdout, "\n" );
  }

  /* Free the metric, and exit. */
  SUB( LALDDestroyVector( &stat, &metric ), &stat );
  LALCheckMemoryLeaks();
  INFO( STACKMETRICTESTC_MSGENORM );
  return STACKMETRICTESTC_ENORM;
}
