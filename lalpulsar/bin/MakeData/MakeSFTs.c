/*
*  Copyright (C) 2007 Gregory Mendell
*  Copyright (C) 2010,2011,2016 Bernd Machenschalk
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
*  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
*  MA  02110-1301  USA
*/

/**
 * \file
 * \ingroup lalpulsar_bin_SFTTools
 * \author Gregory Mendell, Xavier Siemens, Bruce Allen, Bernd Machenschalk
 * \brief generate SFTs
 */

/*********************************************************************************/
/*                                                                               */
/* File: MakeSFTs.c                                                              */
/* Purpose: generate SFTs                                                        */
/* Origin: first written by Xavier Siemens, UWM - May 2005,                      */
/*         based on make_sfts.c by Bruce Allen                                   */
/*********************************************************************************/

/* REVISIONS: */
/* 11/02/05 gam; To save memory, change lalDebugLevel to 0; note when this is set to 3 that 3x the memory is used! */
/* 11/02/05 gam; To save memory, do not save the window function in memory; just recompute this for each SFT. */
/* 11/02/05 gam; To save memory, do not hold dataSingle.data and vtilde in memory at the same time. */
/* 11/03/05 gam; Add TRACKMEMUSE preprocessor flag and function for tracking memory usage; copied from make_sfts.c */
/* 11/19/05 gam; Reorganize code so that function appear in order called */
/* 11/19/05 gam; Add command line option to use single rather than double precision; will use LALDButterworthREAL4TimeSeries in single precision case. */
/* 11/19/05 gam; Rename vtilde fftDataDouble; add in fftDatasingle; rename fplan fftPlanDouble; add in fftPlanSingle */
/* 11/29/05 gam; Add PRINTEXAMPLEDATA to print the first NUMTOPRINT, middle NUMTOPRINT, and last NUMTOPRINT input/ouput data at various stages */
/* 12/27/05 gam; Add option make-gps-dirs, -D <num>, to make directory based on this many GPS digits. */
/* 12/28/05 gam; Add option misc-desc, -X <string> giving misc. part of the SFT description field in the filename */
/* 12/28/05 gam; Make FMIN = 48.0 and DF = 2000.0 global variables, add start-freq -F and band -B options to enter these */
/* 12/28/05 gam; Add in window-type options; 0 = no window, 1 = default = Matlab style Tukey window; 2 = make_sfts.c Tukey window; 3 = Hann window */
/* 12/28/05 gam; Add option --overlap-fraction -P (for use with windows; e.g., use -P 0.5 with -w 3 Hann windows; default is 0.0) */
/* 12/28/05 gam; Add sft-version, -v option to select output SFT version (1 = default is version 1 SFTs; 2 = version 2 SFTs */
/* 12/28/05 gam; Add comment-field, -c option, for comment for version 2 SFTs */
/* 01/05/06 gam; Add in version 2 normalization; add function to print example version 2 SFT data; add memory checking */
/* 01/09/06 gam; Add make-tmp-file, -Z option; write SFT to .*.tmp file, then move to final file name. */
/* 01/10/07 gam; Add -u --frame-struct-type option; specified the input data type in the frames (default ADC_REAL4) */
/* 01/14/07 gam; Add -i --ifo option to specify the ifo independent of the channel name which can begin with H0, L0, or G0. */
/* 06/26/07 gam; Write all command line arguments to commentField of version 2 SFTs, based on lalapps/src/calibration/ComputeStrainDriver.c */
/* 06/26/07 gam; Use finite to check that data does not contains a non-FINITE (+/- Inf, NaN) values, based on sftlib/SFTvalidate.c */
/* 10/05/12 gam; Add to version 2 normalization one over the root mean square of the window function (defined here as winFncRMS) as per RedMine LALSuite CW Bug #560*/
/* 24/07/14 eag; Change default SFT output to version 2 per RedMine LALSuite CW patch #1518 */

#include "config.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <glob.h>
#include <errno.h>
#include <stdarg.h>

#include <lal/LALDatatypes.h>
#include <lal/LALgetopt.h>
#include <lal/LALStdlib.h>
#include <lal/LALStdio.h>
#include <lal/FileIO.h>
#include <lal/AVFactories.h>
#include <lal/LALCache.h>
#include <lal/LALFrStream.h>
#include <lal/Window.h>
#include <lal/Calibration.h>
#include <lal/LALConstants.h>
#include <lal/BandPassTimeSeries.h>
#include <lal/LALStdlib.h>
#include <lal/LALConstants.h>
#include <lal/AVFactories.h>
#include <lal/TimeFreqFFT.h>
#include <lal/RealFFT.h>
#include <lal/ComplexFFT.h>
#include <lal/SFTfileIO.h>
#include <lal/LALVCSInfo.h>
#include <lal/LALPulsarVCSInfo.h>

/* print the first NUMTOPRINT, middle NUMTOPRINT, and last NUMTOPRINT input/ouput data at various stages */
#define PRINTEXAMPLEDATA 0
#define NUMTOPRINT       2

/* 06/26/07 gam; use finite to check that data does not contains a non-FINITE (+/- Inf, NaN) values */
#define CHECKFORINFINITEANDNANS 1

#define TESTSTATUS( pstat ) \
  if ( (pstat)->statusCode ) { REPORTSTATUS(pstat); return 100; } else ((void)0)

/***************************************************************************/

/* STRUCTURES */
struct CommandLineArgsTag {
  REAL8 HPf;              /* High pass filtering frequency */
  INT4 T;                 /* SFT duration */
  char *stringT;          /* 12/27/05 gam; string with SFT duration */
  INT4 GPSStart;
  INT4 GPSEnd;
  INT4 makeGPSDirs;        /* 12/27/05 gam; add option to make directories based on gps time */
  INT4 sftVersion;         /* 12/28/05 gam; output SFT version */
  char *commentField;      /* 12/28/05 gam; string comment for version 2 SFTs */
  BOOLEAN htdata;          /* flag that indicates we're doing h(t) data */
  BOOLEAN makeTmpFile;     /* 01/09/06 gam */
  char *FrCacheFile;       /* Frame cache file */
  char *ChannelName;
  char *IFO;               /* 01/14/07 gam */
  char *SFTpath;           /* path to SFT file location */
  char *miscDesc;          /* 12/28/05 gam; string giving misc. part of the SFT description field in the filename */
  INT4 windowOption;       /* 12/28/05 gam; window options; 0 = no window, 1 = default = Matlab style Tukey window; 2 = make_sfts.c Tukey window; 3 = Hann window */
  REAL8 windowR;
  REAL8 overlapFraction;   /* 12/28/05 gam; overlap fraction (for use with windows; e.g., use -P 0.5 with -w 3 Hann windows; default is 1.0). */
  BOOLEAN useSingle;       /* 11/19/05 gam; use single rather than double precision */
  char *frameStructType;   /* 01/10/07 gam */
} CommandLineArgs;

struct headertag {
  REAL8 endian;
  INT4  gps_sec;
  INT4  gps_nsec;
  REAL8 tbase;
  INT4  firstfreqindex;
  INT4  nsamples;
} header;


/***************************************************************************/

/* GLOBAL VARIABLES */
REAL8 FMIN = 48.0; /* default start frequency */
REAL8 DF = 2000.0; /* default band */

REAL8 winFncRMS = 1.0; /* 10/05/12 gam; global variable with the RMS of the window function; default value is 1.0 */

static LALStatus status;
LALCache *framecache;         /* frame reading variables */
LALFrStream *framestream=NULL;

REAL8TimeSeries dataDouble;
REAL4TimeSeries dataSingle;
INT2TimeSeries dataINT2;
INT4TimeSeries dataINT4;
INT8TimeSeries dataINT8;
INT4 SegmentDuration;
LIGOTimeGPS gpsepoch;

/* REAL8Vector *window; */ /* 11/02/05 gam */

REAL8FFTPlan *fftPlanDouble;           /* fft plan and data container, double precision case */
COMPLEX16Vector *fftDataDouble = NULL;

REAL4FFTPlan *fftPlanSingle;           /* 11/19/05 gam; fft plan and data container, single precision case */
COMPLEX8Vector *fftDataSingle = NULL;

CHAR allargs[16384]; /* 06/26/07 gam; copy all command line args into commentField, based on lalapps/src/calibration/ComputeStrainDriver.c */
/***************************************************************************/

/* FUNCTION PROTOTYPES */
/* Reads the command line */
int ReadCommandLine(int argc,char *argv[],struct CommandLineArgsTag *CLA);

/* Allocates space for data time series */
int AllocateData(struct CommandLineArgsTag CLA);

/* Reads data */
int ReadData(struct CommandLineArgsTag CLA);

