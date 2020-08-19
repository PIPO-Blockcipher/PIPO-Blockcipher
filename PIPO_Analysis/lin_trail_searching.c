#include "pipo_spec.h"
#include "lc_corr.h"
#include "searching.h"
#include <stdio.h>
#include <memory.h>
#include <stdlib.h>

extern CORR_t MAX_CORR; //non-one abs correlation
extern CORR_t MIN_CORR; //non-zero correlation
extern LC_I_O_CORR_t	LC_IOC[PIPO64_SBOX_CARDINALITY * PIPO64_SBOX_CARDINALITY];
extern LC_I_CORR_t		LC_IC_FO[PIPO64_SBOX_CARDINALITY][PIPO64_SBOX_CARDINALITY];
extern LC_O_CORR_t		LC_OC_FI[PIPO64_SBOX_CARDINALITY][PIPO64_SBOX_CARDINALITY];
extern SUB_CNT_t LC_NUM_IOC_NONZERO;
extern SUB_CNT_t LC_NUM_IOC_MAX;
extern SUB_CNT_t LC_NUM_IC_FO_NONZERO[PIPO64_SBOX_CARDINALITY];
extern SUB_CNT_t LC_NUM_IC_FO_MAX[PIPO64_SBOX_CARDINALITY];
extern SUB_CNT_t LC_NUM_OC_FI_NONZERO[PIPO64_SBOX_CARDINALITY];
extern SUB_CNT_t LC_NUM_OC_FI_MAX[PIPO64_SBOX_CARDINALITY];


//here
CORR_t	LC_BOUNDS[PIPO64_NUM_ROUND + 1];
LC_1ROUND_CHAR_t LIN_TRAIL_IN_PROG[PIPO64_NUM_ROUND + 1];
LC_1ROUND_CHAR_t LIN_TRAIL_FOR_OUT[PIPO64_NUM_ROUND + 1];
CORR_t	LC_BOUND_IN_PROG;
FLAG_t	TOUCH_THE_LEAF;
char	OUT_FILE_NAME[512];
FILE  * ofp;


void SPN_Round1_Corr_Only(int target_round);
void SPN_Round1_j_Corr_Only(
	SUB_STATE_t X1,
	SUB_STATE_t Y1,
	CORR_t C1[PIPO64_SBOX_CARDINALITY],
	CORR_t BEF_DET_CORR,
	int target_round,
	TRUNC_STATE_t AT1,
	int j);
void SPN_Roundi_Corr_Only(CORR_t BEF_DET_CORR, int target_round, int i);
void SPN_Roundi_j_Corr_Only(
	SUB_STATE_t Xi,
	SUB_STATE_t Yi,
	CORR_t Ci[PIPO64_NUM_SBOX_IN_A_STATE],
	CORR_t BEF_DET_CORR,
	int target_round,
	int i,
	int j);
void SPN_Roundn_Corr_Only(CORR_t BEF_DET_CORR, int n);
void SPN_Roundn_j_Corr_Only(
	SUB_STATE_t Xn,
	SUB_STATE_t Yn,
	CORR_t Cn[PIPO64_NUM_SBOX_IN_A_STATE],
	CORR_t BEF_DET_CORR,
	int n,
	int j);


static inline CORR_t put_corrs(CORR_t c[PIPO64_NUM_SBOX_IN_A_STATE])
{
	int i;
	CORR_t C;
	C = c[0];
	for (i = 1; i < PIPO64_NUM_SBOX_IN_A_STATE; i++)
	{
		MUL_CORR(&C, C, c[i]);
	}
	return C;
}

static inline void init_corrs(CORR_t c[PIPO64_NUM_SBOX_IN_A_STATE])
{
	int i;
	for (i = 0; i < PIPO64_NUM_SBOX_IN_A_STATE; i++)
	{
		c[i] = ONE_CORR;
	}
}


static inline int bound_checker_corr(CORR_t check)
{
	if (TOUCH_THE_LEAF == TRUE)
	{
		if (COMP_ABS_CORR(check, LC_BOUND_IN_PROG) != LEFT)
			return EXCEED_BOUND;
		else
			return UNDER_BOUND;
	}
	else
	{
		if (COMP_ABS_CORR(check, LC_BOUND_IN_PROG) == RIGHT)
			return EXCEED_BOUND;
		else
			return UNDER_BOUND;
	}
}

