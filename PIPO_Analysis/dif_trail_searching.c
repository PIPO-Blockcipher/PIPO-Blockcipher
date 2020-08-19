#include "pipo_spec.h"
#include "dc_prob.h"
#include "searching.h"
#include <stdio.h>
#include <memory.h>
#include <stdlib.h>

extern int PIPO64_OFFSET[8];

extern PROB_t MAX_PROB; //non-one probability
extern PROB_t MIN_PROB; //non-zero probability
extern DP_I_O_PROB_t	DP_IOP[PIPO64_SBOX_CARDINALITY * PIPO64_SBOX_CARDINALITY];
extern DP_I_PROB_t		DP_IP_FO[PIPO64_SBOX_CARDINALITY][PIPO64_SBOX_CARDINALITY];
extern DP_O_PROB_t		DP_OP_FI[PIPO64_SBOX_CARDINALITY][PIPO64_SBOX_CARDINALITY];
extern SUB_CNT_t DP_NUM_IOP_NONZERO;
extern SUB_CNT_t DP_NUM_IOP_MAX;
extern SUB_CNT_t DP_NUM_IP_FO_NONZERO[PIPO64_SBOX_CARDINALITY];
extern SUB_CNT_t DP_NUM_IP_FO_MAX[PIPO64_SBOX_CARDINALITY];
extern SUB_CNT_t DP_NUM_OP_FI_NONZERO[PIPO64_SBOX_CARDINALITY];
extern SUB_CNT_t DP_NUM_OP_FI_MAX[PIPO64_SBOX_CARDINALITY];


//here
PROB_t	DP_BOUNDS[PIPO64_NUM_ROUND + 1];
DC_1ROUND_CHAR_t DIFF_TRAIL_IN_PROG[PIPO64_NUM_ROUND + 1];
DC_1ROUND_CHAR_t DIFF_TRAIL_FOR_OUT[PIPO64_NUM_ROUND + 1];
PROB_t	DP_BOUND_IN_PROG;
FLAG_t	TOUCH_THE_LEAF;
char	OUT_FILE_NAME[512];
FILE  * ofp;


void SPN_Round1_Prob_Only(int target_round);
void SPN_Round1_j_Prob_Only(
	SUB_STATE_t X1,
	SUB_STATE_t Y1,
	PROB_t P1[PIPO64_SBOX_CARDINALITY],
	PROB_t BEF_DET_PROB,
	int target_round,
	TRUNC_STATE_t AT1,
	int j);
void SPN_Roundi_Prob_Only(PROB_t BEF_DET_PROB, int target_round, int i);
void SPN_Roundi_j_Prob_Only(
	SUB_STATE_t Xi,
	SUB_STATE_t Yi,
	PROB_t Pi[PIPO64_NUM_SBOX_IN_A_STATE],
	PROB_t BEF_DET_PROB,
	int target_round,
	int i,
	int j);
void SPN_Roundn_Prob_Only(PROB_t BEF_DET_PROB, int n);
void SPN_Roundn_j_Prob_Only(
	SUB_STATE_t Xn,
	SUB_STATE_t Yn,
	PROB_t Pn[PIPO64_NUM_SBOX_IN_A_STATE],
	PROB_t BEF_DET_PROB,
	int n,
	int j);


static inline PROB_t put_probs(PROB_t p[PIPO64_NUM_SBOX_IN_A_STATE])
{
	int i;
	PROB_t P;
	P = p[0];
	for (i = 1; i < PIPO64_NUM_SBOX_IN_A_STATE; i++)
	{
		MUL_PROB(&P, P, p[i]);
	}
	return P;
}

static inline void init_probs(PROB_t	p[PIPO64_NUM_SBOX_IN_A_STATE])
{
	int i;
	for (i = 0; i < PIPO64_NUM_SBOX_IN_A_STATE; i++)
	{
		p[i] = ONE_PROB;
	}
}


static inline int bound_checker_prob(PROB_t check)
{
	if (TOUCH_THE_LEAF == TRUE)
	{
		if (COMP_PROB(check, DP_BOUND_IN_PROG) != LEFT)
			return EXCEED_BOUND;
		else
			return UNDER_BOUND;
	}
	else
	{
		if (COMP_PROB(check, DP_BOUND_IN_PROG) == RIGHT)
			return EXCEED_BOUND;
		else
			return UNDER_BOUND;
	}
}

