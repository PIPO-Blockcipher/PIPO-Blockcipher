#ifndef _PIPO64_h_
#define _PIPO64_h_
#include <stdint.h>

extern int PIPO64_OFFSET[8];

//Data Types
#define PIPO64_NUM_SBOX_IN_A_STATE	8
#define PIPO64_SBOX_BIT_SIZE		8
#define PIPO64_SBOX_MASK			0xff
#define PIPO64_SBOX_CARDINALITY		256
#define PIPO64_NUM_ROUND			13
#define TRUE	1
#define FALSE	0


typedef	uint8_t				PIPO64_1_WRD_t;  //8bits
typedef uint16_t			PIPO64_2_WRD_t;	//16bits
typedef	uint32_t			PIPO64_4_WRD_t;  //32bits
typedef	uint64_t			PIPO64_8_WRD_t;  //64bits

typedef PIPO64_1_WRD_t		SUB_WRD_t;
typedef SUB_WRD_t			SUB_STATE_t[PIPO64_NUM_SBOX_IN_A_STATE];
typedef int					SUB_CNT_t;
typedef PIPO64_1_WRD_t		DIF_WRD_t;
typedef DIF_WRD_t			DIF_STATE_t[PIPO64_NUM_SBOX_IN_A_STATE];
typedef int					DIF_CNT_t;
typedef int					FLAG_t;

static SUB_WRD_t PIPO64_Sbox[PIPO64_SBOX_CARDINALITY] =
{
	0x5E, 0xF9, 0xFC, 0x00, 0x3F, 0x85, 0xBA, 0x5B, 0x18, 0x37, 0xB2, 0xC6, 0x71, 0xC3, 0x74, 0x9D,
	0xA7, 0x94, 0x0D, 0xE1, 0xCA, 0x68, 0x53, 0x2E, 0x49, 0x62, 0xEB, 0x97, 0xA4, 0x0E, 0x2D, 0xD0,
	0x16, 0x25, 0xAC, 0x48, 0x63, 0xD1, 0xEA, 0x8F, 0xF7, 0x40, 0x45, 0xB1, 0x9E, 0x34, 0x1B, 0xF2,
	0xB9, 0x86, 0x03, 0x7F, 0xD8, 0x7A, 0xDD, 0x3C, 0xE0, 0xCB, 0x52, 0x26, 0x15, 0xAF, 0x8C, 0x69,
	0xC2, 0x75, 0x70, 0x1C, 0x33, 0x99, 0xB6, 0xC7, 0x04, 0x3B, 0xBE, 0x5A, 0xFD, 0x5F, 0xF8, 0x81,
	0x93, 0xA0, 0x29, 0x4D, 0x66, 0xD4, 0xEF, 0x0A, 0xE5, 0xCE, 0x57, 0xA3, 0x90, 0x2A, 0x09, 0x6C,
	0x22, 0x11, 0x88, 0xE4, 0xCF, 0x6D, 0x56, 0xAB, 0x7B, 0xDC, 0xD9, 0xBD, 0x82, 0x38, 0x07, 0x7E,
	0xB5, 0x9A, 0x1F, 0xF3, 0x44, 0xF6, 0x41, 0x30, 0x4C, 0x67, 0xEE, 0x12, 0x21, 0x8B, 0xA8, 0xD5,
	0x55, 0x6E, 0xE7, 0x0B, 0x28, 0x92, 0xA1, 0xCC, 0x2B, 0x08, 0x91, 0xED, 0xD6, 0x64, 0x4F, 0xA2,
	0xBC, 0x83, 0x06, 0xFA, 0x5D, 0xFF, 0x58, 0x39, 0x72, 0xC5, 0xC0, 0xB4, 0x9B, 0x31, 0x1E, 0x77,
	0x01, 0x3E, 0xBB, 0xDF, 0x78, 0xDA, 0x7D, 0x84, 0x50, 0x6B, 0xE2, 0x8E, 0xAD, 0x17, 0x24, 0xC9,
	0xAE, 0x8D, 0x14, 0xE8, 0xD3, 0x61, 0x4A, 0x27, 0x47, 0xF0, 0xF5, 0x19, 0x36, 0x9C, 0xB3, 0x42,
	0x1D, 0x32, 0xB7, 0x43, 0xF4, 0x46, 0xF1, 0x98, 0xEC, 0xD7, 0x4E, 0xAA, 0x89, 0x23, 0x10, 0x65,
	0x8A, 0xA9, 0x20, 0x54, 0x6F, 0xCD, 0xE6, 0x13, 0xDB, 0x7C, 0x79, 0x05, 0x3A, 0x80, 0xBF, 0xDE,
	0xE9, 0xD2, 0x4B, 0x2F, 0x0C, 0xA6, 0x95, 0x60, 0x0F, 0x2C, 0xA5, 0x51, 0x6A, 0xC8, 0xE3, 0x96,
	0xB0, 0x9F, 0x1A, 0x76, 0xC1, 0x73, 0xC4, 0x35, 0xFE, 0x59, 0x5C, 0xB8, 0x87, 0x3D, 0x02, 0xFB,
};

#define  ROL8( _8bit_ , i) ((( _8bit_) << ((i)%8)) | (( _8bit_) >> ( 8 - ((i)% 8))))
#define  ROR8( _8bit_ , i) ((( _8bit_) >> ((i)%8)) | (( _8bit_) << ( 8 - ((i)% 8))))