static inline int expect_state_corr_at_1_j(CORR_t * expected_corr, TRUNC_STATE_t AT, int j)
{
	int first_active = THE_LAST_WORD;
	CORR_t tmp_cum_corr = ONE_CORR;
	int wrd_idx;
	if (j == ROUND_START)
	{
		for (wrd_idx = 0; wrd_idx < PIPO64_NUM_SBOX_IN_A_STATE; wrd_idx++)
		{
			if ((AT & (0x1 << wrd_idx)) != 0)
			{
				MUL_CORR(&tmp_cum_corr, tmp_cum_corr, MAX_CORR);
				if (first_active == THE_LAST_WORD)
				{
					first_active = wrd_idx;
				}
			}
		}
	}
	else if (j < (PIPO64_NUM_SBOX_IN_A_STATE - 1))
	{
		for (wrd_idx = (j + 1); wrd_idx < PIPO64_NUM_SBOX_IN_A_STATE; wrd_idx++)
		{
			if ((AT & (0x1 << wrd_idx)) != 0)
			{
				MUL_CORR(&tmp_cum_corr, tmp_cum_corr, MAX_CORR);
				if (first_active == THE_LAST_WORD)
				{
					first_active = wrd_idx;
				}
			}
		}
	}
	else // if (j == (PIPO64_NUM_SBOX_IN_A_STATE - 1))
	{
		//return initial out
	}
	*expected_corr = tmp_cum_corr;
	return first_active;
}



static inline int expect_state_corr_at_j(CORR_t * expected_corr, SUB_STATE_t X, int j)
{
	int first_active = THE_LAST_WORD;
	CORR_t tmp_cum_corr = ONE_CORR;
	int wrd_idx;
	if (j == ROUND_START)
	{
		for (wrd_idx = 0; wrd_idx < PIPO64_NUM_SBOX_IN_A_STATE; wrd_idx++)
		{
			if (X[wrd_idx] != 0)
			{
				MUL_CORR(&tmp_cum_corr, tmp_cum_corr, MAX_CORR);
				if (first_active == THE_LAST_WORD)
				{
					first_active = wrd_idx;
				}
			}
		}
	}
	else if (j < (PIPO64_NUM_SBOX_IN_A_STATE - 1))
	{
		for (wrd_idx = (j + 1); wrd_idx < PIPO64_NUM_SBOX_IN_A_STATE; wrd_idx++)
		{
			if (X[wrd_idx] != 0)
			{
				MUL_CORR(&tmp_cum_corr, tmp_cum_corr, MAX_CORR);
				if (first_active == THE_LAST_WORD)
				{
					first_active = wrd_idx;
				}
			}
		}
	}
	else // if (j == (PIPO64_NUM_SBOX_IN_A_STATE - 1))
	{
		//return initial out
	}
	*expected_corr = tmp_cum_corr;
	return first_active;
}

static inline void expect_next_state_corr_at_j(CORR_t * expected_next_corr, SUB_STATE_t Y, int j)
{
	SUB_STATE_t copied = { 0, };
	DIF_STATE_t dif_part;
	CORR_t tmp_cum_corr = ONE_CORR;
	int wrd_idx;
	memcpy(copied, Y, sizeof(SUB_WRD_t) * (j + 1));

	PIPO64_R(dif_part, copied);

	for (wrd_idx = 0; wrd_idx < PIPO64_NUM_SBOX_IN_A_STATE; wrd_idx++)
	{
		if (dif_part[wrd_idx] != 0)
		{
			MUL_CORR(&tmp_cum_corr, tmp_cum_corr, MAX_CORR);
		}
	}

	*expected_next_corr = tmp_cum_corr;
}

