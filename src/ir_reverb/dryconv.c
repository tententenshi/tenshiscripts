/* 
 * dryconv.c - Fast convolution between dry source and impulse response
 *  
 * Copyright (c) 2000 Mitsubishi Research Institute, Inc.
 * All rights reserved.
 *  
 * This is free software with ABSOLUTELY NO WARRANTY.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA
 */

/**
 * Command Name : dryconv
 * Title: Fast convolution between dry source and impulse response
 * Description:
 *    - Reconstruct sound signal by convolution dry source signal and
 *      impulse response. 
 *    
 *    - FFT based fast convolution.
 *    
 *    - Impulse response is automatically normalized by its total power.
 *    
 *    - File format:
 *        dry source        (input)  : 16bit integer
 *        impulse response  (input)  : 32bit float
 *        reproduced signal (output) : 16bit integer
 *    
 * Version: 0.9.0
 *
 * Required library:
 *   The GNU Scientific Library for numerical analysis (gsl-0.6)
 *   http://sourceware.cygnus.com/gsl/
 *
 * How to compile:
 *   $ gcc -O2 -o dryconv dryconv.c -lgsl -lgslcblas -lm
 *
 * Author : Kazuo Hiyane (Mitsubishi Research Institute, Inc.)
 *
 * ChangeLog:
 *    2000/12/01 Ver.1.0.0 add licence notice (GPL)
 *    2000/10/13 Ver.0.9.1 add algorithm descriptions
 *    2000/09/06 Ver.0.9.0 created  
 *
 */
#define VERSION "1.0.0"

#include <stdio.h>
#include <math.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_fft_complex.h>

/* accessor for real and imaginary part of complex array */
#define REAL(z,i) ((z)[2*(i)])
#define IMAG(z,i) ((z)[2*(i)+1])

/* degug macro */
#define FPERR1(s)          if(debug) fprintf(stderr,s)
#define FPERR2(s,a)        if(debug) fprintf(stderr,s,a)
#define FPERR3(s,a,b)      if(debug) fprintf(stderr,s,a,b)
#define FPERR4(s,a,b,c)    if(debug) fprintf(stderr,s,a,b,c)
#define FPERR5(s,a,b,c,d)  if(debug) fprintf(stderr,s,a,b,c,d)

/* print debug information (-debug) */
int debug = 0;

/* function prototype declaration */
short* read_drysource(char*, int*);
float* read_impulse_response(char*, int*);
void write_signal(char*, short*, int);
void usage(char*);

/************************************************************************
 * Algorithm
 * 
 * 1. Definition of convolution
 *   dry source signal : x(t) : t=0...N-1
 *   impulse response  : w(t) : t=0...M-1
 *   reproduced signal : y(t) : t=0...N+M-1
 * 
 *           M
 *   y(t) = \sum x(t-i) w(i)
 *          i=1
 * 
 *   Note: length M of impulse response should be 2д╬╢╥╛ш
 *         by addding zeros at the end of array.
 * 
 * 2. Convolution by FFT
 * 
 *  (1) Split dry source signal x(t) into signals x_k(t), k=1...K that 
 *      each length is M.
 * 
 *  (2) For each dry sources x_k(t), k=1...K and impulse response w(t), add zeros 
 *      at the end so that its length is 2M.
 * 
 *  (3) Apply FFT to each dry sources x_k(t) and impulse response w(t).
 * 
 *      X_k(f) = F[x_k(t)],    k=1...K
 *      W(f)   = F[w(t)],     
 * 
 *  (4) Multiply dry sources X_k(f) and impulse response W(f) at each frequency 
 *      to get reproduced signals Y_k(f).
 * 
 *      Y_k(f) = X_k(f) W(f),  f=0...2M-1, k=1...K
 * 
 *  (5) Apply inverse FFT to each reproduced signals Y_k(f).
 * 
 *      y_k(t) = F'[Y_k(f)],   k=1...K
 * 
 *  (6) Add all reproduced signals by shifting M as shown below.
 *      
 *                               вл             N          вк
 *  dry source signal  x(t)     |*******|*******|*******|****---|  (*** : non-zero)
 *                               вл M вк вл M вк вл M вк вл M вк
 *		             
 *                               вл M вк вл M вк
 *  dry source signal1 x_1(t)   |*******|-------|
 *  impulse response   w(t)     |******-|-------|
 *  reproduced signal1 y_1(t)   |**************-|
 *                                       вл M вк вл M вк
 *  dry source signal2 x_2(t)           |*******|-------|
 *  impulse response   w(t)             |******-|-------|
 *  reproduced signal2 y_2(t)           |**************-|
 *                                               вл M вк вл M вк
 *  dry source signal3 x_3(t)                   |*******|-------|
 *  impulse response   w(t)                     |******-|-------|
 *  reproduced signal3 y_3(t)                   |**************-|
 *                                                       вл M вк вл M вк
 *  dry source signal4 x_4(t)                           |****---|-------|
 *  impulse response   w(t)                             |******-|-------|
 *  reproduced signal4 y_4(t)                           |***********----|
 *		             
 *  reproduced signal  y(t)     |*******|*******|*******|*******|***----|
 *                               вл M вк вл M вк вл M вк вл M вк вл M вк
 * 
 ************************************************************************/
