//This header is used for computing Differential Probability.
//When need to compute correlation, use lat_cor.h

#ifndef _DC_PROB_H_
#define _DC_PROB_H_


#include "pipo_spec.h"
#include <math.h>

//0<= 2^{PROB_t} <=1 always i.e, always negative
typedef double PROB_t;
//ZERO_PROB means 2^{-infinte} i.e, "0"
#define ZERO_PROB (PROB_t)1 
#define ONE_PROB (PROB_t)0


/*
*Warning Point : In general, checking the eaulity of two double data fails only by using ==.
*				However, the difference of two double data must be bigger than or equal to -log2{(NM)/(M)}
*				where M is the maximum probability NM is the next-maximum probability exept for ONE probability.
*				With 8-bit S-box, the difference must be bigger than -log2{(252)/(254)} \approx 0.0114047632.
*				With the error magin, 0.00001 is enough
*/
#define PROB_PRECISION_THRESHOLD ((PROB_t)0.00001)


//multiplication of probability
static inline void MUL_PROB(PROB_t * out, PROB_t in1, PROB_t in2)
{
	if ((in1 == ZERO_PROB) || (in2 == ZERO_PROB))
	{
		*out = ZERO_PROB;
	}
	else
	{
		*out = in1 + in2;
	}
}

//comparison of probability
//p1 == p2 :0, p1 > p2 : 1, p1 < p2 : -1
#define EQUAL 0
#define LEFT 1
#define RIGHT -1
//multiplication of probability
static inline int COMP_PROB(PROB_t x, PROB_t y)
{
	if (x == y)
	{
		return EQUAL;
	}
	else if (x == ZERO_PROB)
	{
		return RIGHT;
	}
	else if (y == ZERO_PROB)
	{
		return LEFT;
	}
	else if (fabs(x - y) < PROB_PRECISION_THRESHOLD)
	{
		return EQUAL;
	}
	else if (x > y)
	{
		return LEFT;
	}
	else
	{
		return RIGHT;
	}
}


typedef struct
{
	SUB_STATE_t sub_i;	//1Round Substitution Input
	SUB_STATE_t sub_o;	//1Round Substitution Output(ie, Diffusion Input)
	DIF_STATE_t dif_o;	//1Round Diffusion Output
	PROB_t				   p;	//1Round Prob
} DC_1ROUND_CHAR_t;

typedef struct
{
	SUB_WRD_t i;	//S-Box Iuput
	SUB_WRD_t o;	//S-Box Ouput
	PROB_t p;			//Probability
} DP_I_O_PROB_t;

typedef struct
{
	SUB_WRD_t o;	//S-Box Ouput
	PROB_t p;			//Probability
} DP_O_PROB_t;

typedef struct
{
	SUB_WRD_t i;	//S-Box Iuput
	PROB_t p;			//Probability
} DP_I_PROB_t;

#endif