void SPN_Roundn_j_Corr_Only(SUB_STATE_t Xn, SUB_STATE_t Yn, CORR_t Cn[PIPO64_NUM_SBOX_IN_A_STATE], CORR_t BEF_DET_CORR, int n, int j)
{
	SUB_WRD_t	Xn_j = Xn[j]; //It is fixed from before
	SUB_WRD_t	Yn_j = LC_OC_FI[Xn_j][0].o;
	CORR_t		Cn_j = LC_OC_FI[Xn_j][0].c; //max corr with fixed in
	int			next_word_idx;
	CORR_t		_n_round_expected;
	CORR_t		_n_bound;
	CORR_t		DET_CORR;

	MUL_CORR(&DET_CORR, BEF_DET_CORR, Cn_j);
	next_word_idx = expect_state_corr_at_j(&_n_round_expected, Xn, j);
	MUL_CORR(&_n_bound, _n_round_expected, DET_CORR);						//[1~n]R

	//pruning
	if (bound_checker_corr(_n_bound) == EXCEED_BOUND)
		return;

	Yn[j] = Yn_j;
	Cn[j] = Cn_j;


	if (next_word_idx == THE_LAST_WORD)  //when this word is the last word
	{
		int L;

		TOUCH_THE_LEAF = TRUE;
		printf("--New Bound!! 2^{%0.4lf}\n", DET_CORR.magnitude);
		//update Progress bound
		LC_BOUND_IN_PROG = DET_CORR;

		//Copy 1~n-1-Round states to Out trail 
		for (L = 1; L <= n - 1; L++)
		{
			memcpy(LIN_TRAIL_FOR_OUT[L].sub_i, LIN_TRAIL_IN_PROG[L].sub_i, sizeof(SUB_STATE_t));
			memcpy(LIN_TRAIL_FOR_OUT[L].sub_o, LIN_TRAIL_IN_PROG[L].sub_o, sizeof(SUB_STATE_t));
			memcpy(LIN_TRAIL_FOR_OUT[L].dif_o, LIN_TRAIL_IN_PROG[L].dif_o, sizeof(DIF_STATE_t));
			LIN_TRAIL_FOR_OUT[L].c = LIN_TRAIL_IN_PROG[L].c;
		}
		//Copy n-Round state to Out trail
		memcpy(LIN_TRAIL_FOR_OUT[n].sub_i, Xn, sizeof(SUB_STATE_t));
		memcpy(LIN_TRAIL_FOR_OUT[n].sub_o, Yn, sizeof(SUB_STATE_t));
		PIPO64_R(LIN_TRAIL_FOR_OUT[n].dif_o, Yn);
		LIN_TRAIL_FOR_OUT[n].c = put_corrs(Cn);
		LC_BOUNDS[n] = DET_CORR;
	}
	else if (next_word_idx < PIPO64_NUM_SBOX_IN_A_STATE) //when this word is not the last word
	{
		//Move to next word
		SPN_Roundn_j_Corr_Only(Xn, Yn, Cn, DET_CORR, n, next_word_idx);
	}
}



void SPN_Roundn_Corr_Only(CORR_t BEF_DET_CORR, int n)
{
	SUB_STATE_t Xn;
	SUB_STATE_t Yn;
	CORR_t		Cn[PIPO64_NUM_SBOX_IN_A_STATE];
	CORR_t		_n_round_expected;
	CORR_t		_n_bound;
	int			next_word_idx;

	memcpy(Xn, LIN_TRAIL_IN_PROG[n - 1].dif_o, sizeof(SUB_STATE_t));

	_n_bound = BEF_DET_CORR;														//[1~(n-1)]R
	next_word_idx = expect_state_corr_at_j(&_n_round_expected, Xn, ROUND_START);	//nR
	MUL_CORR(&_n_bound, _n_bound, _n_round_expected);

	if (bound_checker_corr(_n_bound) != EXCEED_BOUND)
	{
		//init the sub out state and correlations
		memset(Yn, 0, sizeof(SUB_STATE_t));
		init_corrs(Cn);

		SPN_Roundn_j_Corr_Only(Xn, Yn, Cn, BEF_DET_CORR, n, next_word_idx);
	}
}