/* High passes data */
int HighPass(struct CommandLineArgsTag CLA);

/* Windows data */
int WindowData(struct CommandLineArgsTag CLA);
int WindowDataTukey2(struct CommandLineArgsTag CLA);
int WindowDataHann(struct CommandLineArgsTag CLA);

/* create an SFT */
int CreateSFT(struct CommandLineArgsTag CLA);

/* writes out an SFT */
int WriteSFT(struct CommandLineArgsTag CLA);
int WriteVersion2SFT(struct CommandLineArgsTag CLA);

/* Frees the memory */
int FreeMem(struct CommandLineArgsTag CLA);

/* prototypes */
FILE* tryopen(char *name, const char *mode);
void getSFTDescField(CHAR *sftDescField, CHAR *numSFTs, CHAR *ifo, CHAR *stringT, CHAR *typeMisc);
void mkSFTDir(CHAR *sftPath, CHAR *site, CHAR *numSFTs, CHAR *ifo, CHAR *stringT, CHAR *typeMisc,CHAR *gpstime, INT4 numGPSdigits);
void mkSFTFilename(CHAR *sftFilename, CHAR *site, CHAR *numSFTs, CHAR *ifo, CHAR *stringT, CHAR *typeMisc,CHAR *gpstime);
void mvFilenames(CHAR *filename1, CHAR *filename2);


/* -------------------- function definitions -------------------- */


/* This repeatedly tries to re-open a file.  This is useful on a
   cluster because automount may fail or time out.  This allows a
   number of tries with a bit of rest in between.
*/
FILE* tryopen(char *name, const char *mode){
  int count=0;
  FILE *fp;
  
  while (!(fp=fopen(name, mode))){
    fprintf(stderr,"Unable to open file %s in mode %s.  Will retry...\n", name, mode);
    if (count++<10)
      sleep(10);
    else
      exit(3);
  }
  return fp;
}


/* 12/27/05 gam; function to create sftDescField for output directory or filename. */
void getSFTDescField(CHAR *sftDescField, CHAR *numSFTs, CHAR *ifo, CHAR *stringT, CHAR *typeMisc) {
       strcpy(sftDescField,numSFTs);
       strcat(sftDescField, "_");
       strcat(sftDescField,ifo);
       strcat(sftDescField, "_");
       strcat(sftDescField,stringT);
       strcat(sftDescField, "SFT");
       if (typeMisc != NULL) {
          strcat(sftDescField, "_");
          strcat(sftDescField, typeMisc);
       }
}

/* 12/27/05 gam; concat to the sftPath the directory name based on GPS time; make this directory if it does not already exist */
void mkSFTDir(CHAR *sftPath, CHAR *site, CHAR *numSFTs, CHAR *ifo, CHAR *stringT, CHAR *typeMisc,CHAR *gpstime, INT4 numGPSdigits) {
     CHAR sftDescField[256];
     CHAR mkdirCommand[256];
     strcat(sftPath,"/");
     strcat(sftPath,site);
     strcat(sftPath,"-");
     getSFTDescField(sftDescField, numSFTs, ifo, stringT, typeMisc);
     strcat(sftPath,sftDescField);
     strcat(sftPath,"-");
     strncat(sftPath,gpstime,numGPSdigits);
     sprintf(mkdirCommand,"mkdir -p %s",sftPath);
     if ( system(mkdirCommand) ) XLALPrintError ("system() returned non-zero status\n");
}

/* 12/27/05 gam; make SFT file name according to LIGO T040164-01 specification */
void mkSFTFilename(CHAR *sftFilename, CHAR *site, CHAR *numSFTs, CHAR *ifo, CHAR *stringT, CHAR *typeMisc,CHAR *gpstime) {
     CHAR sftDescField[256];
     strcpy(sftFilename,site);
     strcat(sftFilename,"-");
     getSFTDescField(sftDescField, numSFTs, ifo, stringT, typeMisc);
     strcat(sftFilename,sftDescField);
     strcat(sftFilename,"-");
     strcat(sftFilename,gpstime);
     strcat(sftFilename,"-");
     strcat(sftFilename,stringT);
     strcat(sftFilename,".sft");
}

/* 01/09/06 gam; move filename1 to filename2 */
void mvFilenames(CHAR *filename1, CHAR *filename2) {
     CHAR mvFilenamesCommand[512+4];
     sprintf(mvFilenamesCommand,"mv %s %s",filename1,filename2);
     if ( system(mvFilenamesCommand) ) XLALPrintError ("system() returned non-zero status\n");
}

#if PRINTEXAMPLEDATA
void printExampleDataSingle() {
      INT4 i,dataLength;
      dataLength = dataSingle.data->length;
      fprintf(stdout,"dataSingle.deltaT, 1.0/dataSingle.deltaT = %23.16e, %23.16e\n",dataSingle.deltaT,1.0/dataSingle.deltaT);
      fprintf(stdout,"dataSingle.epoch.gpsSeconds,dataSingle.epoch.gpsNanoSeconds = %i, %i\n",dataSingle.epoch.gpsSeconds,dataSingle.epoch.gpsNanoSeconds);
      for (i=0;i<NUMTOPRINT;i++) {
        fprintf(stdout,"%i %23.16e\n",i,dataSingle.data->data[i]);
      }
      for (i=dataLength/2;i<dataLength/2+NUMTOPRINT;i++) {
        fprintf(stdout,"%i %23.16e\n",i,dataSingle.data->data[i]);
      }
      for (i=dataLength-NUMTOPRINT;i<dataLength;i++) {
        fprintf(stdout,"%i %23.16e\n",i,dataSingle.data->data[i]);
      }
      fflush(stdout);
}    

void printExampleDataDouble() {
      INT4 i,dataLength;
      dataLength = dataDouble.data->length;
      fprintf(stdout,"dataDouble.deltaT, 1.0/dataDouble.deltaT = %23.16e, %23.16e\n",dataDouble.deltaT,1.0/dataDouble.deltaT);
      fprintf(stdout,"dataDouble.epoch.gpsSeconds,dataDouble.epoch.gpsNanoSeconds = %i, %i\n",dataDouble.epoch.gpsSeconds,dataDouble.epoch.gpsNanoSeconds);
      for (i=0;i<NUMTOPRINT;i++) {
        fprintf(stdout,"%i %23.16e\n",i,dataDouble.data->data[i]);
      }
      for (i=dataLength/2;i<dataLength/2+NUMTOPRINT;i++) {
        fprintf(stdout,"%i %23.16e\n",i,dataDouble.data->data[i]);
      }
      for (i=dataLength-NUMTOPRINT;i<dataLength;i++) {
        fprintf(stdout,"%i %23.16e\n",i,dataDouble.data->data[i]);
      }
      fflush(stdout);
}

void printExampleFFTData(struct CommandLineArgsTag CLA)
{
  int firstbin=(INT4)(FMIN*CLA.T+0.5);
  int k;
  INT4 nsamples=(INT4)(DF*CLA.T+0.5);

  if(CLA.useSingle) {
    for (k=0; k<NUMTOPRINT; k++) {
      fprintf(stdout,"%i %23.16e %23.16e\n",k,fftDataSingle->data[k+firstbin].re,fftDataSingle->data[k+firstbin].im);
    }
    for (k=nsamples/2; k<nsamples/2+NUMTOPRINT; k++) {
      fprintf(stdout,"%i %23.16e %23.16e\n",k,fftDataSingle->data[k+firstbin].re,fftDataSingle->data[k+firstbin].im);
    }
    for (k=nsamples-NUMTOPRINT; k<nsamples; k++) {
      fprintf(stdout,"%i %23.16e %23.16e\n",k,fftDataSingle->data[k+firstbin].re,fftDataSingle->data[k+firstbin].im);
    }    
  } else {
    for (k=0; k<NUMTOPRINT; k++) {
      fprintf(stdout,"%i %23.16e %23.16e\n",k,fftDataDouble->data[k+firstbin].re,fftDataDouble->data[k+firstbin].im);
    }
    for (k=nsamples/2; k<nsamples/2+NUMTOPRINT; k++) {
      fprintf(stdout,"%i %23.16e %23.16e\n",k,fftDataDouble->data[k+firstbin].re,fftDataDouble->data[k+firstbin].im);
    }
    for (k=nsamples-NUMTOPRINT; k<nsamples; k++) {
      fprintf(stdout,"%i %23.16e %23.16e\n",k,fftDataDouble->data[k+firstbin].re,fftDataDouble->data[k+firstbin].im);
    }    
  }
  fflush(stdout);
}

