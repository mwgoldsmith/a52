
/* 
 *  srfftp.h
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
 *  srfftp.h is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  srfftp.h is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef SRFFTP_H__
#define SRFFTP_H__

static complex_t delta16[4] = 
 { {1.00000000000000,  0.00000000000000},
   {0.92387953251129, -0.38268343236509},
   {0.70710678118655, -0.70710678118655},
   {0.38268343236509, -0.92387953251129}};

static complex_t delta16_3[4] = 
 { {1.00000000000000,  0.00000000000000},
   {0.38268343236509, -0.92387953251129},
   {-0.70710678118655, -0.70710678118655},
   {-0.92387953251129, 0.38268343236509}};

static complex_t delta32[8] = 
 { {1.00000000000000,  0.00000000000000},
   {0.98078528040323, -0.19509032201613},
   {0.92387953251129, -0.38268343236509},
   {0.83146961230255, -0.55557023301960},
   {0.70710678118655, -0.70710678118655},
   {0.55557023301960, -0.83146961230255},
   {0.38268343236509, -0.92387953251129},
   {0.19509032201613, -0.98078528040323}};

static complex_t delta32_3[8] = 
 { {1.00000000000000,  0.00000000000000},
   {0.83146961230255, -0.55557023301960},
   {0.38268343236509, -0.92387953251129},
   {-0.19509032201613, -0.98078528040323},
   {-0.70710678118655, -0.70710678118655},
   {-0.98078528040323, -0.19509032201613},
   {-0.92387953251129, 0.38268343236509},
   {-0.55557023301960, 0.83146961230255}};

static complex_t delta64[16] = 
 { {1.00000000000000,  0.00000000000000},
   {0.99518472667220, -0.09801714032956},
   {0.98078528040323, -0.19509032201613},
   {0.95694033573221, -0.29028467725446},
   {0.92387953251129, -0.38268343236509},
   {0.88192126434836, -0.47139673682600},
   {0.83146961230255, -0.55557023301960},
   {0.77301045336274, -0.63439328416365},
   {0.70710678118655, -0.70710678118655},
   {0.63439328416365, -0.77301045336274},
   {0.55557023301960, -0.83146961230255},
   {0.47139673682600, -0.88192126434835},
   {0.38268343236509, -0.92387953251129},
   {0.29028467725446, -0.95694033573221},
   {0.19509032201613, -0.98078528040323},
   {0.09801714032956, -0.99518472667220}};

static complex_t delta64_3[16] = 
 { {1.00000000000000,  0.00000000000000},
   {0.95694033573221, -0.29028467725446},
   {0.83146961230255, -0.55557023301960},
   {0.63439328416365, -0.77301045336274},
   {0.38268343236509, -0.92387953251129},
   {0.09801714032956, -0.99518472667220},
   {-0.19509032201613, -0.98078528040323},
   {-0.47139673682600, -0.88192126434836},
   {-0.70710678118655, -0.70710678118655},
   {-0.88192126434835, -0.47139673682600},
   {-0.98078528040323, -0.19509032201613},
   {-0.99518472667220, 0.09801714032956},
   {-0.92387953251129, 0.38268343236509},
   {-0.77301045336274, 0.63439328416365},
   {-0.55557023301960, 0.83146961230255},
   {-0.29028467725446, 0.95694033573221}};

static complex_t delta128[32] = 
 { {1.00000000000000,  0.00000000000000},
   {0.99879545620517, -0.04906767432742},
   {0.99518472667220, -0.09801714032956},
   {0.98917650996478, -0.14673047445536},
   {0.98078528040323, -0.19509032201613},
   {0.97003125319454, -0.24298017990326},
   {0.95694033573221, -0.29028467725446},
   {0.94154406518302, -0.33688985339222},
   {0.92387953251129, -0.38268343236509},
   {0.90398929312344, -0.42755509343028},
   {0.88192126434836, -0.47139673682600},
   {0.85772861000027, -0.51410274419322},
   {0.83146961230255, -0.55557023301960},
   {0.80320753148064, -0.59569930449243},
   {0.77301045336274, -0.63439328416365},
   {0.74095112535496, -0.67155895484702},
   {0.70710678118655, -0.70710678118655},
   {0.67155895484702, -0.74095112535496},
   {0.63439328416365, -0.77301045336274},
   {0.59569930449243, -0.80320753148064},
   {0.55557023301960, -0.83146961230255},
   {0.51410274419322, -0.85772861000027},
   {0.47139673682600, -0.88192126434835},
   {0.42755509343028, -0.90398929312344},
   {0.38268343236509, -0.92387953251129},
   {0.33688985339222, -0.94154406518302},
   {0.29028467725446, -0.95694033573221},
   {0.24298017990326, -0.97003125319454},
   {0.19509032201613, -0.98078528040323},
   {0.14673047445536, -0.98917650996478},
   {0.09801714032956, -0.99518472667220},
   {0.04906767432742, -0.99879545620517}};

static complex_t delta128_3[32] = 
 { {1.00000000000000,  0.00000000000000},
   {0.98917650996478, -0.14673047445536},
   {0.95694033573221, -0.29028467725446},
   {0.90398929312344, -0.42755509343028},
   {0.83146961230255, -0.55557023301960},
   {0.74095112535496, -0.67155895484702},
   {0.63439328416365, -0.77301045336274},
   {0.51410274419322, -0.85772861000027},
   {0.38268343236509, -0.92387953251129},
   {0.24298017990326, -0.97003125319454},
   {0.09801714032956, -0.99518472667220},
   {-0.04906767432742, -0.99879545620517},
   {-0.19509032201613, -0.98078528040323},
   {-0.33688985339222, -0.94154406518302},
   {-0.47139673682600, -0.88192126434836},
   {-0.59569930449243, -0.80320753148065},
   {-0.70710678118655, -0.70710678118655},
   {-0.80320753148065, -0.59569930449243},
   {-0.88192126434835, -0.47139673682600},
   {-0.94154406518302, -0.33688985339222},
   {-0.98078528040323, -0.19509032201613},
   {-0.99879545620517, -0.04906767432742},
   {-0.99518472667220, 0.09801714032956},
   {-0.97003125319454, 0.24298017990326},
   {-0.92387953251129, 0.38268343236509},
   {-0.85772861000027, 0.51410274419322},
   {-0.77301045336274, 0.63439328416365},
   {-0.67155895484702, 0.74095112535496},
   {-0.55557023301960, 0.83146961230255},
   {-0.42755509343028, 0.90398929312344},
   {-0.29028467725446, 0.95694033573221},
   {-0.14673047445536, 0.98917650996478}};

#define HSQRT2 0.707106781188;

#define TRANSZERO(A0,A4,A8,A12) { \
  u_r = wTB[0].re; \
  v_i = u_r - wTB[k*2].re; \
  u_r += wTB[k*2].re; \
  u_i = wTB[0].im; \
  v_r = wTB[k*2].im - u_i; \
  u_i += wTB[k*2].im; \
  a_r = A0.re; \
  a_i = A0.im; \
  a1_r = a_r; \
  a1_r += u_r; \
  A0.re = a1_r; \
  a_r -= u_r; \
  A8.re = a_r; \
  a1_i = a_i; \
  a1_i += u_i; \
  A0.im = a1_i; \
  a_i -= u_i; \
  A8.im = a_i; \
  a1_r = A4.re; \
  a1_i = A4.im; \
  a_r = a1_r; \
  a_r -= v_r; \
  A4.re = a_r; \
  a1_r += v_r; \
  A12.re = a1_r; \
  a_i = a1_i; \
  a_i -= v_i; \
  A4.im = a_i; \
  a1_i += v_i; \
  A12.im = a1_i; \
  }

#define TRANSHALF_16(A2,A6,A10,A14) {\
  u_r = wTB[2].re; \
  a_r = u_r; \
  u_i = wTB[2].im; \
  u_r += u_i; \
  u_i -= a_r; \
  a_r = wTB[6].re; \
  a1_r = a_r; \
  a_i = wTB[6].im; \
  a_r = a_i - a_r; \
  a_i += a1_r; \
  v_i = u_r - a_r; \
  u_r += a_r; \
  v_r = u_i + a_i; \
  u_i -= a_i; \
  v_i *= HSQRT2; \
  v_r *= HSQRT2; \
  u_r *= HSQRT2; \
  u_i *= HSQRT2; \
  a_r = A2.re; \
  a_i = A2.im; \
  a1_r = a_r; \
  a1_r += u_r; \
  A2.re = a1_r; \
  a_r -= u_r; \
  A10.re = a_r; \
  a1_i = a_i; \
  a1_i += u_i; \
  A2.im = a1_i; \
  a_i -= u_i; \
  A10.im = a_i; \
  a1_r = A6.re; \
  a1_i = A6.im;  \
  a_r = a1_r; \
  a1_r += v_r; \
  A6.re = a1_r; \
  a_r -= v_r; \
  A14.re = a_r; \
  a_i = a1_i; \
  a1_i -= v_i; \
  A6.im = a1_i; \
  a_i += v_i; \
  A14.im = a_i; \
  }

#define TRANS(A1,A5,A9,A13,WT,WB,D,D3) { \
  u_r = WT.re; \
  a_r = u_r; \
  a_r *= D.im; \
  u_r *= D.re; \
  a_i = WT.im; \
  a1_i = a_i; \
  a1_i *= D.re; \
  a_i *= D.im; \
  u_r -= a_i; \
  u_i = a_r; \
  u_i += a1_i; \
  a_r = WB.re; \
  a1_r = a_r; \
  a1_r *= D3.re; \
  a_r *= D3.im; \
  a_i = WB.im; \
  a1_i = a_i; \
  a_i *= D3.re; \
  a1_i *= D3.im; \
  a1_r -= a1_i; \
  a_r += a_i; \
  v_i = u_r - a1_r; \
  u_r += a1_r; \
  v_r = a_r - u_i; \
  u_i += a_r; \
  a_r = A1.re; \
  a_i = A1.im; \
  a1_r = a_r; \
  a1_r += u_r; \
  A1.re = a1_r; \
  a_r -= u_r; \
  A9.re = a_r; \
  a1_i = a_i; \
  a1_i += u_i; \
  A1.im = a1_i; \
  a_i -= u_i; \
  A9.im = a_i; \
  a1_r = A5.re; \
  a1_i = A5.im;  \
  a_r = a1_r; \
  a1_r -= v_r; \
  A5.re = a1_r; \
  a_r += v_r; \
  A13.re = a_r; \
  a_i = a1_i; \
  a1_i -= v_i; \
  A5.im = a1_i; \
  a_i += v_i; \
  A13.im = a_i; \
  }

#endif