void SPN_Roundi_j_Corr_Only(SUB_STATE_t Xi, SUB_STATE_t Yi, CORR_t Ci[PIPO64_NUM_SBOX_IN_A_STATE], CORR_t BEF_DET_CORR, int target_round, int i, int j)
{
	SUB_CNT_t	_i_j;
	SUB_WRD_t	Xi_j = Xi[j]; //It is fixed from before
	SUB_WRD_t	Yi_j;
	CORR_t		Ci_j;
	int			next_word_idx;
	CORR_t		_i_round_expected;
	CORR_t		_i1_round_expected;
	CORR_t		_i_bound;
	CORR_t		_i1_bound;
	CORR_t		DET_CORR;

	for (_i_j = 0; _i_j < LC_NUM_OC_FI_NONZERO[Xi_j]; _i_j++)
	{
		Yi_j = LC_OC_FI[Xi_j][_i_j].o;
		Ci_j = LC_OC_FI[Xi_j][_i_j].c;

		MUL_CORR(&DET_CORR, BEF_DET_CORR, Ci_j);


		next_word_idx = expect_state_corr_at_j(&_i_round_expected, Xi, j);
		MUL_CORR(&_i_bound, _i_round_expected, DET_CORR);						//[1~i]R
		MUL_CORR(&_i_bound, _i_bound, LC_BOUNDS[target_round - i]);				//[i+1~n]R

		//pruning by using descending order
		if (bound_checker_corr(_i_bound) == EXCEED_BOUND)
			break;

		//store current word
		Yi[j] = Yi_j;
		Ci[j] = Ci_j;


		MUL_CORR(&_i1_bound, _i_round_expected, DET_CORR);							//[1~i]R
		expect_next_state_corr_at_j(&_i1_round_expected, Yi, j);
		MUL_CORR(&_i1_bound, _i1_bound, _i1_round_expected);						//i+1R

		if (i < (target_round - 1))
		{
			MUL_CORR(&_i1_bound, _i1_bound, LC_BOUNDS[target_round - 1 - i]);		//[i+2~n]R
		}


		//pruning
		if (bound_checker_corr(_i1_bound) == EXCEED_BOUND)
			continue; //next value

		if (next_word_idx == THE_LAST_WORD)  //when this word is the last word
		{
			memcpy(LIN_TRAIL_IN_PROG[i].sub_i, Xi, sizeof(SUB_STATE_t));
			memcpy(LIN_TRAIL_IN_PROG[i].sub_o, Yi, sizeof(SUB_STATE_t));
			PIPO64_R(LIN_TRAIL_IN_PROG[i].dif_o, LIN_TRAIL_IN_PROG[i].sub_o);
			LIN_TRAIL_IN_PROG[i].c = put_corrs(Ci);
			if (i == (target_round - 1))
			{
				SPN_Roundn_Corr_Only(DET_CORR, target_round);
			}
			else if (i < (target_round - 1))
			{
				SPN_Roundi_Corr_Only(DET_CORR, target_round, i + 1);
			}
		}
		else if (next_word_idx < PIPO64_NUM_SBOX_IN_A_STATE) //when this word is not the last word
		{
			//Move to next word
			SPN_Roundi_j_Corr_Only(Xi, Yi, Ci, DET_CORR, target_round, i, next_word_idx);
		}
	}
}

void SPN_Roundi_Corr_Only(CORR_t BEF_DET_CORR, int target_round, int i)
{
	SUB_STATE_t Xi;
	SUB_STATE_t Yi;
	CORR_t		Ci[PIPO64_NUM_SBOX_IN_A_STATE];
	CORR_t		_i_round_expected;
	CORR_t		_i_bound;
	int			next_word_idx;

	memcpy(Xi, LIN_TRAIL_IN_PROG[i - 1].dif_o, sizeof(SUB_STATE_t));

	_i_bound = BEF_DET_CORR;														//[1~(i-1)]R
	next_word_idx = expect_state_corr_at_j(&_i_round_expected, Xi, ROUND_START);	//iR
	MUL_CORR(&_i_bound, _i_bound, _i_round_expected);
	MUL_CORR(&_i_bound, _i_bound, LC_BOUNDS[target_round - i]);						//[(i+1)~n]R

	if (bound_checker_corr(_i_bound) != EXCEED_BOUND)
	{
		//init the sub out state and correlations
		memset(Yi, 0, sizeof(SUB_STATE_t));
		init_corrs(Ci);

		SPN_Roundi_j_Corr_Only(Xi, Yi, Ci, BEF_DET_CORR, target_round, i, next_word_idx);
	}
}