int
main (int argc, char** argv)
{
  int i;

  char* dryfile = 0;		/* dry source filename */
  char* impfile = 0;		/* impulse response filename */
  char* outfile = 0;		/* output filename */

  short* drydata;		/* array for dry source */
  float* impdata;		/* array for impulse response */
  short* outdata;		/* array for output reproduced signal */
  int    drylen;		/* length of dry source */              
  int    implen;		/* length of impulse response */        
  int    outlen;		/* length of output reproduced signal */
  int    arglen = 0;		/* length of output reproduced signal by command argment*/

  double* dryfft;		/* FFT array for dry source */
  double* impfft;		/* FFT array for impulse response */
  double* outfft;		/* FFT array for output reproduced signal */
  int     M;			/* FFT size */
  int     mm;			/* start index of frame */

  double power = 0;		/* total power of impulse response */

  /**
   * command arguments analysis
   */
  for( i = 1; i < argc; i++ ) {
    if( ! strcmp( argv[i], "-h" ) || ! strcmp( argv[i], "--help" ) ) {
      usage( argv[0] );

      /* output length of data [point] */
    } else if( ! strcmp( argv[i], "-len" ) ) {
      if( ++i >= argc || 1 != sscanf( argv[i], "%d", &arglen ) ) usage( argv[0] );

      /* print debug information */
    } else if( ! strcmp( argv[i], "-debug" ) ) {
      debug = 1;

      /* input filenames and output filename */
    } else {
      if      (dryfile == 0) { dryfile = argv[i]; }
      else if (impfile == 0) { impfile = argv[i]; }
      else                   { outfile = argv[i]; }
    }
  }

  if (dryfile == 0 || impfile == 0)
    usage( argv[0] );

  /**
   * read dry source and impulse response
   */
  drydata = read_drysource( dryfile, &drylen );
  impdata = read_impulse_response( impfile, &implen );

  /**
   * convolution process
   */
  for (M = 1024; M < implen; M *= 2) ; /* FFT size */

  FPERR3("Dry source      : %6d[pts] (%s)\n", drylen, dryfile);
  FPERR3("Impulse response: %6d[pts] (%s)\n", implen, impfile);
  FPERR2("FFT size        : %6d[pts]\n", M);

  /* normalization factor for impulse response */
  for (i = 0; i < implen; i++ ) power += impdata[i] * impdata[i];
  power = sqrt(power);
  FPERR2("Power of impulse: %.3g\n",power);

  /* alloc FFT buffers */
  impfft = (double*)malloc(sizeof(double) * (2*M) * 2);
  dryfft = (double*)malloc(sizeof(double) * (2*M) * 2);
  outfft = (double*)malloc(sizeof(double) * (2*M) * 2);

  /* preparation of impulse response */
  for (i = 0;      i < implen; i++ ) REAL(impfft,i) = impdata[i] / power;
  for (i = implen; i < 2*M;    i++ ) REAL(impfft,i) = 0;
  for (i = 0;      i < 2*M;    i++ ) IMAG(impfft,i) = 0;

  /* FFT of impulse response */
  gsl_fft_complex_radix2_forward (impfft, 1, 2*M); 

  /* preparation of output reproduced signal */
  outlen = (drylen/M + 2) * M; 
  outdata = (short*)malloc(sizeof(short) * outlen);
  for (i = 0; i < outlen; i++) outdata[i] = 0;

  if (arglen == 0 || arglen > outlen) arglen = drylen + implen;
  FPERR4("Reproduced sig  : %6d[pts] (%s)  output: %d[pts]\n",
	 outlen,(outfile?outfile:"stdout"),arglen);

  /**
   * convolution iteration
   */
  for (mm = 0; mm < drylen; mm += M) {

    /* preparation of dry source */
    for (i = 0; i < 2*M; i++ ) REAL(dryfft,i) = IMAG(dryfft,i) = 0;
    for (i = 0; i < M && mm+i < drylen; i++ ) REAL(dryfft,i) = drydata[mm+i];

    /* FFT of dry source */
    gsl_fft_complex_radix2_forward (dryfft, 1, 2*M); 

    /* the product of dry source and impulse response at each frequency */
    for (i = 0; i < 2*M; i++) {
      REAL(outfft,i) = REAL(dryfft,i) * REAL(impfft,i) - IMAG(dryfft,i) * IMAG(impfft,i);
      IMAG(outfft,i) = REAL(dryfft,i) * IMAG(impfft,i) + IMAG(dryfft,i) * REAL(impfft,i);
    }

    /* inverse FFT */
    gsl_fft_complex_radix2_inverse (outfft, 1, 2*M);

    /* add to reproduced signal */
    for (i = 0; i < 2*M; i++) outdata[mm+i] += (short)REAL(outfft,i);
  }

  /**
   * Write to float format file
   */
  write_signal( outfile, outdata, arglen );
}