void printExampleSFTDataGoingToFile(struct CommandLineArgsTag CLA)
{
  int firstbin=(INT4)(FMIN*CLA.T+0.5);
  int k;
  INT4 nsamples=(INT4)(DF*CLA.T+0.5);
  REAL4 rpw,ipw;

  if(CLA.useSingle) {
    for (k=0; k<NUMTOPRINT; k++) {
      rpw=((REAL4)(((REAL8)DF)/(0.5*(REAL8)(1/dataSingle.deltaT))))
	* fftDataSingle->data[k+firstbin].re;
      ipw=((REAL4)(((REAL8)DF)/(0.5*(REAL8)(1/dataSingle.deltaT))))
	* fftDataSingle->data[k+firstbin].im;
      fprintf(stdout,"%i %23.16e %23.16e\n",k,rpw,ipw);
    }
    for (k=nsamples/2; k<nsamples/2+NUMTOPRINT; k++) {
      rpw=((REAL4)(((REAL8)DF)/(0.5*(REAL8)(1/dataSingle.deltaT))))
	* fftDataSingle->data[k+firstbin].re;
      ipw=((REAL4)(((REAL8)DF)/(0.5*(REAL8)(1/dataSingle.deltaT))))
	* fftDataSingle->data[k+firstbin].im;
      fprintf(stdout,"%i %23.16e %23.16e\n",k,rpw,ipw);
    }
    for (k=nsamples-NUMTOPRINT; k<nsamples; k++) {
      rpw=((REAL4)(((REAL8)DF)/(0.5*(REAL8)(1/dataSingle.deltaT))))
	* fftDataSingle->data[k+firstbin].re;
      ipw=((REAL4)(((REAL8)DF)/(0.5*(REAL8)(1/dataSingle.deltaT))))
	* fftDataSingle->data[k+firstbin].im;
      fprintf(stdout,"%i %23.16e %23.16e\n",k,rpw,ipw);
    }    
  } else {
    for (k=0; k<NUMTOPRINT; k++) {
      rpw=(((REAL8)DF)/(0.5*(REAL8)(1/dataDouble.deltaT))) 
	* fftDataDouble->data[k+firstbin].re;
      ipw=(((REAL8)DF)/(0.5*(REAL8)(1/dataDouble.deltaT))) 
	* fftDataDouble->data[k+firstbin].im;
      fprintf(stdout,"%i %23.16e %23.16e\n",k,rpw,ipw);
    }
    for (k=nsamples/2; k<nsamples/2+NUMTOPRINT; k++) {
      rpw=(((REAL8)DF)/(0.5*(REAL8)(1/dataDouble.deltaT))) 
	* fftDataDouble->data[k+firstbin].re;
      ipw=(((REAL8)DF)/(0.5*(REAL8)(1/dataDouble.deltaT))) 
	* fftDataDouble->data[k+firstbin].im;
      fprintf(stdout,"%i %23.16e %23.16e\n",k,rpw,ipw);
    }
    for (k=nsamples-NUMTOPRINT; k<nsamples; k++) {
      rpw=(((REAL8)DF)/(0.5*(REAL8)(1/dataDouble.deltaT))) 
	* fftDataDouble->data[k+firstbin].re;
      ipw=(((REAL8)DF)/(0.5*(REAL8)(1/dataDouble.deltaT))) 
	* fftDataDouble->data[k+firstbin].im;
      fprintf(stdout,"%i %23.16e %23.16e\n",k,rpw,ipw);
    }    
  }
  fflush(stdout);
}

void printExampleVersion2SFTDataGoingToFile(struct CommandLineArgsTag CLA, SFTtype *oneSFT)
{
  int k;
  INT4 nsamples=(INT4)(DF*CLA.T+0.5);

  for (k=0; k<NUMTOPRINT; k++) {  
    fprintf(stdout,"%i %23.16e %23.16e\n",k,oneSFT->data->data[k].re,oneSFT->data->data[k].im);
  }
  for (k=nsamples/2; k<nsamples/2+NUMTOPRINT; k++) {
    fprintf(stdout,"%i %23.16e %23.16e\n",k,oneSFT->data->data[k].re,oneSFT->data->data[k].im);
  }
  for (k=nsamples-NUMTOPRINT; k<nsamples; k++) {
    fprintf(stdout,"%i %23.16e %23.16e\n",k,oneSFT->data->data[k].re,oneSFT->data->data[k].im);
  }    
}
#endif


/************************************* MAIN PROGRAM *************************************/

int main(int argc,char *argv[])
{
  /* int j; */ /* 12/28/05 gam */

  if (ReadCommandLine(argc,argv,&CommandLineArgs)) return 1;
  SegmentDuration = CommandLineArgs.GPSEnd - CommandLineArgs.GPSStart ;

  /* create Frame cache, open frame stream and delete frame cache */
  framecache = XLALCacheImport(CommandLineArgs.FrCacheFile);
  LALFrCacheOpen(&status,&framestream,framecache);
  TESTSTATUS( &status );
  XLALDestroyCache(framecache);

  if( SegmentDuration < CommandLineArgs.T)
    {
      fprintf(stderr,"Cannot fit an SFT of duration %d between %d and %d\n",
	      CommandLineArgs.T, CommandLineArgs.GPSStart, CommandLineArgs.GPSEnd );
      return 0;;
    }

  gpsepoch.gpsSeconds = CommandLineArgs.GPSStart;
  gpsepoch.gpsNanoSeconds = 0;

  /* Allocates space for data */
  if (AllocateData(CommandLineArgs)) return 2;

  /* for(j=0; j < SegmentDuration/CommandLineArgs.T; j++) */ /* 12/28/05 gam */
  while(gpsepoch.gpsSeconds + CommandLineArgs.T <= CommandLineArgs.GPSEnd)
    {

      /* gpsepoch.gpsSeconds = CommandLineArgs.GPSStart + j*CommandLineArgs.T;
      gpsepoch.gpsNanoSeconds = 0; */ /* 12/28/05 gam; now updated at the bottom of while loop */

      /* Reads T seconds of data */
      if (ReadData(CommandLineArgs)) return 3;

      /* High-pass data with Butterworth filter */
      if(HighPass(CommandLineArgs)) return 4;

      /* Window data; 12/28/05 gam; add options */
      if (CommandLineArgs.windowOption==1) {
         if(WindowData(CommandLineArgs)) return 5; /* CommandLineArgs.windowOption==1 is the default */
      } else if (CommandLineArgs.windowOption==2) {
         if(WindowDataTukey2(CommandLineArgs)) return 5;
      } else if (CommandLineArgs.windowOption==3) {
         if(WindowDataHann(CommandLineArgs)) return 5;
      } else {
        /* Continue with no windowing; parsing of command line args makes sure options are one of the above or 0 for now windowing. */
      }

      /* create an SFT */
      if(CreateSFT(CommandLineArgs)) return 6;

      /* write out sft */
      if (CommandLineArgs.sftVersion==1) {
        if(WriteSFT(CommandLineArgs)) return 7;
      } else if (CommandLineArgs.sftVersion==2) {
        if(WriteVersion2SFT(CommandLineArgs)) return 7; /* default now is to output version 2 SFTs */
      } else {
        fprintf( stderr, "Invalid input for 'sft-version = %d', must be either '1' or '2'\n", CommandLineArgs.sftVersion );
        return 0;;
      }

      gpsepoch.gpsSeconds = gpsepoch.gpsSeconds + (INT4)((1.0 - CommandLineArgs.overlapFraction)*((REAL8)CommandLineArgs.T));
      gpsepoch.gpsNanoSeconds = 0;
    }

  if(FreeMem(CommandLineArgs)) return 8;

  return 0;
}

/************************************* MAIN PROGRAM ENDS *************************************/

/*** FUNCTIONS ***/