void SPN_Round1_j_Corr_Only(SUB_STATE_t X1, SUB_STATE_t Y1, CORR_t C1[PIPO64_SBOX_CARDINALITY], CORR_t BEF_DET_CORR, int target_round, TRUNC_STATE_t AT1, int j)
{
	SUB_CNT_t	_1_j;
	SUB_WRD_t	X1_j;
	SUB_WRD_t	Y1_j;
	CORR_t		C1_j;
	int			next_word_idx;
	CORR_t		_1_round_expected;
	CORR_t		_2_round_expected;
	CORR_t		_1_bound;
	CORR_t      _2_bound;
	CORR_t		DET_CORR;

	// this word is always active
	for (_1_j = 0x1; _1_j < PIPO64_SBOX_CARDINALITY; _1_j++)
	{
		X1_j = LC_IC_FO[_1_j][0].i;
		Y1_j = (SUB_WRD_t)_1_j;
		C1_j = LC_IC_FO[_1_j][0].c;

		MUL_CORR(&DET_CORR, BEF_DET_CORR, C1_j);

		next_word_idx = expect_state_corr_at_1_j(&_1_round_expected, AT1, j);
		MUL_CORR(&_1_bound, _1_round_expected, DET_CORR);			//1R
		MUL_CORR(&_1_bound, _1_bound, LC_BOUNDS[target_round - 1]); //[2~n]R

		//pruning
		if (bound_checker_corr(_1_bound) == EXCEED_BOUND)
			continue; //need to check next value on j word

		//store current word
		X1[j] = X1_j;
		Y1[j] = Y1_j;
		C1[j] = C1_j;


		MUL_CORR(&_2_bound, _1_round_expected, DET_CORR);				//1R
		expect_next_state_corr_at_j(&_2_round_expected, Y1, j);
		MUL_CORR(&_2_bound, _2_round_expected, _2_bound);				//2R

		if (target_round > 2)
		{
			MUL_CORR(&_2_bound, _2_bound, LC_BOUNDS[target_round - 2]); //[3~n]R
		}

		//pruning
		if (bound_checker_corr(_2_bound) == EXCEED_BOUND)
			continue; //need to check next value on j word

		if (next_word_idx == THE_LAST_WORD) //when this word is the last word
		{
			memcpy(LIN_TRAIL_IN_PROG[1].sub_i, X1, sizeof(SUB_STATE_t));
			memcpy(LIN_TRAIL_IN_PROG[1].sub_o, Y1, sizeof(SUB_STATE_t));
			PIPO64_R(LIN_TRAIL_IN_PROG[1].dif_o, LIN_TRAIL_IN_PROG[1].sub_o);
			LIN_TRAIL_IN_PROG[1].c = put_corrs(C1);

			if (target_round == 2)
			{
				//Move to the last round
				SPN_Roundn_Corr_Only(DET_CORR, target_round);
			}
			else
			{
				//Move to next(2) round
				SPN_Roundi_Corr_Only(DET_CORR, target_round, 2);
			}
		}
		else if (next_word_idx < PIPO64_NUM_SBOX_IN_A_STATE) //when this word is not the last word
		{
			//Move to next word
			SPN_Round1_j_Corr_Only(X1, Y1, C1, DET_CORR, target_round, AT1, next_word_idx);
		}
	}
}

void SPN_Round1_Corr_Only(int target_round)
{
	SUB_STATE_t		X1;
	SUB_STATE_t		Y1;
	CORR_t			C1[PIPO64_NUM_SBOX_IN_A_STATE];
	TRUNC_STATE_t	AT1;
	int				idx_1round_trunc;
	CORR_t			_1_round_expected;
	CORR_t			_1_bound;
	int				next_word_idx;
	CORR_t			DET_CORR = ONE_CORR;

	for (idx_1round_trunc = 0; idx_1round_trunc < SIZE_OF_1R_TRUNC; idx_1round_trunc++)
	{
		AT1 = _1ROUND_ACTIVE_TRUNCATIONS[idx_1round_trunc];

		next_word_idx = expect_state_corr_at_1_j(&_1_round_expected, AT1, ROUND_START); //1R
		MUL_CORR(&_1_bound, _1_round_expected, LC_BOUNDS[target_round - 1]);			//[2~n]R

		//pruning
		if (bound_checker_corr(_1_bound) == EXCEED_BOUND)
			break;

		//init
		memset(X1, 0, sizeof(SUB_STATE_t));
		memset(Y1, 0, sizeof(SUB_STATE_t));
		init_corrs(C1);

		SPN_Round1_j_Corr_Only(X1, Y1, C1, DET_CORR, target_round, AT1, next_word_idx);
	}
}