//R For Differential Trail searching
static inline void PIPO64_R(DIF_STATE_t out, DIF_STATE_t in)
{
	uint8_t X[8];
	//
	//L_Shift   7          6          5          4          3          2          1          0 
	//        in[0]  ||  in[1]  ||  in[2]  ||  in[3]  ||  in[4]  ||  in[5]  ||  in[6]  ||  in[7]
	//X[0] =    0          0          0          0          0          0          0          0      LSB     
	//X[1] =    1          1          1          1          1          1          1          1     
	//X[.] =   ...        ...        ...        ...        ...        ...        ...        ...    
	//X[7] =    7          7          7          7          7          7          7          7      MSB
	int i, j;
	for (i = 0; i <= 7; i++)
	{
		X[i] = 0;
		for (j = 0; j <= 7; j++)
		{
			X[i] |= ((((uint8_t)in[j] >> (i)) & 0x1) << (7 - j));
		}
	}

	X[0] = ROL8(X[0], PIPO64_OFFSET[0]);
	X[1] = ROL8(X[1], PIPO64_OFFSET[1]);
	X[2] = ROL8(X[2], PIPO64_OFFSET[2]);
	X[3] = ROL8(X[3], PIPO64_OFFSET[3]);
	X[4] = ROL8(X[4], PIPO64_OFFSET[4]);
	X[5] = ROL8(X[5], PIPO64_OFFSET[5]);
	X[6] = ROL8(X[6], PIPO64_OFFSET[6]);
	X[7] = ROL8(X[7], PIPO64_OFFSET[7]);

	//L_Shift   7          6          5          4          3          2          1          0 
	//        in[0]  ||  in[1]  ||  in[2]  ||  in[3]  ||  in[4]  ||  in[5]  ||  in[6]  ||  in[7]
	//X[0] =    0          0          0          0          0          0          0          0      LSB     
	//X[1] =    1          1          1          1          1          1          1          1     
	//X[.] =   ...        ...        ...        ...        ...        ...        ...        ...    
	//X[7] =    7          7          7          7          7          7          7          7      MSB


	for (i = 0; i <= 7; i++)
	{
		out[i] = 0;
		for (j = 0; j <= 7; j++)
		{
			out[i] |= ((((uint8_t)X[j] >> (7 - i)) & 0x1) << (j));
		}
	}

}

//Inverse R For Differential Trail searching
static inline void PIPO64_Inv_R(DIF_STATE_t out, DIF_STATE_t in)
{
	uint8_t X[8];
	//
	//L_Shift   7          6          5          4          3          2          1          0 
	//        in[0]  ||  in[1]  ||  in[2]  ||  in[3]  ||  in[4]  ||  in[5]  ||  in[6]  ||  in[7]
	//X[0] =    0          0          0          0          0          0          0          0      LSB     
	//X[1] =    1          1          1          1          1          1          1          1     
	//X[.] =   ...        ...        ...        ...        ...        ...        ...        ...    
	//X[7] =    7          7          7          7          7          7          7          7      MSB
	int i, j;
	for (i = 0; i <= 7; i++)
	{
		X[i] = 0;
		for (j = 0; j <= 7; j++)
		{
			X[i] |= (((uint8_t)(in[j] >> (i)) & 0x1) << (7 - j));
		}
	}


	X[0] = ROR8(X[0], PIPO64_OFFSET[0]);
	X[1] = ROR8(X[1], PIPO64_OFFSET[1]);
	X[2] = ROR8(X[2], PIPO64_OFFSET[2]);
	X[3] = ROR8(X[3], PIPO64_OFFSET[3]);
	X[4] = ROR8(X[4], PIPO64_OFFSET[4]);
	X[5] = ROR8(X[5], PIPO64_OFFSET[5]);
	X[6] = ROR8(X[6], PIPO64_OFFSET[6]);
	X[7] = ROR8(X[7], PIPO64_OFFSET[7]);


	//L_Shift   7          6          5          4          3          2          1          0 
	//        in[0]  ||  in[1]  ||  in[2]  ||  in[3]  ||  in[4]  ||  in[5]  ||  in[6]  ||  in[7]
	//X[0] =    0          0          0          0          0          0          0          0      LSB     
	//X[1] =    1          1          1          1          1          1          1          1     
	//X[.] =   ...        ...        ...        ...        ...        ...        ...        ...    
	//X[7] =    7          7          7          7          7          7          7          7      MSB


	for (i = 0; i <= 7; i++)
	{
		out[i] = 0;
		for (j = 0; j <= 7; j++)
		{
			out[i] |= ((((uint8_t)X[j] >> (7 - i)) & 0x1) << (j));
		}
	}
}
///////////////////////////////////


typedef uint32_t			TRUNC_STATE_t;
#define SIZE_OF_1R_TRUNC	35

//All trails of PIPO64 have their rotationally equivalent trails
static TRUNC_STATE_t _1ROUND_ACTIVE_TRUNCATIONS[SIZE_OF_1R_TRUNC] =
{
	//1-active
	0x01, // 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, (eg) are rotaionlly equivalent
	//2-active
	0x09, 0x03, 0x05, 0x11,
	//3-active
	0x43, 0x23, 0x25, 0x07, 0x0B, 0x13, 0x15,
	//4-active
	0x53, 0x2B, 0x27, 0x47, 0x0F, 0x4B, 0x33, 0x55, 0x17, 0x1B,
	//5-active
	0x5B, 0x67, 0x37, 0x4F, 0x57, 0x2F, 0x1F,
	//6-active
	0x6F, 0x77, 0x5F, 0x3F,
	//7-active
	0x7F,
	//8-active
	0xFF
};

#endif