/************************************************************************
 * read dry source file - 16 bits integer raw format
 *
 *     args filename : [I/ ] filename
 *          len      : [ /O] length of dry source
 *     return        : array data of dry source
 ***********************************************************************/
short* read_drysource
(char* filename, int* len)
{
  FILE* fp;
  short* data;			/* short array for dry source */
  int curlen = 1024;		/* length for reading */
  int n;

  /**
   * Read from binary format file to array
   * 
   *        <----curlen----> <----curlen---->
   *  data [*|*|*|*|*|*|*|*|+|+|+|+|+|+|+|+]
   *    * : already read
   *    + : to be read at the next fread()
   */

  /* open input file */
  if (filename && (fp = fopen(filename,"r")) == 0)
    { perror(filename); exit(1); }

  /* first, read 1k words */
  data = (short*)malloc(sizeof(short) * curlen); 
  *len = fread(data, sizeof(short), curlen, fp);

  while ( 1 ) {
    /* extend array length twice */
    data = (short*)realloc(data, sizeof(short) * curlen * 2); 
    /* read from half of array */
    n = fread(data + curlen, sizeof(short), curlen, fp);
    *len += n;
    if (n < curlen) break;

    curlen *= 2;
  }

  if (filename) fclose(fp);

  return data;
}

/************************************************************************
 * read impulse response file - 32 bits float raw format
 *
 *     args filename : [I/ ] filename
 *          len      : [ /O] length of impulse response
 *     return        : array data of  impulse response
 ***********************************************************************/
float*
read_impulse_response
(char* filename, int* len)
{
  FILE* fp;
  float* data;			/* short array for dry source */
  int curlen = 1024;		/* length for reading */
  int n;

  /* open input file */
  if (filename && (fp = fopen(filename,"r")) == 0)
    { perror(filename); exit(1); }

  /* first, read 1k words */
  data = (float*)malloc(sizeof(float) * curlen); 
  *len = fread(data, sizeof(float), curlen, fp);

  while ( 1 ) {
    /* extend array length twice */
    data = (float*)realloc(data, sizeof(float) * curlen * 2); 
    /* read from half of array */
    n = fread(data + curlen, sizeof(float), curlen, fp);
    *len += n;
    if (n < curlen) break;

    curlen *= 2;
  }

  if (filename) fclose(fp);

  return data;
}

/************************************************************************
 * write reproduced signal - 16 bits integer raw format
 *
 *     args filename : [I/ ] filename
 *          data     : [I/ ] array data of signal
 *          len      : [I/ ] length of signal
 ***********************************************************************/
void
write_signal
(char* filename, short* data, int len)
{
  FILE* fp = stdout;		/* file pointer for output */

  /* open output file */
  if (filename && (fp = fopen(filename,"w")) == 0)
    { perror(filename); exit(1); }

  /* Write float array to output file */
  fwrite(data, sizeof(short), len, fp);

  if (filename) fclose(fp);
}

/************************************************************************
 * print usage
 ***********************************************************************/
void
usage(char* com)
{
  fprintf(stderr,"Copyright (c) 2000 Mitsubishi Research Institute, Inc. All rights reserved.\n");
  fprintf(stderr,"  Fast convolution between dry source and impulse response\n");
  fprintf(stderr,"  dryconv Ver.%s\n",VERSION);
  fprintf(stderr,"\n");
  fprintf(stderr,"  usage : %s [option] dryfile impulsefile [outfile]\n", com);
  fprintf(stderr,"\n");
  fprintf(stderr,"  (necessaries)\n");
  fprintf(stderr,"    dryfile     : dry source file        - 16bit int\n");
  fprintf(stderr,"    impulsefile : impulse response file  - 32bit float\n");
  fprintf(stderr,"  (options)\n");
  fprintf(stderr,"    -len ???    : output length of data [point]\n");
  fprintf(stderr,"    outfile     : reproduced signal file - 16bit int\n");
  fprintf(stderr,"\n");
  exit(0);
}