void LC_fprintf(int round_idx);

int Best_Trail_Corr_Only(int target_round)
{
	int round_idx;
	FLAG_t first_trial_flag;

	LC_BOUNDS[1] = MAX_CORR;
	printf("=================Searching(Correlation Only)...=================\n\n");
	if (LC_BOUNDS[1].sign == NEGA)
	{
		printf("==Set   [ 1]-Round Bound with the Known Correlation -2^{%0.4lf}\n", LC_BOUNDS[1].magnitude);
	}
	else
	{
		printf("==Set   [ 1]-Round Bound with the Known Correlation +2^{%0.4lf}\n", LC_BOUNDS[1].magnitude);
	}


	//Start searching
	for (round_idx = 2; round_idx <= target_round; round_idx++)
	{
		first_trial_flag = TRUE;
		TOUCH_THE_LEAF = FALSE;

		sprintf(OUT_FILE_NAME, "[%d%d%d%d%d%d%d%d]_%dRound_Best_Linear_Trail.txt",
			PIPO64_OFFSET[0], PIPO64_OFFSET[1], PIPO64_OFFSET[2], PIPO64_OFFSET[3],
			PIPO64_OFFSET[4], PIPO64_OFFSET[5], PIPO64_OFFSET[6], PIPO64_OFFSET[7],
			round_idx);

		while (TOUCH_THE_LEAF != TRUE)
		{
			if (first_trial_flag == TRUE)
			{
				first_trial_flag = FALSE;
				MUL_CORR(&LC_BOUND_IN_PROG, LC_BOUNDS[round_idx - 1], MAX_CORR);
				Realtime_Print(START_TIME);
			}
			else
			{
				MUL_CORR(&LC_BOUND_IN_PROG, LC_BOUND_IN_PROG, MIN_CORR);
				Realtime_Print(END_TIME);
			}
			printf("==Start [%2d]-Round With Bound  +-2^{%0.4lf}==\n", round_idx, LC_BOUND_IN_PROG.magnitude);
			/*Finding Trail*/
			SPN_Round1_Corr_Only(round_idx);
			/***************/
		}
		Realtime_Print(END_TIME);
		if (LC_BOUNDS[round_idx].sign == NEGA)
		{
			printf("==End   [%2d]-Round With Bound   -2^{%0.4lf}==\n\n", round_idx, LC_BOUNDS[round_idx].magnitude);
		}
		else
		{
			printf("==End   [%2d]-Round With Bound   +2^{%0.4lf}==\n\n", round_idx, LC_BOUNDS[round_idx].magnitude);
		}
		LC_fprintf(round_idx);
	}
	printf("\n\n Everything's done \n\n");
	printf("===================================================================\n\n");
	for (round_idx = 1; round_idx <= target_round; round_idx++)
	{
		printf("==   [%2d]-Round Best Correlation Potential : 2^{%0.4lf}\n", round_idx, 2 * LC_BOUNDS[round_idx].magnitude);
	}

	return 0;
}



