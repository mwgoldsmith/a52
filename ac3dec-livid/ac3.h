/* 
 *    ac3.h
 *
 *	Aaron Holtzman - May 1999
 *
 */

typedef unsigned long  uint_32;
typedef unsigned short uint_16;
typedef unsigned char  uint_8;

/* The following structures are filled in by their corresponding fill_*
 * functions. See http://www.atsc.org/Standards/A52/a_52.pdf for
 * full details on each field. Indented fields are used to denote
 * conditional fields.
 */

typedef struct syncinfo_s
{
	/* Sync word == 0x0B77 */
	/* uint_16   syncword; */
	/* crc for the first 5/8 of the sync block */
	/* uint_16   crc1; */
	/* Stream Sampling Rate (kHz) 0 = 48, 1 = 44.1, 2 = 32, 3 = reserved */
	uint_32		fscod;	
	/* Frame size code */
	uint_32		frmsizecod;
} syncinfo_t;

typedef struct bsi_s
{
	/* Bit stream identification == 0x8 */
	uint_16 bsid;	
	/* Bit stream mode */
	uint_16 bsmod;
	/* Audio coding mode */
	uint_16 acmod;
	/* Number of channels (excluding LFE) */
	/* This data isn't actually in the AC3 stream, but derived from
	 * acmod */
	uint_16 nfchans;
	/* If we're using the centre channel then */
		/* centre mix level */
		uint_16 cmixlev;
	/* If we're using the surround channel then */
		/* surround mix level */
		uint_16 surmixlev;
	/* If we're in 2/0 mode then */
		/* Dolby surround mix level - NOT USED - */
		uint_16 dsurmod;
	/* Low frequency effects on */
	uint_16 lfeon;
	/* Dialogue Normalization level */
	uint_16 dialnorm;
	/* Compression exists */
	uint_16 compre;
		/* Compression level */
		uint_16 compr;
	/* Language code exists */
	uint_16 langcode;
		/* Language code */
		uint_16 langcod;
	/* Audio production info exists*/
	uint_16 audprodie;
		uint_16 mixlevel;
		uint_16 roomtyp;
	/* If we're in dual mono mode (acmod == 0) then extra stuff */
		uint_16 dialnorm2;
		uint_16 compr2e;
			uint_16 compr2;
		uint_16 langcod2e;
			uint_16 langcod2;
		uint_16 audprodi2e;
			uint_16 mixlevel2;
			uint_16 roomtyp2;
	/* Copyright bit */
	uint_16 copyrightb;
	/* Original bit */
	uint_16 origbs;
	/* Timecode 1 exists */
	uint_16 timecod1e;
		/* Timecode 1 */
		uint_16 timecod1;
	/* Timecode 2 exists */
	uint_16 timecod2e;
		/* Timecode 2 */
		uint_16 timecod2;
	/* Additional bit stream info exists */
	uint_16 addbsie;
		/* Additional bit stream length - 1 (in bytes) */
		uint_16 addbsil;
		/* Additional bit stream information (max 64 bytes) */
		uint_8	addbsi[64];
} bsi_t;


/* more pain */
typedef struct audblk_s
{
	/* block switch bit indexed by channel num */
	uint_16 blksw[5];
	/* dither enable bit indexed by channel num */
	uint_16 dithflag[5];
	/* dynamic range gain exists */
	uint_16 dynrnge;
		/* dynamic range gain */
		uint_16 dynrng;
	/* if acmod==0 then */
	/* dynamic range 2 gain exists */
	uint_16 dynrng2e;
		/* dynamic range 2 gain */
		uint_16 dynrng2;
	/* coupling strategy exists */
	uint_16 cpistre;
		/* coupling in use */
		uint_16 cplinu;
			/* channel coupled */
			uint_16 chincpl[5];
			/* if acmod==2 then */
				/* Phase flags in use */
				uint_16 phsflginu;
			/* coupling begin frequency code */
			uint_16 cplbegf;
			/* coupling end frequency code */
			uint_16 cplendf;
			



} audblk_t;