/*******************************************************************************/
int ReadCommandLine(int argc,char *argv[],struct CommandLineArgsTag *CLA)
{
  INT4 errflg=0;
  INT4 i;              /* 06/26/07 gam */
  struct LALoption long_options[] = {
    {"high-pass-freq",       required_argument, NULL,          'f'},
    {"sft-duration",         required_argument, NULL,          't'},
    {"sft-write-path",       required_argument, NULL,          'p'},
    {"frame-cache",          required_argument, NULL,          'C'},
    {"channel-name",         required_argument, NULL,          'N'},
    {"gps-start-time",       required_argument, NULL,          's'},
    {"gps-end-time",         required_argument, NULL,          'e'},
    {"sft-version",          required_argument, NULL,          'v'},
    {"comment-field",        required_argument, NULL,          'c'},
    {"start-freq",           required_argument, NULL,          'F'},
    {"band",                 required_argument, NULL,          'B'},
    {"make-gps-dirs",        required_argument, NULL,          'D'},
    {"make-tmp-file",        required_argument, NULL,          'Z'},
    {"misc-desc",            required_argument, NULL,          'X'},
    {"frame-struct-type",    required_argument, NULL,          'u'},
    {"ifo",                  required_argument, NULL,          'i'},
    {"window-type",          required_argument, NULL,          'w'},
    {"window-radius",        required_argument, NULL,          'r'},
    {"overlap-fraction",     required_argument, NULL,          'P'},
    {"ht-data",              no_argument,       NULL,          'H'},
    {"use-single",           no_argument,       NULL,          'S'},
    {"help",                 no_argument,       NULL,          'h'},
    {"version",              no_argument,       NULL,          'V'},
    {0, 0, 0, 0}
  };
  char args[] = "hHZSf:t:C:N:i:s:e:v:c:F:B:D:X:u:w:r:P:p:ab:";

  /* Initialize default values */
  CLA->HPf=-1.0;
  CLA->T=0.0;
  CLA->stringT=NULL;  /* 12/27/05 gam */
  CLA->FrCacheFile=NULL;
  CLA->GPSStart=0;
  CLA->GPSEnd=0;
  CLA->makeGPSDirs=0; /* 12/27/05 gam; add option to make directories based on gps time */
  CLA->sftVersion=2;  /* 07/24/14 eag; output SFT version; default is version 2 SFTs */
  CLA->ChannelName=NULL;
  CLA->IFO=NULL;        /* 01/14/07 gam */
  CLA->SFTpath=NULL;
  CLA->miscDesc=NULL;   /* 12/28/05 gam; misc. part of the SFT description field in the filename (also used if makeGPSDirs > 0) */
  CLA->commentField=NULL; /* 12/28/05 gam; comment for version 2 SFT header. */
  CLA->windowOption=1;  /* 12/28/05 gam; window options; 0 = no window, 1 = default = Matlab style Tukey window; 2 = make_sfts.c Tukey window; 3 = Hann window */
  CLA->windowR = 0.001;
  CLA->overlapFraction=0.0; /* 12/28/05 gam; overlap fraction (for use with windows; e.g., use -P 0.5 with -w 3 Hann windows; default is 0.0). */
  CLA->htdata = 0;
  CLA->makeTmpFile = 0; /* 01/09/06 gam */  
  CLA->useSingle = 0;        /* 11/19/05 gam; default is to use double precision, not single. */
  CLA->frameStructType=NULL; /* 01/10/07 gam */

  strcat(allargs, "\nMakeSFTs ");
  strcat(allargs, lalVCSIdentInfo.vcsId);
  strcat(allargs, lalVCSIdentInfo.vcsStatus);
  strcat(allargs, "\nMakeSFTs ");
  strcat(allargs, lalPulsarVCSIdentInfo.vcsId);
  strcat(allargs, lalPulsarVCSIdentInfo.vcsStatus);
  strcat(allargs, "\nMakeSFTs command line args: "); /* 06/26/07 gam; copy all command line args into commentField */
  for(i = 0; i < argc; i++)
  {
      /* if the argument describes an additioanl comment, don't record it
         in the command-line as well, as it will be recoeded in the comment later.
         if the option is a single argument and does not include the comment itself,
         skip the next argument as well, as this should be the comment then. */
      if ((strstr(argv[i], "-c")==argv[i])) {
          strcat(allargs, "-c ... ");
          if (strcmp(argv[i], "-c") == 0) {
              i++;
          }
          continue;
      }
      if ((strstr(argv[i], "--comment-field")==argv[i])) {
          strcat(allargs, "--comment-field ... ");
          if (strcmp(argv[i], "--comment-field") == 0) {
              i++;
          }
          continue;
      }
      strcat(allargs,argv[i]);
      strcat(allargs, " ");
  }
  strcat(allargs, "\n");
  CLA->commentField=allargs;
      
  /* Scan through list of command line arguments */
  while ( 1 )
  {
    int option_index = 0; /* LALgetopt_long stores long option here */
    int c;

    c = LALgetopt_long_only( argc, argv, args, long_options, &option_index );
    if ( c == -1 ) /* end of options */
      break;

    switch ( c )
    {
    case 'H':
      /* high pass frequency */
      CLA->htdata=1;
      break;
    case 'Z':
      /* make tmp file */
      CLA->makeTmpFile=1;
      break;
    case 'S':
      /* use single precision */
      CLA->useSingle=1;
      break;
    case 'f':
      /* high pass frequency */
      CLA->HPf=atof(LALoptarg);
      break;
    case 't':
      /* SFT time */
      CLA->stringT = LALoptarg;  /* 12/27/05 gam; keep pointer to string that gives the SFT duration */
      CLA->T=atoi(LALoptarg);
      break;
    case 'C':
      /* name of frame cache file */
      CLA->FrCacheFile=LALoptarg;
      break;
    case 's':
      /* GPS start */
      CLA->GPSStart=atof(LALoptarg);
      break;
    case 'e':
      /* GPS end */
      CLA->GPSEnd=atof(LALoptarg);
      break;
    case 'F':
      /* 12/28/05 gam; start frequency */
      FMIN=(REAL8)atof(LALoptarg);
      break;
    case 'B':
      /* 12/28/05 gam; band */
      DF=(REAL8)atof(LALoptarg);
      break;
    case 'D':
      /* 12/27/05 gam; make directories based on GPS time */
      CLA->makeGPSDirs=atof(LALoptarg);
      break;
    case 'v':
      /* 07/24/14 eag; output SFT version; default is version 2 SFTs */
      CLA->sftVersion=atoi(LALoptarg);
      break;
    case 'c':
      /* 12/28/05 gam; comment for version 2 SFTs */
      /* CLA->commentField=LALoptarg; */ /* 06/26/07 gam */
      strcat(CLA->commentField, "MakeSFTs additional comment: "); /* 06/26/07 gam; copy all command line args into commentField */
      strcat(CLA->commentField,LALoptarg);
      strcat(CLA->commentField,"\n");
      break;
    case 'X':
      /* 12/28/05 gam; misc. part of the SFT description field in the filename (also used if makeGPSDirs > 0) */
      CLA->miscDesc=LALoptarg;
      break;
    case 'u':
      CLA->frameStructType=LALoptarg; /* 01/10/07 gam */
      break;
    case 'w':
      /* 12/28/05 gam; window options; 0 = no window, 1 = default = Matlab style Tukey window; 2 = make_sfts.c Tukey window; 3 = Hann window */
      CLA->windowOption=atoi(LALoptarg);
      break;
    case 'r':
      /* defulat 0.001 */
      CLA->windowR=(REAL8)atof(LALoptarg);
      break;
    case 'P':
      /* 12/28/05 gam; overlap fraction (for use with windows; e.g., use -P 0.5 with -w 3 Hann windows; default is 1.0). */
      CLA->overlapFraction=(REAL8)atof(LALoptarg);
      break;
    case 'N':
      CLA->ChannelName=LALoptarg;
      break;
    case 'i':
      CLA->IFO=LALoptarg; /* 01/14/07 gam */
      break;
    case 'p':
      CLA->SFTpath=LALoptarg;
      break;
    case 'h':
      /* print usage/help message */
      fprintf(stdout,"Arguments are:\n");
      fprintf(stdout,"\thigh-pass-freq (-f)\tFLOAT\t High pass filtering frequency in Hz.\n");
      fprintf(stdout,"\tsft-duration (-t)\tFLOAT\t SFT duration in seconds.\n");
      fprintf(stdout,"\tsft-write-path (-p)\tFLOAT\t Location of output SFTs.\n");
      fprintf(stdout,"\tframe-cache (-C)\tSTRING\t Path to frame cache file (including the filename).\n");
      fprintf(stdout,"\tgps-start-time (-s)\tINT\t GPS start time of segment.\n");
      fprintf(stdout,"\tgps-end-time (-e)\tINT\t GPS end time of segment.\n");
      fprintf(stdout,"\tchannel-name (-N)\tSTRING\t Name of channel to read within a frame.\n");
      fprintf(stdout,"\tifo (-i)\t\tSTRING\t (optional) Name of IFO, i.e., H1, H2, L1, or G1; use if channel name begins with H0, L0, or G0; default: use first two characters from channel name.\n");
      fprintf(stdout,"\tsft-version (-v)\tINT\t (optional) SFT version (1 = output version 1 SFTs; 2 = default = output version 2 SFTs.\n");
      fprintf(stdout,"\tcomment-field (-c)\tSTRING\t (optional) Comment for version 2 SFT header.\n");
      fprintf(stdout,"\tstart-freq (-F) \tFLOAT\t (optional) Start frequency of the SFTs (default is 48 Hz).\n");
      fprintf(stdout,"\tband (-B)       \tFLOAT\t (optional) Frequency band of the SFTs (default is 2000 Hz).\n");
      fprintf(stdout,"\tmake-gps-dirs (-D)\tINT\t (optional) Make directories for output SFTs based on this many digits of the GPS time.\n");
      fprintf(stdout,"\tmake-tmp-file (-Z)\tINT\t (optional) Write SFT to .*.tmp file, then move to final filename.\n");
      fprintf(stdout,"\tmisc-desc (-X)   \tSTRING\t (optional) Misc. part of the SFT description field in the filename (also used if make-gps-dirs, -D option, is > 0)\n");
      fprintf(stdout,"\twindow-type (-w)\tINT\t (optional) 0 = no window, 1 = default = Matlab style Tukey window; 2 = make_sfts.c Tukey window; 3 = Hann window\n");
      fprintf(stdout,"\twindow-radius (-r)\tFLOAT\t (optional) default = 0.001\n");
      fprintf(stdout,"\toverlap-fraction (-P)\tFLOAT\t (optional) Overlap fraction (for use with windows; e.g., use -P 0.5 with -w 3 Hann windows; default is 0.0).\n");
      fprintf(stdout,"\tht-data (-H)\t\tFLAG\t (optional) Input data is h(t) data (input is PROC_REAL8 data ).\n");
      fprintf(stdout,"\tuse-single (-S)\t\tFLAG\t (optional) Use single precision for window, plan, and fft; double precision filtering is always done.\n");
      fprintf(stdout,"\tframe-struct-type (-u)\tSTRING\t (optional) String specifying the input frame structure and data type. Must begin with ADC_ or PROC_ followed by REAL4, REAL8, INT2, INT4, or INT8; default: ADC_REAL4; -H is the same as PROC_REAL8.\n");
      fprintf(stdout,"\tversion (-V)\t\tFLAG\t Print LAL & LALPulsar version and exit.\n");
      fprintf(stdout,"\thelp (-h)\t\tFLAG\t This message.\n");
      exit(0);
    case 'V':
      /* print version */
      fprintf(stdout,"MakeSFTs %s %s\n", lalVCSIdentInfo.vcsId, lalVCSIdentInfo.vcsStatus);
      fprintf(stdout,"MakeSFTs %s %s\n", lalPulsarVCSIdentInfo.vcsId, lalPulsarVCSIdentInfo.vcsStatus);
      exit(0);
    default:
      /* unrecognized option */
      errflg++;
      if((c>=48) && (c<128))
	fprintf(stderr,"Unrecognized option '%c'\n",c);
      else
	fprintf(stderr,"Unrecognized option %d\n",c);
      exit(1);
      break;
    }
  }

  if(CLA->HPf < 0 )
    {
      fprintf(stderr,"No high pass filtering frequency specified.\n"); 
      fprintf(stderr,"If you don't want to high pass filter set the frequency to 0.\n");
      fprintf(stderr,"Try %s -h \n",argv[0]);
      return 1;
    }      
  if(CLA->T == 0.0)
    {
      fprintf(stderr,"No SFT duration specified.\n");
      fprintf(stderr,"Try %s -h \n", argv[0]);
      return 1;
    }      
  if(CLA->GPSStart == 0)
    {
      fprintf(stderr,"No GPS start time specified.\n");
       fprintf(stderr,"Try %s -h \n", argv[0]);
      return 1;
    }
  if(CLA->GPSEnd == 0)
    {
      fprintf(stderr,"No GPS end time specified.\n");
      fprintf(stderr,"Try %s -h \n", argv[0]);
      return 1;
    }
  if(FMIN < 0.0)
    {
      fprintf(stderr,"Illegal start-freq option given.\n");
      fprintf(stderr,"Try %s -h \n", argv[0]);
      return 1;
    }
  if(DF < 0.0)
    {
      fprintf(stderr,"Illegal band option given.\n");
      fprintf(stderr,"Try %s -h \n", argv[0]);
      return 1;
    }
  if(CLA->makeGPSDirs < 0)
    {
      fprintf(stderr,"Illegal make-gps-dirs option given.\n");
      fprintf(stderr,"Try %s -h \n", argv[0]);
      return 1;
    }
  if( (CLA->sftVersion < 1) || (CLA->sftVersion > 2) )
    {
      fprintf(stderr,"Illegal sft-version given.\n");
      fprintf(stderr,"Try %s -h \n", argv[0]);
      return 1;
    }
  if( (CLA->windowOption < 0) || (CLA->windowOption > 3) )
    {
      fprintf(stderr,"Illegal window-type given.\n");
      fprintf(stderr,"Try %s -h \n", argv[0]);
      return 1;
    }
  if( (CLA->overlapFraction < 0.0) || (CLA->overlapFraction >= 1.0) )
    {
      fprintf(stderr,"Illegal overlap-fraction given.\n");
      fprintf(stderr,"Try %s -h \n", argv[0]);
      return 1;
    }
  if(CLA->FrCacheFile == NULL)
    {
      fprintf(stderr,"No frame cache file specified.\n");
      fprintf(stderr,"Try %s -h \n", argv[0]);
      return 1;
    }      
  if(CLA->ChannelName == NULL)
    {
      fprintf(stderr,"No data channel name specified.\n");
      fprintf(stderr,"Try %s -h \n", argv[0]);
      return 1;
    }      
  if(CLA->SFTpath == NULL)
    {
      fprintf(stderr,"No output path specified for SFTs.\n");
      fprintf(stderr,"Try %s -h \n", argv[0]);
      return 1;
    }      

  return errflg;
}
/*******************************************************************************/

