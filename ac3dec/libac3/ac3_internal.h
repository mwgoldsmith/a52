/* 
 *    ac3_internal.h
 *
 *	Copyright (C) Aaron Holtzman - May 1999
 *
 *  This file is part of ac3dec, a free Dolby AC-3 stream decoder.
 *	
 *  ac3dec is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  ac3dec is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *   
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *
 */

#ifndef __GNUC__
#define inline 
#endif

/* Exponent strategy constants */
#define EXP_REUSE (0)
#define EXP_D15   (1)
#define EXP_D25   (2)
#define EXP_D45   (3)

/* Delta bit allocation constants */
#define DELTA_BIT_REUSE (0)
#define DELTA_BIT_NEW (1)
#define DELTA_BIT_NONE (2)
#define DELTA_BIT_RESERVED (3)

/* samples work structure */
typedef float stream_samples_t[6][256];

/* Everything you wanted to know about band structure */
/*
 * The entire frequency domain is represented by 256 real
 * floating point fourier coefficients. Only the lower 253
 * coefficients are actually utilized however. We use arrays
 * of 256 to be efficient in some cases.
 *
 * The 5 full bandwidth channels (fbw) can have their higher
 * frequencies coupled together. These coupled channels then
 * share their high frequency components.
 *
 * This coupling band is broken up into 18 sub-bands starting
 * at mantissa number 37. Each sub-band is 12 bins wide.
 *
 * There are 50 bit allocation sub-bands which cover the entire
 * frequency range. The sub-bands are of non-uniform width, and
 * approximate a 1/6 octave scale.
 */

/* The following structures are filled in by their corresponding parse_*
 * functions. See http://www.atsc.org/Standards/A52/a_52.pdf for
 * full details on each field. Indented fields are used to denote
 * conditional fields.
 */

typedef struct ac3_state_s {
    int nfchans;	// number of channels, derived from acmod

    /* from syncinfo */
    uint8_t fscod;	// sample rate

    /* from bsi */
    uint8_t acmod;	// coded channels
    float clev;		// centre channel mix level
    float slev;		// surround channels mix level
    uint8_t lfeon;	// coded lfe channel
} ac3_state_t;

typedef struct ac3_ba_s {
    uint16_t fsnroffst;	// fine SNR offset
    uint16_t fgaincod;	// fast gain
    uint16_t deltbae;	// delta bit allocation exists
    int8_t deltba[50];	// per-band delta bit allocation
} ac3_ba_t;

typedef struct audblk_s {
    uint16_t cplinu;		// coupling in use
    uint16_t chincpl[5];	// channel coupled
    uint16_t phsflginu;		// phase flags in use (stereo only)
    uint16_t cplbndstrc[18];	// coupling band structure
    uint16_t cplstrtmant;	// coupling channel start mantissa
    uint16_t cplendmant;	// coupling channel end mantissa
    float cplco[5][18];		// coupling coordinates

    // derived information
    uint16_t cplstrtbnd;	// coupling start band (for bit allocation)
    uint16_t ncplbnd;		// number of coupling bands

    uint16_t rematflg[4];	// stereo rematrixing

    uint16_t endmant[5];	// channel end mantissa

    uint8_t cpl_exp[256];	// decoded coupling channel exponents
    uint8_t fbw_exp[5][256];	// decoded channel exponents
    uint8_t lfe_exp[7];		// decoded lfe channel exponents

    uint16_t sdcycod;		// slow decay
    uint16_t fdcycod;		// fast decay
    uint16_t sgaincod;		// slow gain
    uint16_t dbpbcod;		// dB per bit - encodes the dbknee value
    uint16_t floorcod;		// masking floor

    uint16_t csnroffst;		// coarse SNR offset
    ac3_ba_t cplba;		// coupling bit allocation parameters
    ac3_ba_t ba[5];		// channel bit allocation parameters
    ac3_ba_t lfeba;		// lfe bit allocation parameters

    uint16_t cplfleak;		// coupling fast leak init
    uint16_t cplsleak;		// coupling slow leak init

    // derived bit allocation information
    int8_t fbw_bap[5][256];
    int8_t cpl_bap[256];
    int8_t lfe_bap[7];
} audblk_t;
