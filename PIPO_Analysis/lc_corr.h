//This header is used for computing Linear Apporximation's Correlation.
//When need to compute differential probabilty, use ddt_prob.h

#ifndef _LC_CORR_H_
#define _LC_CORR_H_

#include <math.h>

//0<= 2^{ABS_CORR_t} <=1 always i.e, always negative
typedef double ABS_CORR_t;
typedef int	   SIGN_CORR_t;
#define NEGA -1
#define ZERO 0
#define POSI 1

/*
*Warning Point : In general, checking the eaulity of two double data fails only by using ==.
*				However, the difference of two double data must be bigger than or equal to -log2{(NM)/(M)}
*				where M is the maximum probability NM is the next-maximum probability exept for ONE probability.
*				With 8-bit S-box, the difference must be bigger than -log2{(252)/(254)} \approx 0.0114047632.
*				With the error magin, 0.00001 is enough
*/
#define CORR_PRECISION_THRESHOLD ((ABS_CORR_t)0.00001)

typedef struct corr
{
	SIGN_CORR_t sign;
	ABS_CORR_t magnitude;
}CORR_t;


static inline void INIT_CORR(CORR_t * inout)
{
	inout->sign = (SIGN_CORR_t)ZERO;
	inout->magnitude = (ABS_CORR_t)1;
}

static CORR_t ZERO_CORR = { ZERO,  (ABS_CORR_t)1 };
static CORR_t ONE_CORR = { POSI,  (ABS_CORR_t)0 };

#define IS_ZERO_CORR(c) (((c).sign==ZERO)? 1 : 0)


//multiplication of correlation
static inline void MUL_CORR(CORR_t * out, CORR_t x, CORR_t y)
{
	INIT_CORR(out);
	if ((x.sign == ZERO) || (y.sign == ZERO))
	{
		out->sign = ZERO;
		out->magnitude = (ABS_CORR_t)1;
	}
	else
	{
		out->sign = x.sign * y.sign;
		out->magnitude = x.magnitude + y.magnitude;
	}
}


//comparison of correlation
//|c1| == |c2| :0, |c1| > |c2| : 1, |c1| < |c2| : -1
#define EQUAL 0
#define LEFT 1
#define RIGHT -1
static inline int COMP_ABS_CORR(CORR_t x, CORR_t y)
{
	if ((x.sign == ZERO) || (y.sign == ZERO))
	{
		if ((x.sign == ZERO) && (y.sign == ZERO))
		{
			return EQUAL;
		}
		else if (x.sign == ZERO)
		{
			return RIGHT;
		}
		else
		{
			return LEFT;
		}
	}
	else //non-zero magnitude
	{
		if (x.magnitude == y.magnitude)
		{
			return EQUAL;
		}
		else if (fabs(x.magnitude - y.magnitude) < CORR_PRECISION_THRESHOLD)
		{
			return EQUAL;
		}
		else if (x.magnitude > y.magnitude)
		{
			return LEFT;
		}
		else
		{
			return RIGHT;
		}
	}
}


//LC
typedef struct
{
	SUB_STATE_t sub_i;	//1Round Substitution Input
	SUB_STATE_t sub_o;	//1Round Substitution Output(ie, Diffusion Input)
	DIF_STATE_t dif_o;	//1Round Diffusion Output
	CORR_t c;					//1Round Correlation
} LC_1ROUND_CHAR_t;

typedef struct
{
	SUB_WRD_t i;	//S-Box Iuput
	SUB_WRD_t o;	//S-Box Ouput
	CORR_t c;			//Correlation
} LC_I_O_CORR_t;

typedef struct
{
	SUB_WRD_t o;	//S-Box Ouput
	CORR_t c;			//Correlation
} LC_O_CORR_t;

typedef struct
{
	SUB_WRD_t i;	//S-Box Iuput
	CORR_t c;			//Correlation
} LC_I_CORR_t;

#endif