/*******************************************************************************/
int AllocateData(struct CommandLineArgsTag CLA)
{
  static FrChanIn chanin;

  chanin.name  = CLA.ChannelName;

  /* These calls just return deltaT for the channel */
  if(CLA.htdata)
    {
      chanin.type  = ProcDataChannel;
      /* Get channel time step size by calling LALFrGetREAL8TimeSeries */
      LALFrSeek(&status,&gpsepoch,framestream);
      TESTSTATUS( &status );
      LALFrGetREAL8TimeSeries(&status,&dataDouble,&chanin,framestream);
      TESTSTATUS( &status );
      dataSingle.deltaT=dataDouble.deltaT;      
    }
  else if (CLA.frameStructType != NULL) 
    {
      /* 01/10/07 gam */  
      if ( strstr(CLA.frameStructType,"ADC_") ) {
         chanin.type  = ADCDataChannel;
      } else if ( strstr(CLA.frameStructType,"PROC_") ) {
         chanin.type  = ProcDataChannel;
      } else {
        return 1; 
      }
      /* Get channel time step size by calling LALFrGet... functions: */
      LALFrSeek(&status,&gpsepoch,framestream);
      TESTSTATUS( &status );      
      if ( strstr(CLA.frameStructType,"REAL8") ) {
         LALFrGetREAL8TimeSeries(&status,&dataDouble,&chanin,framestream);
         TESTSTATUS( &status );
         dataSingle.deltaT=dataDouble.deltaT;
      } else if ( strstr(CLA.frameStructType,"REAL4") ) { 
         LALFrGetREAL4TimeSeries(&status,&dataSingle,&chanin,framestream);
         TESTSTATUS( &status );
         dataDouble.deltaT=dataSingle.deltaT;
      } else if ( strstr(CLA.frameStructType,"INT2") ) {
         LALFrGetINT2TimeSeries(&status,&dataINT2,&chanin,framestream);
         TESTSTATUS( &status );
         dataDouble.deltaT = dataINT2.deltaT;
         dataSingle.deltaT=dataDouble.deltaT;
      } else if ( strstr(CLA.frameStructType,"INT4") ) {
         LALFrGetINT4TimeSeries(&status,&dataINT4,&chanin,framestream);
         TESTSTATUS( &status );
         dataDouble.deltaT = dataINT4.deltaT;
         dataSingle.deltaT=dataDouble.deltaT;
      } else if ( strstr(CLA.frameStructType,"INT8") ) {      
         LALFrGetINT8TimeSeries(&status,&dataINT8,&chanin,framestream);
         TESTSTATUS( &status );
         dataDouble.deltaT = dataINT8.deltaT;
         dataSingle.deltaT=dataDouble.deltaT;
      } else {
        return 1;
      }      
    }
  else
    {
      chanin.type  = ADCDataChannel;
      /* Get channel time step size by calling LALFrGetREAL4TimeSeries */
      LALFrSeek(&status,&gpsepoch,framestream);
      TESTSTATUS( &status );
      LALFrGetREAL4TimeSeries(&status,&dataSingle,&chanin,framestream);
      TESTSTATUS( &status );
      dataDouble.deltaT=dataSingle.deltaT;
    }

  /*  11/19/05 gam; will keep dataDouble or dataSingle in memory at all times, depending on whether using single or double precision */
  if(CLA.useSingle) {
      LALCreateVector(&status,&dataSingle.data,(UINT4)(CLA.T/dataSingle.deltaT +0.5));
      TESTSTATUS( &status );

      fftPlanSingle = XLALCreateForwardREAL4FFTPlan( dataSingle.data->length, 0 );
      XLAL_CHECK( fftPlanSingle != NULL, XLAL_EFUNC );

  } else {  
      LALDCreateVector(&status,&dataDouble.data,(UINT4)(CLA.T/dataDouble.deltaT +0.5));
      TESTSTATUS( &status );

      fftPlanDouble = XLALCreateForwardREAL8FFTPlan( dataDouble.data->length, 0 );
      XLAL_CHECK( fftPlanDouble != NULL, XLAL_EFUNC );
  }

  return 0;
}
/*******************************************************************************/