void LC_fprintf(int round_idx)
{
	int __FF__;
	int i;
	ofp = fopen(OUT_FILE_NAME, "w");

	if (LC_BOUND_IN_PROG.sign == NEGA)
	{
		fprintf(ofp, "[%2d] Round Best Correlation : -2^{%0.4lf} \n", round_idx, LC_BOUND_IN_PROG.magnitude);
	}
	else if (LC_BOUND_IN_PROG.sign == POSI)
	{
		fprintf(ofp, "[%2d] Round Best Correlation : +2^{%0.4lf} \n", round_idx, LC_BOUND_IN_PROG.magnitude);
	}
	else if (LC_BOUND_IN_PROG.sign == ZERO)
	{
		fprintf(ofp, "[%2d] Round Best Correlation : ZERO \n", round_idx);
	}

	fprintf(ofp, "========================\n");

	LC_1ROUND_CHAR_t out_LC_perm_wise;

	for (__FF__ = 1; __FF__ <= round_idx; __FF__++)
	{

		int ii, jj;
		for (ii = 0; ii <= (PIPO64_SBOX_BIT_SIZE - 1); ii++)
		{
			out_LC_perm_wise.sub_i[ii] = 0;
			out_LC_perm_wise.sub_o[ii] = 0;
			out_LC_perm_wise.dif_o[ii] = 0;
			for (jj = 0; jj <= (PIPO64_NUM_SBOX_IN_A_STATE - 1); jj++)
			{

				out_LC_perm_wise.sub_i[ii] |= ((((uint8_t)LIN_TRAIL_FOR_OUT[__FF__].sub_i[jj] >> ((PIPO64_SBOX_BIT_SIZE - 1) - ii)) & 0x1) << ((PIPO64_NUM_SBOX_IN_A_STATE - 1) - jj));
				out_LC_perm_wise.sub_o[ii] |= ((((uint8_t)LIN_TRAIL_FOR_OUT[__FF__].sub_o[jj] >> ((PIPO64_SBOX_BIT_SIZE - 1) - ii)) & 0x1) << ((PIPO64_NUM_SBOX_IN_A_STATE - 1) - jj));
				out_LC_perm_wise.dif_o[ii] |= ((((uint8_t)LIN_TRAIL_FOR_OUT[__FF__].dif_o[jj] >> ((PIPO64_SBOX_BIT_SIZE - 1) - ii)) & 0x1) << ((PIPO64_NUM_SBOX_IN_A_STATE - 1) - jj));
			}
		}

		fprintf(ofp, "R %2d: SBOXIN              | ", __FF__);
		for (i = 0; i < PIPO64_NUM_SBOX_IN_A_STATE; i++)
		{
			fprintf(ofp, "%02X ", out_LC_perm_wise.sub_i[i]);
		}
		fprintf(ofp, "\t");

		fprintf(ofp, "R %2d: SBOXIN(Sbox-wise)   | ", __FF__);
		for (i = 0; i < PIPO64_NUM_SBOX_IN_A_STATE; i++)
		{
			fprintf(ofp, "%02X ", LIN_TRAIL_FOR_OUT[__FF__].sub_i[i]);
		}
		fprintf(ofp, "\n");


		fprintf(ofp, "R %2d: SBOXOUT             | ", __FF__);
		for (i = 0; i < PIPO64_NUM_SBOX_IN_A_STATE; i++)
		{
			fprintf(ofp, "%02X ", out_LC_perm_wise.sub_o[i]);
		}
		fprintf(ofp, "\t");

		fprintf(ofp, "R %2d: SBOXOUT(Sbox-wise)  | ", __FF__);
		for (i = 0; i < PIPO64_NUM_SBOX_IN_A_STATE; i++)
		{
			fprintf(ofp, "%02X ", LIN_TRAIL_FOR_OUT[__FF__].sub_o[i]);
		}
		fprintf(ofp, "\n");

		fprintf(ofp, "R %2d: DIFFOUT             | ", __FF__);
		for (i = 0; i < PIPO64_NUM_SBOX_IN_A_STATE; i++)
		{
			fprintf(ofp, "%02X ", out_LC_perm_wise.dif_o[i]);
		}
		fprintf(ofp, "\n");

		if (LIN_TRAIL_FOR_OUT[__FF__].c.sign == NEGA)
		{
			fprintf(ofp, "R %2d: CORR     | -2^{%0.4lf}\n", __FF__, LIN_TRAIL_FOR_OUT[__FF__].c.magnitude);
		}
		else if (LIN_TRAIL_FOR_OUT[__FF__].c.sign == POSI)
		{
			fprintf(ofp, "R %2d: CORR     | +2^{%0.4lf}\n", __FF__, LIN_TRAIL_FOR_OUT[__FF__].c.magnitude);
		}
		else if (LIN_TRAIL_FOR_OUT[__FF__].c.sign == ZERO)
		{
			fprintf(ofp, "R %2d: CORR     | ZERO\n", __FF__);
		}

		fprintf(ofp, "========================\n");
	}
	fclose(ofp);
}