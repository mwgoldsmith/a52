/* 
 *  srfft.h
 *
 *  Copyright (C) Yuqing Deng <Yuqing_Deng@brown.edu> - April 2000
 *
 *  64 and 128 point split radix fft for ac3dec
 *
 *  The algorithm is desribed in the book:
 *  "Computational Frameworks of the Fast Fourier Transform".
 *
 *  The ideas and the the organization of code borrowed from djbfft written by
 *  D. J. Bernstein <djb@cr.py.to>.  djbff can be found at 
 *  http://cr.yp.to/djbfft.html.
 *
 *  srfft.h is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  srfft.h is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef SRFFT_H__
#define SRFFT_H__

typedef struct complex
{
  float re;
  float im;
} complex_t;

void fft_asmb16(complex_t *x, complex_t *wTB);
void fft_asmb(int n, complex_t *x, complex_t *wTB, const complex_t *d, const complex_t *d_3);

void fft_64p(complex_t *x);
void fft_128p(complex_t *x);

#endif /* SRFFT_H__ */