/*******************************************************************************/
int ReadData(struct CommandLineArgsTag CLA)
{
  static FrChanIn chanin;
  INT4 k;
  chanin.name  = CLA.ChannelName;
  LALFrSeek(&status,&gpsepoch,framestream);
  TESTSTATUS( &status );

  /* 11/19/05 gam */
  if(CLA.useSingle) {
    
    if(CLA.htdata)
    {

      LALDCreateVector(&status,&dataDouble.data,(UINT4)(CLA.T/dataDouble.deltaT +0.5));
      TESTSTATUS( &status );

      chanin.type  = ProcDataChannel;
      LALFrGetREAL8TimeSeries(&status,&dataDouble,&chanin,framestream);
      TESTSTATUS( &status );

      /*copy the data into the single precision timeseries */
      for (k = 0; k < (int)dataSingle.data->length; k++) {
          dataSingle.data->data[k] = dataDouble.data->data[k];
      }

      LALDDestroyVector(&status,&dataDouble.data);
      TESTSTATUS( &status );

    }
    else if (CLA.frameStructType != NULL)
    {
      /* 01/10/07 gam */  
      if ( strstr(CLA.frameStructType,"ADC_") ) {
         chanin.type  = ADCDataChannel;
      } else if ( strstr(CLA.frameStructType,"PROC_") ) {
         chanin.type  = ProcDataChannel;
      } else {
        return 1; 
      }
      if ( strstr(CLA.frameStructType,"REAL8") ) {

         LALDCreateVector(&status,&dataDouble.data,(UINT4)(CLA.T/dataDouble.deltaT +0.5));
         TESTSTATUS( &status );

         LALFrGetREAL8TimeSeries(&status,&dataDouble,&chanin,framestream);
         TESTSTATUS( &status );

         /*copy the data into the single precision timeseries */
         for (k = 0; k < (int)dataSingle.data->length; k++) {
             dataSingle.data->data[k] = dataDouble.data->data[k];
         }

         LALDDestroyVector(&status,&dataDouble.data);
         TESTSTATUS( &status );

      } else if ( strstr(CLA.frameStructType,"REAL4") ) {      

         LALFrGetREAL4TimeSeries(&status,&dataSingle,&chanin,framestream);
         TESTSTATUS( &status );

      } else if ( strstr(CLA.frameStructType,"INT2") ) {

         LALI2CreateVector(&status,&dataINT2.data,(UINT4)(CLA.T/dataINT2.deltaT +0.5));
         TESTSTATUS( &status );

         LALFrGetINT2TimeSeries(&status,&dataINT2,&chanin,framestream);
         TESTSTATUS( &status );

         /*copy the data into the single precision timeseries */
         for (k = 0; k < (int)dataDouble.data->length; k++) {
             dataSingle.data->data[k] = (REAL4)(dataINT2.data->data[k]);
         }     

         LALI2DestroyVector(&status,&dataINT2.data);
         TESTSTATUS( &status );

      } else if ( strstr(CLA.frameStructType,"INT4") ) {

         LALI4CreateVector(&status,&dataINT4.data,(UINT4)(CLA.T/dataINT4.deltaT +0.5));
         TESTSTATUS( &status );

         LALFrGetINT4TimeSeries(&status,&dataINT4,&chanin,framestream);
         TESTSTATUS( &status );

         /*copy the data into the single precision timeseries */
         for (k = 0; k < (int)dataDouble.data->length; k++) {
             dataSingle.data->data[k] = (REAL4)(dataINT4.data->data[k]);
         }     

         LALI4DestroyVector(&status,&dataINT4.data);
         TESTSTATUS( &status );

      } else if ( strstr(CLA.frameStructType,"INT8") ) {

         LALI8CreateVector(&status,&dataINT8.data,(UINT4)(CLA.T/dataINT8.deltaT +0.5));
         TESTSTATUS( &status );

         LALFrGetINT8TimeSeries(&status,&dataINT8,&chanin,framestream);
         TESTSTATUS( &status );

         /*copy the data into the single precision timeseries */
         for (k = 0; k < (int)dataDouble.data->length; k++) {
             dataSingle.data->data[k] = (REAL4)(dataINT8.data->data[k]);
         }     

         LALI8DestroyVector(&status,&dataINT8.data);
         TESTSTATUS( &status );

      } else {
        return 1;
      }      
    }
    else
    {

      chanin.type  = ADCDataChannel;
      LALFrGetREAL4TimeSeries(&status,&dataSingle,&chanin,framestream);
      TESTSTATUS( &status );

    }

    #if PRINTEXAMPLEDATA
        printf("\nExample dataSingle values after reading data from frames in ReadData:\n"); printExampleDataSingle();
    #endif

  } else {

    if(CLA.htdata)
    {
    
      chanin.type  = ProcDataChannel;
      LALFrGetREAL8TimeSeries(&status,&dataDouble,&chanin,framestream);
      TESTSTATUS( &status );

    }
    else if (CLA.frameStructType != NULL) 
    {
      /* 01/10/07 gam */  
      if ( strstr(CLA.frameStructType,"ADC_") ) {
         chanin.type  = ADCDataChannel;
      } else if ( strstr(CLA.frameStructType,"PROC_") ) {
         chanin.type  = ProcDataChannel;
      } else {
        return 1; 
      }
      if ( strstr(CLA.frameStructType,"REAL8") ) {

         LALFrGetREAL8TimeSeries(&status,&dataDouble,&chanin,framestream);
         TESTSTATUS( &status );

      } else if ( strstr(CLA.frameStructType,"REAL4") ) {      

         LALCreateVector(&status,&dataSingle.data,(UINT4)(CLA.T/dataSingle.deltaT +0.5));
         TESTSTATUS( &status );

         LALFrGetREAL4TimeSeries(&status,&dataSingle,&chanin,framestream);
         TESTSTATUS( &status );
 
         /*copy the data into the double precision timeseries */
         for (k = 0; k < (int)dataDouble.data->length; k++) {
             dataDouble.data->data[k] = dataSingle.data->data[k];
         }     

         LALDestroyVector(&status,&dataSingle.data);
         TESTSTATUS( &status );

      } else if ( strstr(CLA.frameStructType,"INT2") ) {

         LALI2CreateVector(&status,&dataINT2.data,(UINT4)(CLA.T/dataINT2.deltaT +0.5));
         TESTSTATUS( &status );

         LALFrGetINT2TimeSeries(&status,&dataINT2,&chanin,framestream);
         TESTSTATUS( &status );

         /*copy the data into the double precision timeseries */
         for (k = 0; k < (int)dataDouble.data->length; k++) {
             dataDouble.data->data[k] = (REAL8)(dataINT2.data->data[k]);
         }     

         LALI2DestroyVector(&status,&dataINT2.data);
         TESTSTATUS( &status );

      } else if ( strstr(CLA.frameStructType,"INT4") ) {

         LALI4CreateVector(&status,&dataINT4.data,(UINT4)(CLA.T/dataINT4.deltaT +0.5));
         TESTSTATUS( &status );

         LALFrGetINT4TimeSeries(&status,&dataINT4,&chanin,framestream);
         TESTSTATUS( &status );

         /*copy the data into the double precision timeseries */
         for (k = 0; k < (int)dataDouble.data->length; k++) {
             dataDouble.data->data[k] = (REAL8)(dataINT4.data->data[k]);
         }     

         LALI4DestroyVector(&status,&dataINT4.data);
         TESTSTATUS( &status );

      } else if ( strstr(CLA.frameStructType,"INT8") ) {

         LALI8CreateVector(&status,&dataINT8.data,(UINT4)(CLA.T/dataINT8.deltaT +0.5));
         TESTSTATUS( &status );

         LALFrGetINT8TimeSeries(&status,&dataINT8,&chanin,framestream);
         TESTSTATUS( &status );

         /*copy the data into the double precision timeseries */
         for (k = 0; k < (int)dataDouble.data->length; k++) {
             dataDouble.data->data[k] = (REAL8)(dataINT8.data->data[k]);
         }     

         LALI8DestroyVector(&status,&dataINT8.data);
         TESTSTATUS( &status );

      } else {
        return 1;
      }      
    }
    else
    {
      /* 11/02/05 gam; add next two lines */
      LALCreateVector(&status,&dataSingle.data,(UINT4)(CLA.T/dataSingle.deltaT +0.5));
      TESTSTATUS( &status );
      
      chanin.type  = ADCDataChannel;
      LALFrGetREAL4TimeSeries(&status,&dataSingle,&chanin,framestream);
      TESTSTATUS( &status );

      /*copy the data into the double precision timeseries */
      for (k = 0; k < (int)dataDouble.data->length; k++) {
          dataDouble.data->data[k] = dataSingle.data->data[k];
      }     

      LALDestroyVector(&status,&dataSingle.data);
      TESTSTATUS( &status );

    }

    #if PRINTEXAMPLEDATA
        printf("\nExample dataDouble values after reading data from frames in ReadData:\n"); printExampleDataDouble();
    #endif

  } /* END if(CLA.useSingle) else */

  return 0;
}
/*******************************************************************************/