static inline int expect_state_prob_at_1_j(PROB_t * expected_prob, TRUNC_STATE_t AT, int j)
{
	int first_active = THE_LAST_WORD;
	PROB_t tmp_cum_prob = ONE_PROB;
	int wrd_idx;
	if (j == ROUND_START)
	{
		for (wrd_idx = 0; wrd_idx < PIPO64_NUM_SBOX_IN_A_STATE; wrd_idx++)
		{
			if ((AT & (0x1<<wrd_idx)) != 0)
			{
				MUL_PROB(&tmp_cum_prob, tmp_cum_prob, MAX_PROB);
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
				MUL_PROB(&tmp_cum_prob, tmp_cum_prob, MAX_PROB);
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
	*expected_prob = tmp_cum_prob;
	return first_active;
}



static inline int expect_state_prob_at_j(PROB_t * expected_prob, SUB_STATE_t X, int j)
{
	int first_active = THE_LAST_WORD;
	PROB_t tmp_cum_prob = ONE_PROB;
	int wrd_idx;
	if (j == ROUND_START)
	{
		for (wrd_idx = 0; wrd_idx < PIPO64_NUM_SBOX_IN_A_STATE; wrd_idx++)
		{
			if (X[wrd_idx] != 0)
			{
				MUL_PROB(&tmp_cum_prob, tmp_cum_prob, MAX_PROB);
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
				MUL_PROB(&tmp_cum_prob, tmp_cum_prob, MAX_PROB);
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
	*expected_prob = tmp_cum_prob;
	return first_active;
}

static inline void expect_next_state_prob_at_j(PROB_t * expected_next_prob, SUB_STATE_t Y, int j)
{
	SUB_STATE_t copied = { 0, };
	DIF_STATE_t dif_part;
	PROB_t tmp_cum_prob = ONE_PROB;
	int wrd_idx;
	memcpy(copied, Y, sizeof(SUB_WRD_t) * (j + 1));
	
	PIPO64_R(dif_part, copied);

	for (wrd_idx = 0; wrd_idx < PIPO64_NUM_SBOX_IN_A_STATE; wrd_idx++)
	{
		if (dif_part[wrd_idx] != 0)
		{
			MUL_PROB(&tmp_cum_prob, tmp_cum_prob, MAX_PROB);
		}
	}

	*expected_next_prob = tmp_cum_prob;
}

void SPN_Roundn_j_Prob_Only(SUB_STATE_t Xn, SUB_STATE_t Yn, PROB_t Pn[PIPO64_NUM_SBOX_IN_A_STATE], PROB_t BEF_DET_PROB, int n, int j)
{
	SUB_WRD_t	Xn_j = Xn[j]; //It is fixed from before
	SUB_WRD_t	Yn_j = DP_OP_FI[Xn_j][0].o;
	PROB_t		Pn_j = DP_OP_FI[Xn_j][0].p; //max prob with fixed in
	int			next_word_idx;
	PROB_t		_n_round_expected;
	PROB_t		_n_bound;
	PROB_t		DET_PROB;

	MUL_PROB(&DET_PROB, BEF_DET_PROB, Pn_j);
	next_word_idx = expect_state_prob_at_j(&_n_round_expected, Xn, j);
	MUL_PROB(&_n_bound, _n_round_expected, DET_PROB);						//[1~n]R

	//pruning
	if (bound_checker_prob(_n_bound) == EXCEED_BOUND)
		return;

	Yn[j] = Yn_j;
	Pn[j] = Pn_j;

	
	if (next_word_idx == THE_LAST_WORD)  //when this word is the last word
	{
		int L;

		TOUCH_THE_LEAF = TRUE;
		printf("--New Bound!! 2^{%0.4lf}\n", DET_PROB);
		//update Progress bound
		DP_BOUND_IN_PROG = DET_PROB;

		//Copy 1~n-1-Round states to Out trail 
		for (L = 1; L <= n - 1; L++)
		{
			memcpy(DIFF_TRAIL_FOR_OUT[L].sub_i, DIFF_TRAIL_IN_PROG[L].sub_i, sizeof(SUB_STATE_t));
			memcpy(DIFF_TRAIL_FOR_OUT[L].sub_o, DIFF_TRAIL_IN_PROG[L].sub_o, sizeof(SUB_STATE_t));
			memcpy(DIFF_TRAIL_FOR_OUT[L].dif_o, DIFF_TRAIL_IN_PROG[L].dif_o, sizeof(DIF_STATE_t));
			DIFF_TRAIL_FOR_OUT[L].p = DIFF_TRAIL_IN_PROG[L].p;
		}
		//Copy n-Round state to Out trail
		memcpy(DIFF_TRAIL_FOR_OUT[n].sub_i, Xn, sizeof(SUB_STATE_t));
		memcpy(DIFF_TRAIL_FOR_OUT[n].sub_o, Yn, sizeof(SUB_STATE_t));
		PIPO64_R(DIFF_TRAIL_FOR_OUT[n].dif_o, Yn);
		DIFF_TRAIL_FOR_OUT[n].p = put_probs(Pn);
		DP_BOUNDS[n] = DET_PROB;
	}
	else if (next_word_idx < PIPO64_NUM_SBOX_IN_A_STATE) //when this word is not the last word
	{
		//Move to next word
		SPN_Roundn_j_Prob_Only(Xn, Yn, Pn, DET_PROB, n, next_word_idx);
	}
}



void SPN_Roundn_Prob_Only(PROB_t BEF_DET_PROB, int n)
{
	SUB_STATE_t Xn;
	SUB_STATE_t Yn;
	PROB_t		Pn[PIPO64_NUM_SBOX_IN_A_STATE];
	PROB_t		_n_round_expected;
	PROB_t		_n_bound;
	int			next_word_idx;

	memcpy(Xn, DIFF_TRAIL_IN_PROG[n - 1].dif_o, sizeof(SUB_STATE_t));

	_n_bound = BEF_DET_PROB;														//[1~(n-1)]R
	next_word_idx = expect_state_prob_at_j(&_n_round_expected, Xn, ROUND_START);	//nR
	MUL_PROB(&_n_bound, _n_bound, _n_round_expected);

	if (bound_checker_prob(_n_bound) != EXCEED_BOUND)
	{
		//init the sub out state and probabilities
		memset(Yn, 0, sizeof(SUB_STATE_t));
		init_probs(Pn);

		SPN_Roundn_j_Prob_Only(Xn, Yn, Pn, BEF_DET_PROB, n, next_word_idx);
	}
}

void SPN_Roundi_j_Prob_Only(SUB_STATE_t Xi, SUB_STATE_t Yi, PROB_t Pi[PIPO64_NUM_SBOX_IN_A_STATE], PROB_t BEF_DET_PROB, int target_round, int i, int j)
{
	SUB_CNT_t	_i_j;
	SUB_WRD_t	Xi_j = Xi[j]; //It is fixed from before
	SUB_WRD_t	Yi_j;
	PROB_t		Pi_j;
	int			next_word_idx;
	PROB_t		_i_round_expected;
	PROB_t		_i1_round_expected;
	PROB_t		_i_bound;
	PROB_t		_i1_bound;
	PROB_t		DET_PROB;

	for (_i_j = 0; _i_j < DP_NUM_OP_FI_NONZERO[Xi_j]; _i_j++)
	{
		Yi_j = DP_OP_FI[Xi_j][_i_j].o;
		Pi_j = DP_OP_FI[Xi_j][_i_j].p;

		MUL_PROB(&DET_PROB, BEF_DET_PROB, Pi_j);								
		
		
		next_word_idx = expect_state_prob_at_j(&_i_round_expected, Xi, j);
		MUL_PROB(&_i_bound, _i_round_expected, DET_PROB);						//[1~i]R
		MUL_PROB(&_i_bound, _i_bound, DP_BOUNDS[target_round - i]);				//[i+1~n]R

		//pruning by using descending order
		if (bound_checker_prob(_i_bound) == EXCEED_BOUND)
			break;

		//store current word
		Yi[j] = Yi_j;
		Pi[j] = Pi_j;


		MUL_PROB(&_i1_bound, _i_round_expected, DET_PROB);							//[1~i]R
		expect_next_state_prob_at_j(&_i1_round_expected, Yi, j);
		MUL_PROB(&_i1_bound, _i1_bound, _i1_round_expected);						//i+1R

		if (i < (target_round - 1))
		{
			MUL_PROB(&_i1_bound, _i1_bound, DP_BOUNDS[target_round - 1 - i]);		//[i+2~n]R
		}


		//pruning
		if (bound_checker_prob(_i1_bound) == EXCEED_BOUND)
			continue; //next value

		if (next_word_idx == THE_LAST_WORD)  //when this word is the last word
		{
			memcpy(DIFF_TRAIL_IN_PROG[i].sub_i, Xi, sizeof(SUB_STATE_t));
			memcpy(DIFF_TRAIL_IN_PROG[i].sub_o, Yi, sizeof(SUB_STATE_t));
			PIPO64_R(DIFF_TRAIL_IN_PROG[i].dif_o, DIFF_TRAIL_IN_PROG[i].sub_o);
			DIFF_TRAIL_IN_PROG[i].p = put_probs(Pi);
			if (i == (target_round - 1))
			{
				SPN_Roundn_Prob_Only(DET_PROB, target_round);
			}
			else if (i < (target_round - 1))
			{
				SPN_Roundi_Prob_Only(DET_PROB, target_round, i + 1);
			}
		}
		else if (next_word_idx < PIPO64_NUM_SBOX_IN_A_STATE) //when this word is not the last word
		{
			//Move to next word
			SPN_Roundi_j_Prob_Only(Xi, Yi, Pi, DET_PROB, target_round, i, next_word_idx);
		}
	}
}

void SPN_Roundi_Prob_Only(PROB_t BEF_DET_PROB, int target_round, int i)
{
	SUB_STATE_t Xi;
	SUB_STATE_t Yi;
	PROB_t		Pi[PIPO64_NUM_SBOX_IN_A_STATE];
	PROB_t		_i_round_expected;
	PROB_t		_i_bound;
	int			next_word_idx;

	memcpy(Xi, DIFF_TRAIL_IN_PROG[i - 1].dif_o, sizeof(SUB_STATE_t));

	_i_bound = BEF_DET_PROB;														//[1~(i-1)]R
	next_word_idx = expect_state_prob_at_j(&_i_round_expected, Xi, ROUND_START);	//iR
	MUL_PROB(&_i_bound, _i_bound, _i_round_expected);
	MUL_PROB(&_i_bound, _i_bound, DP_BOUNDS[target_round - i]);						//[(i+1)~n]R

	if (bound_checker_prob(_i_bound) != EXCEED_BOUND)
	{
		//init the sub out state and probabilities
		memset(Yi, 0, sizeof(SUB_STATE_t));
		init_probs(Pi);

		SPN_Roundi_j_Prob_Only(Xi, Yi, Pi, BEF_DET_PROB, target_round, i, next_word_idx);
	}
}

void SPN_Round1_j_Prob_Only(SUB_STATE_t X1, SUB_STATE_t Y1, PROB_t P1[PIPO64_SBOX_CARDINALITY], PROB_t BEF_DET_PROB, int target_round, TRUNC_STATE_t AT1, int j)
{
	SUB_CNT_t	_1_j;
	SUB_WRD_t	X1_j;
	SUB_WRD_t	Y1_j;
	PROB_t		P1_j;
	int			next_word_idx;
	PROB_t		_1_round_expected;
	PROB_t		_2_round_expected;
	PROB_t		_1_bound;
	PROB_t      _2_bound;
	PROB_t		DET_PROB;

	// this word is always active
	for (_1_j = 0x1; _1_j < PIPO64_SBOX_CARDINALITY; _1_j++)
	{
		X1_j = DP_IP_FO[_1_j][0].i;
		Y1_j = (SUB_WRD_t)_1_j;
		P1_j = DP_IP_FO[_1_j][0].p;

		MUL_PROB(&DET_PROB, BEF_DET_PROB, P1_j);

		next_word_idx = expect_state_prob_at_1_j(&_1_round_expected, AT1, j);
		MUL_PROB(&_1_bound, _1_round_expected, DET_PROB);			//1R
		MUL_PROB(&_1_bound, _1_bound, DP_BOUNDS[target_round - 1]); //[2~n]R

		//pruning
		if (bound_checker_prob(_1_bound) == EXCEED_BOUND)
			continue; //need to check next value on j word

		//store current word
		X1[j] = X1_j;
		Y1[j] = Y1_j;
		P1[j] = P1_j;


		MUL_PROB(&_2_bound, _1_round_expected, DET_PROB);				//1R
		expect_next_state_prob_at_j(&_2_round_expected, Y1, j);
		MUL_PROB(&_2_bound, _2_round_expected, _2_bound);				//2R
		
		if (target_round > 2)
		{
			MUL_PROB(&_2_bound, _2_bound, DP_BOUNDS[target_round - 2]); //[3~n]R
		}

		//pruning
		if (bound_checker_prob(_2_bound) == EXCEED_BOUND)
			continue; //need to check next value on j word

		if (next_word_idx == THE_LAST_WORD) //when this word is the last word
		{
			memcpy(DIFF_TRAIL_IN_PROG[1].sub_i, X1, sizeof(SUB_STATE_t));
			memcpy(DIFF_TRAIL_IN_PROG[1].sub_o, Y1, sizeof(SUB_STATE_t));
			PIPO64_R(DIFF_TRAIL_IN_PROG[1].dif_o, DIFF_TRAIL_IN_PROG[1].sub_o);
			DIFF_TRAIL_IN_PROG[1].p = put_probs(P1);

			if (target_round == 2)
			{
				//Move to the last round
				SPN_Roundn_Prob_Only(DET_PROB, target_round);
			}
			else
			{
				//Move to next(2) round
				SPN_Roundi_Prob_Only(DET_PROB, target_round, 2);
			}
		}
		else if (next_word_idx < PIPO64_NUM_SBOX_IN_A_STATE) //when this word is not the last word
		{
			//Move to next word
			SPN_Round1_j_Prob_Only(X1, Y1, P1, DET_PROB, target_round, AT1, next_word_idx);
		}
	}
}

void SPN_Round1_Prob_Only(int target_round)
{
	SUB_STATE_t		X1;
	SUB_STATE_t		Y1;
	PROB_t			P1[PIPO64_NUM_SBOX_IN_A_STATE];
	TRUNC_STATE_t	AT1;
	int				idx_1round_trunc;
	PROB_t			_1_round_expected;
	PROB_t			_1_bound;
	int				next_word_idx;
	PROB_t			DET_PROB = ONE_PROB;

	for (idx_1round_trunc = 0; idx_1round_trunc < SIZE_OF_1R_TRUNC; idx_1round_trunc++)
	{
		AT1 = _1ROUND_ACTIVE_TRUNCATIONS[idx_1round_trunc];

		next_word_idx = expect_state_prob_at_1_j(&_1_round_expected, AT1, ROUND_START); //1R
		MUL_PROB(&_1_bound, _1_round_expected, DP_BOUNDS[target_round - 1]);			//[2~n]R

		//pruning
		if (bound_checker_prob(_1_bound) == EXCEED_BOUND)
			break; 

		//init
		memset(X1, 0, sizeof(SUB_STATE_t));
		memset(Y1, 0, sizeof(SUB_STATE_t));
		init_probs(P1);

		SPN_Round1_j_Prob_Only(X1, Y1, P1, DET_PROB, target_round, AT1, next_word_idx);
	}
}

void DC_fprintf(int round_idx);

int Best_Trail_Prob_Only(int target_round)
{
	int round_idx;
	FLAG_t first_trial_flag;

	DP_BOUNDS[1] = MAX_PROB;
	printf("=================Searching(Probability Only)...=================\n\n");
	printf("==Set   [ 1]-Round Bound with the Known Probability 2^{%0.4lf}\n", DP_BOUNDS[1]);


	//Start searching
	for (round_idx = 2; round_idx <= target_round; round_idx++)
	{
		first_trial_flag = TRUE;
		TOUCH_THE_LEAF = FALSE;

		sprintf(OUT_FILE_NAME, "[%d%d%d%d%d%d%d%d]_%dRound_Best_Differential_Trail.txt",
			PIPO64_OFFSET[0], PIPO64_OFFSET[1], PIPO64_OFFSET[2], PIPO64_OFFSET[3],
			PIPO64_OFFSET[4], PIPO64_OFFSET[5], PIPO64_OFFSET[6], PIPO64_OFFSET[7],
			round_idx);

		while (TOUCH_THE_LEAF != TRUE)
		{
			if (first_trial_flag == TRUE)
			{
				first_trial_flag = FALSE;
				MUL_PROB(&DP_BOUND_IN_PROG, DP_BOUNDS[round_idx - 1], MAX_PROB);
				Realtime_Print(START_TIME);
			}
			else
			{
				MUL_PROB(&DP_BOUND_IN_PROG, DP_BOUND_IN_PROG, MIN_PROB);
				Realtime_Print(END_TIME);		
			}
			printf("==Start [%2d]-Round With Bound 2^{%0.4lf}==\n", round_idx, DP_BOUND_IN_PROG);
			/*Finding Trail*/
			SPN_Round1_Prob_Only(round_idx);
			/***************/
		}
		Realtime_Print(END_TIME);
		printf("==End   [%2d]-Round With Bound 2^{%0.4lf}==\n\n", round_idx, DP_BOUNDS[round_idx]);
		DC_fprintf(round_idx);
	}
	printf("\n\n Everything's done \n\n");
	printf("===================================================================\n\n");
	for (round_idx = 1; round_idx <= target_round; round_idx++)
	{
		printf("==   [%2d]-Round Best Differential Probabilty : 2^{%0.4lf}\n", round_idx, DP_BOUNDS[round_idx]);
	}

	return 0;
}









void DC_fprintf(int round_idx)
{
	int __FF__;
	int i;
	ofp = fopen(OUT_FILE_NAME, "w");

	fprintf(ofp, "[%2d] Round Best : 2^{%0.4lf} \n", round_idx, DP_BOUND_IN_PROG);
	fprintf(ofp, "========================\n");

	DC_1ROUND_CHAR_t out_DC_perm_wise;

	for (__FF__ = 1; __FF__ <= round_idx; __FF__++)
	{

		int ii, jj;
		for (ii = 0; ii <= (PIPO64_SBOX_BIT_SIZE - 1); ii++)
		{
			out_DC_perm_wise.sub_i[ii] = 0;
			out_DC_perm_wise.sub_o[ii] = 0;
			out_DC_perm_wise.dif_o[ii] = 0;
			for (jj = 0; jj <= (PIPO64_NUM_SBOX_IN_A_STATE - 1); jj++)
			{

				out_DC_perm_wise.sub_i[ii] |= ((((uint8_t)DIFF_TRAIL_FOR_OUT[__FF__].sub_i[jj] >> ((PIPO64_SBOX_BIT_SIZE - 1) - ii)) & 0x1) << ((PIPO64_NUM_SBOX_IN_A_STATE - 1) - jj));
				out_DC_perm_wise.sub_o[ii] |= ((((uint8_t)DIFF_TRAIL_FOR_OUT[__FF__].sub_o[jj] >> ((PIPO64_SBOX_BIT_SIZE - 1) - ii)) & 0x1) << ((PIPO64_NUM_SBOX_IN_A_STATE - 1) - jj));
				out_DC_perm_wise.dif_o[ii] |= ((((uint8_t)DIFF_TRAIL_FOR_OUT[__FF__].dif_o[jj] >> ((PIPO64_SBOX_BIT_SIZE - 1) - ii)) & 0x1) << ((PIPO64_NUM_SBOX_IN_A_STATE - 1) - jj));
			}
		}

		fprintf(ofp, "R %2d: SBOXIN              | ", __FF__);
		for (i = 0; i < PIPO64_NUM_SBOX_IN_A_STATE; i++)
		{
			fprintf(ofp, "%02X ", out_DC_perm_wise.sub_i[i]);
		}
		fprintf(ofp, "\t");

		fprintf(ofp, "R %2d: SBOXIN(Sbox-wise)   | ", __FF__);
		for (i = 0; i < PIPO64_NUM_SBOX_IN_A_STATE; i++)
		{
			fprintf(ofp, "%02X ", DIFF_TRAIL_FOR_OUT[__FF__].sub_i[i]);
		}
		fprintf(ofp, "\n");


		fprintf(ofp, "R %2d: SBOXOUT             | ", __FF__);
		for (i = 0; i < PIPO64_NUM_SBOX_IN_A_STATE; i++)
		{
			fprintf(ofp, "%02X ", out_DC_perm_wise.sub_o[i]);
		}
		fprintf(ofp, "\t");

		fprintf(ofp, "R %2d: SBOXOUT(Sbox-wise)  | ", __FF__);
		for (i = 0; i < PIPO64_NUM_SBOX_IN_A_STATE; i++)
		{
			fprintf(ofp, "%02X ", DIFF_TRAIL_FOR_OUT[__FF__].sub_o[i]);
		}
		fprintf(ofp, "\n");



		fprintf(ofp, "R %2d: DIFFOUT             | ", __FF__);
		for (i = 0; i < PIPO64_NUM_SBOX_IN_A_STATE; i++)
		{
			fprintf(ofp, "%02X ", out_DC_perm_wise.dif_o[i]);
		}
		fprintf(ofp, "\n");

		fprintf(ofp, "R %2d: PROB     | 2^{%0.4lf}\n", __FF__, DIFF_TRAIL_FOR_OUT[__FF__].p);

		fprintf(ofp, "========================\n");
	}
	fclose(ofp);
}