/*******************************************************************************/
int HighPass(struct CommandLineArgsTag CLA)
{
  PassBandParamStruc filterpar;
  char tmpname[] = "Butterworth High Pass";

  filterpar.name  = tmpname;
  filterpar.nMax  = 10;
  filterpar.f2    = CLA.HPf;
  filterpar.a2    = 0.5;
  filterpar.f1    = -1.0;
  filterpar.a1    = -1.0;

  if (CLA.HPf > 0.0 )
    {
      if(CLA.useSingle) {

        /* High pass the time series */
        LALDButterworthREAL4TimeSeries(&status,&dataSingle,&filterpar);
        TESTSTATUS( &status );

        #if PRINTEXAMPLEDATA
            printf("\nExample dataSingle values after filtering data in HighPass:\n"); printExampleDataSingle();
        #endif
      } else {

        /* High pass the time series */
        LALButterworthREAL8TimeSeries(&status,&dataDouble,&filterpar);
        TESTSTATUS( &status );

        #if PRINTEXAMPLEDATA
            printf("\nExample dataDouble values after filtering data in HighPass:\n"); printExampleDataDouble();
        #endif
      }
    }

  return 0;
}
/*******************************************************************************/

/*******************************************************************************/
int CreateSFT(struct CommandLineArgsTag CLA)
{

  /* 11/19/05 gam */
  if(CLA.useSingle) {

      /* 11/02/05 gam; allocate container for SFT data */
      LALCCreateVector( &status, &fftDataSingle, dataSingle.data->length / 2 + 1 );
      TESTSTATUS( &status );  

      /* compute sft */
      XLAL_CHECK( XLALREAL4ForwardFFT( fftDataSingle, dataSingle.data, fftPlanSingle ) == XLAL_SUCCESS, XLAL_EFUNC );

      #if PRINTEXAMPLEDATA
        printf("\nExample real and imaginary value of fftDataSingle from CreateSFT:\n"); printExampleFFTData(CLA);
      #endif      
  } else {

      /* 11/02/05 gam; allocate container for SFT data */
      LALZCreateVector( &status, &fftDataDouble, dataDouble.data->length / 2 + 1 );
      TESTSTATUS( &status );  

      /* compute sft */
      XLAL_CHECK( XLALREAL8ForwardFFT( fftDataDouble, dataDouble.data, fftPlanDouble ) == XLAL_SUCCESS, XLAL_EFUNC );

      #if PRINTEXAMPLEDATA
        printf("\nExample real and imaginary value of fftDataDouble from CreateSFT:\n"); printExampleFFTData(CLA);
      #endif      
  }
      return 0;
}
/*******************************************************************************/
int WriteSFT(struct CommandLineArgsTag CLA)
{
  char sftname[256];
  char sftnameFinal[256]; /* 01/09/06 gam */
  char numSFTs[2]; /* 12/27/05 gam */
  char site[2];    /* 12/27/05 gam */
  char ifo[3];     /* 12/27/05 gam; allow 3rd charactor for null termination */
  int firstbin=(INT4)(FMIN*CLA.T+0.5), k;
  FILE *fpsft;
  int errorcode1=0,errorcode2=0;
  char gpstime[11]; /* 12/27/05 gam; allow for 10 digit GPS times and null termination */
  REAL4 rpw,ipw;

  /* 12/27/05 gam; set up the number of SFTs, site, and ifo as null terminated strings */
  numSFTs[0] = '1';
  numSFTs[1] = '\0'; /* null terminate */
  strncpy( site, CLA.ChannelName, 1 );
  site[1] = '\0'; /* null terminate */
  if (CLA.IFO != NULL) {
    strncpy( ifo, CLA.IFO, 2 );
  } else {
    strncpy( ifo, CLA.ChannelName, 2 );
  }
  ifo[2] = '\0'; /* null terminate */
  sprintf(gpstime,"%09d",gpsepoch.gpsSeconds);

  strcpy( sftname, CLA.SFTpath );
  /* 12/27/05 gam; add option to make directories based on gps time */
  if (CLA.makeGPSDirs > 0) {
     /* 12/27/05 gam; concat to the sftname the directory name based on GPS time; make this directory if it does not already exist */
     mkSFTDir(sftname, site, numSFTs, ifo, CLA.stringT, CLA.miscDesc, gpstime, CLA.makeGPSDirs);
  }

  /* 01/09/06 gam; sftname will be temporary; will move to sftnameFinal. */
  if(CLA.makeTmpFile) {
    /* set up sftnameFinal with usual SFT name */
    strcpy(sftnameFinal,sftname);
    strcat(sftnameFinal,"/SFT_");
    strcat(sftnameFinal,ifo);
    strcat(sftnameFinal,".");
    strcat(sftnameFinal,gpstime);
    /* sftname begins with . and ends in .tmp */
    strcat(sftname,"/.SFT_");
    strcat(sftname,ifo);
    strcat(sftname,".");
    strcat(sftname,gpstime);
    strcat(sftname,".tmp");
  } else {     
    strcat(sftname,"/SFT_");
    strcat(sftname,ifo);
    strcat(sftname,".");
    strcat(sftname,gpstime);
  }  

  /* open SFT file for writing */
  fpsft=tryopen(sftname,"w");

  /* write header */
  header.endian=1.0;
  header.gps_sec=gpsepoch.gpsSeconds;
  header.gps_nsec=gpsepoch.gpsNanoSeconds;
  header.tbase=CLA.T;
  header.firstfreqindex=firstbin;
  header.nsamples=(INT4)(DF*CLA.T+0.5);
  if (1!=fwrite((void*)&header,sizeof(header),1,fpsft)){
    fprintf(stderr,"Error in writing header into file %s!\n",sftname);
    return 7;
  }

  /* 11/19/05 gam */
  if(CLA.useSingle) {
    /* Write SFT */
    for (k=0; k<header.nsamples; k++)
    {
      rpw=((REAL4)(((REAL8)DF)/(0.5*(REAL8)(1/dataSingle.deltaT))))
	* crealf(fftDataSingle->data[k+firstbin]);
      ipw=((REAL4)(((REAL8)DF)/(0.5*(REAL8)(1/dataSingle.deltaT))))
	* cimagf(fftDataSingle->data[k+firstbin]);
      /* 06/26/07 gam; use finite to check that data does not contains a non-FINITE (+/- Inf, NaN) values */
      #if CHECKFORINFINITEANDNANS
        if (!isfinite(rpw) || !isfinite(ipw)) {
          fprintf(stderr, "Infinite or NaN data at freq bin %d.\n", k);
          return 7;
        }
      #endif
      errorcode1=fwrite((void*)&rpw, sizeof(REAL4),1,fpsft);
      errorcode2=fwrite((void*)&ipw, sizeof(REAL4),1,fpsft);
    }
    #if PRINTEXAMPLEDATA
        printf("\nExample real and imaginary SFT values going to file from fftDataSingle in WriteSFT:\n"); printExampleSFTDataGoingToFile(CLA);
    #endif     
    LALCDestroyVector( &status, &fftDataSingle );
    TESTSTATUS( &status );
  } else {    
    /* Write SFT */
    for (k=0; k<header.nsamples; k++)
    {
      rpw=(((REAL8)DF)/(0.5*(REAL8)(1/dataDouble.deltaT))) 
	* creal(fftDataDouble->data[k+firstbin]);
      ipw=(((REAL8)DF)/(0.5*(REAL8)(1/dataDouble.deltaT))) 
	* cimag(fftDataDouble->data[k+firstbin]);
      /* 06/26/07 gam; use finite to check that data does not contains a non-FINITE (+/- Inf, NaN) values */
      #if CHECKFORINFINITEANDNANS
        if (!isfinite(rpw) || !isfinite(ipw)) {
          fprintf(stderr, "Infinite or NaN data at freq bin %d.\n", k);
          return 7;
        }
      #endif
      errorcode1=fwrite((void*)&rpw, sizeof(REAL4),1,fpsft);
      errorcode2=fwrite((void*)&ipw, sizeof(REAL4),1,fpsft);
    }
    #if PRINTEXAMPLEDATA
        printf("\nExample real and imaginary SFT values going to file from fftDataDouble in WriteSFT:\n"); printExampleSFTDataGoingToFile(CLA);
    #endif
    LALZDestroyVector( &status, &fftDataDouble );
    TESTSTATUS( &status );
  }

  /* Check that there were no errors while writing SFTS */
  if (errorcode1-1 || errorcode2-1)
    {
      fprintf(stderr,"Error in writing data into SFT file %s!\n",sftname);
      return 8;
    }
  
  fclose(fpsft);

  /* 01/09/06 gam; sftname is temporary; move to sftnameFinal. */
  if(CLA.makeTmpFile) {  
    mvFilenames(sftname,sftnameFinal);
  }

  return 0;
}
/*******************************************************************************/

/*******************************************************************************/
/* 12/28/05 gam; write out version 2 SFT */
int WriteVersion2SFT(struct CommandLineArgsTag CLA) 
{
  char sftname[256];
  char sftFilename[256];
  char sftnameFinal[256]; /* 01/09/06 gam */
  char numSFTs[2]; /* 12/27/05 gam */
  char site[2];    /* 12/27/05 gam */
  char ifo[3];     /* 12/27/05 gam; allow 3rd charactor for null termination */
  int firstbin=(INT4)(FMIN*CLA.T+0.5), k;
  char gpstime[11]; /* 12/27/05 gam; allow for 10 digit GPS times and null termination */
  SFTtype *oneSFT = NULL;
  INT4 nBins = (INT4)(DF*CLA.T+0.5);
  REAL4 singleDeltaT = 0.0; /* 01/05/06 gam */
  REAL8 doubleDeltaT = 0.0; /* 01/05/06 gam */

  /* 12/27/05 gam; set up the number of SFTs, site, and ifo as null terminated strings */
  numSFTs[0] = '1';
  numSFTs[1] = '\0'; /* null terminate */
  strncpy( site, CLA.ChannelName, 1 );
  site[1] = '\0'; /* null terminate */
  if (CLA.IFO != NULL) {
    strncpy( ifo, CLA.IFO, 2 );
  } else {  
    strncpy( ifo, CLA.ChannelName, 2 );
  }
  ifo[2] = '\0'; /* null terminate */
  sprintf(gpstime,"%09d",gpsepoch.gpsSeconds);

  strcpy( sftname, CLA.SFTpath );
  /* 12/27/05 gam; add option to make directories based on gps time */
  if (CLA.makeGPSDirs > 0) {
     /* 12/27/05 gam; concat to the sftname the directory name based on GPS time; make this directory if it does not already exist */
     mkSFTDir(sftname, site, numSFTs, ifo, CLA.stringT, CLA.miscDesc, gpstime, CLA.makeGPSDirs);
  }

  strcat(sftname,"/");
  mkSFTFilename(sftFilename, site, numSFTs, ifo, CLA.stringT, CLA.miscDesc, gpstime);
  /* 01/09/06 gam; sftname will be temporary; will move to sftnameFinal. */
  if(CLA.makeTmpFile) {
    /* set up sftnameFinal with usual SFT name */
    strcpy(sftnameFinal,sftname);
    strcat(sftnameFinal,sftFilename);
    /* sftname begins with . and ends in .tmp */
    strcat(sftname,".");
    strcat(sftname,sftFilename);
    strcat(sftname,".tmp");
  } else {
    strcat(sftname,sftFilename);
  }  

  /* make container to store the SFT data */
  XLAL_CHECK( ( oneSFT = XLALCreateSFT ( ((UINT4)nBins)) ) != NULL, XLAL_EFUNC );
  
  /* copy the data to oneSFT */
  strcpy(oneSFT->name,ifo);
  oneSFT->epoch.gpsSeconds=gpsepoch.gpsSeconds;
  oneSFT->epoch.gpsNanoSeconds=gpsepoch.gpsNanoSeconds;
  oneSFT->f0 = FMIN;
  oneSFT->deltaF = 1.0/((REAL8)CLA.T);
  oneSFT->data->length=nBins;
  
  if(CLA.useSingle) {
    /* singleDeltaT = ((REAL4)dataSingle.deltaT); */ /* 01/05/06 gam; and normalize SFTs using this below */
    singleDeltaT = (REAL4)(dataSingle.deltaT/winFncRMS); /* include 1 over window function RMS */
    for (k=0; k<nBins; k++)
    {
      oneSFT->data->data[k] = (((REAL4) singleDeltaT) * fftDataSingle->data[k+firstbin]);
      /* 06/26/07 gam; use finite to check that data does not contains a non-FINITE (+/- Inf, NaN) values */
      #if CHECKFORINFINITEANDNANS
        if (!isfinite(crealf(oneSFT->data->data[k])) || !isfinite(cimagf(oneSFT->data->data[k]))) {
          fprintf(stderr, "Infinite or NaN data at freq bin %d.\n", k);
          return 7;
        }
      #endif      
    }
    #if PRINTEXAMPLEDATA
        printf("\nExample real and imaginary SFT values going to file from fftDataSingle in WriteVersion2SFT:\n"); printExampleVersion2SFTDataGoingToFile(CLA,oneSFT);
    #endif
    LALCDestroyVector( &status, &fftDataSingle );
    TESTSTATUS( &status );
  } else {
    /*doubleDeltaT = ((REAL8)dataDouble.deltaT); */ /* 01/05/06 gam; and normalize SFTs using this below */
    doubleDeltaT = (REAL8)(dataDouble.deltaT/winFncRMS); /* include 1 over window function RMS */
    for (k=0; k<nBins; k++)
    {
      oneSFT->data->data[k] = crectf( doubleDeltaT*creal(fftDataDouble->data[k+firstbin]), doubleDeltaT*cimag(fftDataDouble->data[k+firstbin]) );
      /* 06/26/07 gam; use finite to check that data does not contains a non-FINITE (+/- Inf, NaN) values */
      #if CHECKFORINFINITEANDNANS
        if (!isfinite(crealf(oneSFT->data->data[k])) || !isfinite(cimagf(oneSFT->data->data[k]))) {
          fprintf(stderr, "Infinite or NaN data at freq bin %d.\n", k);
          return 7;
        }
      #endif
    }
    #if PRINTEXAMPLEDATA
        printf("\nExample real and imaginary SFT values going to file from fftDataDouble in WriteVersion2SFT:\n"); printExampleVersion2SFTDataGoingToFile(CLA,oneSFT);
    #endif
    LALZDestroyVector( &status, &fftDataDouble );
    TESTSTATUS( &status );
  }  

  /* write the SFT */
  XLAL_CHECK( XLALWriteSFT2NamedFile(oneSFT, sftname, "unknown" /* FIXME */, 0, CLA.commentField) == XLAL_SUCCESS, XLAL_EFUNC );

  /* 01/09/06 gam; sftname is temporary; move to sftnameFinal. */
  if(CLA.makeTmpFile) {  
    mvFilenames(sftname,sftnameFinal);
  }

  XLALDestroySFT (oneSFT);

  return 0;
}
/*******************************************************************************/

/*******************************************************************************/
int FreeMem(struct CommandLineArgsTag CLA)
{

  LALFrClose(&status,&framestream);
  TESTSTATUS( &status );

  /* 11/19/05 gam */
  if(CLA.useSingle) {
    LALDestroyVector(&status,&dataSingle.data);
    TESTSTATUS( &status );
    XLALDestroyREAL4FFTPlan( fftPlanSingle );
  } else {
    LALDDestroyVector(&status,&dataDouble.data);
    TESTSTATUS( &status );
    XLALDestroyREAL8FFTPlan( fftPlanDouble );
  }

  LALCheckMemoryLeaks();
 
  return 0;
}
/*******************************************************************************/
