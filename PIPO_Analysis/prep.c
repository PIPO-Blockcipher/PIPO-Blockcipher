#include "pipo_spec.h"
#include "dc_prob.h"
#include "lc_corr.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

//extern
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

//Here
SUB_CNT_t DDT[PIPO64_SBOX_CARDINALITY][PIPO64_SBOX_CARDINALITY];
PROB_t			 PROB_DDT[PIPO64_SBOX_CARDINALITY][PIPO64_SBOX_CARDINALITY];
SUB_CNT_t LAT[PIPO64_SBOX_CARDINALITY][PIPO64_SBOX_CARDINALITY];
CORR_t			 CORR_LAT[PIPO64_SBOX_CARDINALITY][PIPO64_SBOX_CARDINALITY];

//Compare Function for descending order.
int descending_P(const void *first, const void *second)
{
	PROB_t first_p = *((PROB_t*)first);
	PROB_t second_p = *((PROB_t*)second);
	return COMP_PROB(second_p, first_p);
}

//Compare Function for descending order.
int descending_IOP(const void *first, const void *second)
{
	const DP_I_O_PROB_t * t_first = first;
	const DP_I_O_PROB_t * t_second = second;
	PROB_t first_p = t_first->p;
	PROB_t second_p = t_second->p;
	return COMP_PROB(second_p, first_p);
}

//Compare Function for descending order.
int descending_OP(const void *first, const void *second)
{
	const DP_O_PROB_t * t_first = first;
	const DP_O_PROB_t * t_second = second;
	PROB_t first_p = t_first->p;
	PROB_t second_p = t_second->p;
	return COMP_PROB(second_p, first_p);
}

//Compare Function for descending order.
int descending_IP(const void *first, const void *second)
{
	const DP_I_PROB_t * t_first = first;
	const DP_I_PROB_t * t_second = second;
	PROB_t first_p = t_first->p;
	PROB_t second_p = t_second->p;
	return COMP_PROB(second_p, first_p);
}


int Compute_DDT(void)
{
	SUB_CNT_t i;
	SUB_CNT_t delta_i;
	SUB_CNT_t delta_o;

	//init
	for (delta_i = 0; delta_i < PIPO64_SBOX_CARDINALITY; delta_i++)
	{
		for (delta_o = 0; delta_o < PIPO64_SBOX_CARDINALITY; delta_o++)
		{
			DDT[delta_i][delta_o] = 0;
		}
	}

	//compute DDT
	for (delta_i = 0; delta_i < PIPO64_SBOX_CARDINALITY; delta_i++) // difference
	{
		for (i = 0; i < PIPO64_SBOX_CARDINALITY; i++) // plaintext
		{
			SUB_WRD_t in = (SUB_WRD_t)i;
			SUB_WRD_t delta_o = PIPO64_Sbox[in] ^ PIPO64_Sbox[in^delta_i];
			DDT[delta_i][delta_o]++;
		}
	}

	MAX_PROB = ZERO_PROB;
	MIN_PROB = ONE_PROB;
	//compute PROB_DDT
	for (delta_i = 0; delta_i < PIPO64_SBOX_CARDINALITY; delta_i++)
	{
		for (delta_o = 0; delta_o < PIPO64_SBOX_CARDINALITY; delta_o++)
		{
			SUB_CNT_t this_DDT_ent = DDT[delta_i][delta_o];

			if (this_DDT_ent == 0)
			{
				PROB_DDT[delta_i][delta_o] = (PROB_t)ZERO_PROB;
			}
			else //non-zero
			{
				if (this_DDT_ent == PIPO64_SBOX_CARDINALITY)
				{
					PROB_DDT[delta_i][delta_o] = (PROB_t)ONE_PROB;
				}
				else
				{
					PROB_DDT[delta_i][delta_o] = log2((PROB_t)this_DDT_ent / (PROB_t)PIPO64_SBOX_CARDINALITY);
				}

				if (COMP_PROB(MIN_PROB, PROB_DDT[delta_i][delta_o]) == LEFT)
				{
					MIN_PROB = PROB_DDT[delta_i][delta_o];
				}
			}
			if ((delta_i != 0) || (delta_o != 0))
			{
				if (COMP_PROB(MAX_PROB, PROB_DDT[delta_i][delta_o]) == RIGHT)
				{
					MAX_PROB = PROB_DDT[delta_i][delta_o];
				}
			}
			
		}
	}
	return 0;
}

int Prep_DDT_Info(void)
{
	SUB_CNT_t delta_i;
	SUB_CNT_t delta_o;
	int idx;
	PROB_t the_prob;

	// DDT information rearrange
	// all I_O_PROB
	// O_PROB with fixed input
	// I_PROB with fixed output
	//rearrange
	for (delta_i = 0; delta_i < PIPO64_SBOX_CARDINALITY; delta_i++)
	{
		for (delta_o = 0; delta_o < PIPO64_SBOX_CARDINALITY; delta_o++)
		{
			//all I_O_PROB
			the_prob = PROB_DDT[delta_i][delta_o];
			DP_IOP[delta_i * PIPO64_SBOX_CARDINALITY + delta_o].i = (SUB_WRD_t)delta_i;
			DP_IOP[delta_i * PIPO64_SBOX_CARDINALITY + delta_o].o = (SUB_WRD_t)delta_o;
			DP_IOP[delta_i * PIPO64_SBOX_CARDINALITY + delta_o].p = (PROB_t)the_prob;
			//O_PROB with fixed input
			DP_OP_FI[delta_i][delta_o].o = (SUB_WRD_t)delta_o;
			DP_OP_FI[delta_i][delta_o].p = (PROB_t)the_prob;
			//I_PROB with fixed output
			DP_IP_FO[delta_o][delta_i].i = (SUB_WRD_t)delta_i;
			DP_IP_FO[delta_o][delta_i].p = (PROB_t)the_prob;
		}
	}

	//sorting all I_O_PROB
	qsort((void*)DP_IOP, PIPO64_SBOX_CARDINALITY * PIPO64_SBOX_CARDINALITY, sizeof(DP_I_O_PROB_t), descending_IOP);

	//sorting O_PROB with fixed input
	for (delta_i = 0; delta_i < PIPO64_SBOX_CARDINALITY; delta_i++)
	{
		qsort((void*)DP_OP_FI[delta_i], PIPO64_SBOX_CARDINALITY, sizeof(DP_O_PROB_t), descending_OP);
	}
	//sorting I_PROB with fixed output
	for (delta_o = 0; delta_o < PIPO64_SBOX_CARDINALITY; delta_o++)
	{
		qsort((void*)DP_IP_FO[delta_o], PIPO64_SBOX_CARDINALITY, sizeof(DP_I_PROB_t), descending_IP);
	}

	//Counting the numbers of each information
	/*
	*  1) The number of (Input,Output) with NON-zero probability.
	*  2) The number of (Input,Output) with the best probability.
	*  3) The number of (Input)/(Output) with NON-zero probability and each fixed (Output)/(Input).
	*  4) The number of (Input)/(Output) with the best probability and each fixed (Output)/(Input).
	*/

	////init
	DP_NUM_IOP_NONZERO = 0;
	DP_NUM_IOP_MAX = 0;
	for (delta_i = 0; delta_i < PIPO64_SBOX_CARDINALITY; delta_i++)
	{
		DP_NUM_OP_FI_NONZERO[delta_i] = 0;
		DP_NUM_OP_FI_MAX[delta_i] = 0;
	}
	for (delta_o = 0; delta_o < PIPO64_SBOX_CARDINALITY; delta_o++)
	{
		DP_NUM_IP_FO_NONZERO[delta_o] = 0;
		DP_NUM_IP_FO_MAX[delta_o] = 0;
	}

	////Counting IO_PROB
	for (idx = 0; idx < PIPO64_SBOX_CARDINALITY * PIPO64_SBOX_CARDINALITY; idx++)
	{
		PROB_t IO_prob = DP_IOP[idx].p;
		PROB_t max_IO_prob = DP_IOP[0].p;
		if (COMP_PROB(IO_prob, ZERO_PROB) != EQUAL)
		{
			DP_NUM_IOP_NONZERO++;
		}
		if (COMP_PROB(max_IO_prob, IO_prob) == EQUAL)
		{
			DP_NUM_IOP_MAX++;
		}
	}

	////Counting O_PROB
	for (delta_i = 0; delta_i < PIPO64_SBOX_CARDINALITY; delta_i++)
	{
		for (idx = 0; idx < PIPO64_SBOX_CARDINALITY; idx++)
		{
			PROB_t O_prob = DP_OP_FI[delta_i][idx].p;
			PROB_t max_O_prob = DP_OP_FI[delta_i][0].p;
			if (COMP_PROB(O_prob, ZERO_PROB) != EQUAL)
			{
				DP_NUM_OP_FI_NONZERO[delta_i]++;
			}
			if (COMP_PROB(max_O_prob, O_prob) == EQUAL)
			{
				DP_NUM_OP_FI_MAX[delta_i]++;
			}
		}
	}

	////Counting I_PROB
	for (delta_o = 0; delta_o < PIPO64_SBOX_CARDINALITY; delta_o++)
	{
		for (idx = 0; idx < PIPO64_SBOX_CARDINALITY; idx++)
		{
			PROB_t I_prob = DP_IP_FO[delta_o][idx].p;
			PROB_t max_I_prob = DP_IP_FO[delta_o][0].p;
			if (COMP_PROB(I_prob, ZERO_PROB) != EQUAL)
			{
				DP_NUM_IP_FO_NONZERO[delta_o]++;
			}
			if (COMP_PROB(max_I_prob, I_prob) == EQUAL)
			{
				DP_NUM_IP_FO_MAX[delta_o]++;
			}
		}
	}

	return 0;
}


int Prep_Dif_Trail_Searching(void)
{
	Compute_DDT();
	Prep_DDT_Info();
	return 0;
}

//Compare Function for descending order.
int descending_C(const void *first, const void *second)
{
	const CORR_t first_c = *((CORR_t*)first);
	const CORR_t second_c = *((CORR_t*)second);
	return COMP_ABS_CORR(second_c, first_c);
}


//Compare Function for descending order.
int descending_IOC(const void *first, const void *second)
{
	const LC_I_O_CORR_t * t_first = first;
	const LC_I_O_CORR_t * t_second = second;
	CORR_t first_c = t_first->c;
	CORR_t second_c = t_second->c;
	return COMP_ABS_CORR(second_c, first_c);
}

//Compare Function for descending order.
int descending_OC(const void *first, const void *second)
{
	const LC_O_CORR_t * t_first = first;
	const LC_O_CORR_t * t_second = second;
	CORR_t first_c = t_first->c;
	CORR_t second_c = t_second->c;
	return COMP_ABS_CORR(second_c, first_c);
}

//Compare Function for descending order.
int descending_IC(const void *first, const void *second)
{
	const LC_I_CORR_t * t_first = first;
	const LC_I_CORR_t * t_second = second;
	CORR_t first_c = t_first->c;
	CORR_t second_c = t_second->c;
	return COMP_ABS_CORR(second_c, first_c);
}


//compute Linear Approximation Tables for the Corresponding S-Boxes 
int Compute_LAT(void)
{
	int * odd_even_hamming_table = NULL;
	int bigger_bit = PIPO64_SBOX_BIT_SIZE;
	int odd_even_hamming_table_size = 1 << bigger_bit;
	SUB_CNT_t mask_o;
	SUB_CNT_t mask_i;

	//initiating LAT
	for (mask_i = 0; mask_i < PIPO64_SBOX_CARDINALITY; mask_i++)
	{
		for (mask_o = 0; mask_o < PIPO64_SBOX_CARDINALITY; mask_o++)
		{
			LAT[mask_i][mask_o] = 0;
		}
	}


	//compute LAT
	odd_even_hamming_table = (int*)malloc(odd_even_hamming_table_size * sizeof(int));
	if (odd_even_hamming_table == NULL)
	{
		printf("Memory Allocation Error.\n");
		return 0;
	}

	{
		int masked_re;
		int shift;
		//init odd_even_hamming_table
		//odd_even_hamming_table has the entries -1 or 1 with repect to the corresponding mask result
		//when mask result has the odd  hamming weight -> -1  [note: when the mask result x&y has odd hamming weight, inner_product{x,y}=1.] 
		//when mask result has the even hamming weight -> 1   [note: when the mask result x&y has odd hamming weight, inner_product{x,y}=0.]
		for (masked_re = 0; masked_re < odd_even_hamming_table_size; masked_re++)
		{
			int hamming_weight = 0;
			for (shift = 0; shift < bigger_bit; shift++)
			{
				if (((masked_re >> shift) & 1) == 1)
					hamming_weight++;
			}
			odd_even_hamming_table[masked_re] = (hamming_weight & 1) == 1 ? -1 : 1;

		}
	}

#define INNER_PRODUCT(x,y) (odd_even_hamming_table[(x)&(y)])
	for (mask_o = 0; mask_o < PIPO64_SBOX_CARDINALITY; mask_o++)
	{
		//init [m \{dot} SBox(x)] table with a fixed outmask m.
		int m_dot_sbox_boolean_f[PIPO64_SBOX_CARDINALITY];
		SUB_CNT_t i_its;
		for (i_its = 0; i_its < PIPO64_SBOX_CARDINALITY; i_its++)
		{
			SUB_WRD_t in = (SUB_WRD_t)i_its;
			m_dot_sbox_boolean_f[in] = INNER_PRODUCT(mask_o, PIPO64_Sbox[in]);
		}

		//Walsh Transformation of The boolean function m\{dot}SBox(x)
		int size = PIPO64_SBOX_CARDINALITY;
		int i, i0, i1, step;
		int sum, diff;
		for (step = 1; step < size; step <<= 1)
		{
			for (i1 = 0; i1 < size; i1 += 2 * step)
			{
				for (i0 = 0; i0 < step; i0++)
				{
					i = i1 + i0;
					sum = m_dot_sbox_boolean_f[i] + m_dot_sbox_boolean_f[i + step];
					diff = m_dot_sbox_boolean_f[i] - m_dot_sbox_boolean_f[i + step];
					m_dot_sbox_boolean_f[i] = sum;
					m_dot_sbox_boolean_f[i + step] = diff;
				}
			}
		}
		//end Walsh with [a fixed outmask]

		for (mask_i = 0; mask_i < PIPO64_SBOX_CARDINALITY; mask_i++)
		{
			LAT[mask_i][mask_o] = m_dot_sbox_boolean_f[mask_i];
		}

	}
	//finish computing LAT
#undef INNER_PRODUCT

	MAX_CORR = ZERO_CORR;
	MIN_CORR = ONE_CORR;

	//compute CORR_LAT
	for (mask_i = 0; mask_i < PIPO64_SBOX_CARDINALITY; mask_i++)
	{
		for (mask_o = 0; mask_o < PIPO64_SBOX_CARDINALITY; mask_o++)
		{
			SUB_CNT_t this_LAT_ent = LAT[mask_i][mask_o];
			SUB_CNT_t abs_this_LAT_ent = abs(this_LAT_ent);
			if (abs_this_LAT_ent == 0)
			{
				CORR_LAT[mask_i][mask_o].sign = ZERO;
				CORR_LAT[mask_i][mask_o].magnitude = 1;
			}
			else //non-zero
			{
				//sign
				if (this_LAT_ent != abs_this_LAT_ent)
				{
					CORR_LAT[mask_i][mask_o].sign = NEGA;
				}
				else
				{
					CORR_LAT[mask_i][mask_o].sign = POSI;
				}


				if (abs_this_LAT_ent == PIPO64_SBOX_CARDINALITY)
				{
					CORR_LAT[mask_i][mask_o].magnitude = (ABS_CORR_t)0;
				}
				else
				{
					CORR_LAT[mask_i][mask_o].magnitude = (ABS_CORR_t)log2((ABS_CORR_t)abs_this_LAT_ent / (ABS_CORR_t)PIPO64_SBOX_CARDINALITY);
				}

				if (COMP_ABS_CORR(MIN_CORR, CORR_LAT[mask_i][mask_o]) == LEFT)
				{
					MIN_CORR = CORR_LAT[mask_i][mask_o];
				}
				
				if ((mask_i != 0) || (mask_o != 0))
				{
					if (COMP_ABS_CORR(MAX_CORR, CORR_LAT[mask_i][mask_o]) == RIGHT)
					{
						MAX_CORR = CORR_LAT[mask_i][mask_o];
					}
				}

			}
		}
	}

	free(odd_even_hamming_table);
	odd_even_hamming_table = NULL;
	return 0;
}


int Prep_LAT_Info(void)
{
	SUB_CNT_t mask_i;
	SUB_CNT_t mask_o;
	int idx;
	CORR_t the_corr;

	// LAT information rearrange
	// all I_O_CORR
	// O_CORR with fixed input
	// I_CORR with fixed output
	//rearrange
	for (mask_i = 0; mask_i < PIPO64_SBOX_CARDINALITY; mask_i++)
	{
		for (mask_o = 0; mask_o < PIPO64_SBOX_CARDINALITY; mask_o++)
		{
			//all I_O_CORR
			the_corr = CORR_LAT[mask_i][mask_o];
			LC_IOC[mask_i * PIPO64_SBOX_CARDINALITY + mask_o].i = (SUB_WRD_t)mask_i;
			LC_IOC[mask_i * PIPO64_SBOX_CARDINALITY + mask_o].o = (SUB_WRD_t)mask_o;
			LC_IOC[mask_i * PIPO64_SBOX_CARDINALITY + mask_o].c = the_corr;
			//O_CORR with fixed input
			LC_OC_FI[mask_i][mask_o].o = (SUB_WRD_t)mask_o;
			LC_OC_FI[mask_i][mask_o].c = the_corr;
			//I_CORR with fixed output
			LC_IC_FO[mask_o][mask_i].i = (SUB_WRD_t)mask_i;
			LC_IC_FO[mask_o][mask_i].c = the_corr;
		}
	}

	//sorting all I_O_CORR
	qsort((void*)LC_IOC, PIPO64_SBOX_CARDINALITY * PIPO64_SBOX_CARDINALITY, sizeof(LC_I_O_CORR_t), descending_IOC);

	//sorting O_CORR with fixed input
	for (mask_i = 0; mask_i < PIPO64_SBOX_CARDINALITY; mask_i++)
	{
		qsort((void*)LC_OC_FI[mask_i], PIPO64_SBOX_CARDINALITY, sizeof(LC_O_CORR_t), descending_OC);
	}
	//sorting I_CORR with fixed output
	for (mask_o = 0; mask_o < PIPO64_SBOX_CARDINALITY; mask_o++)
	{
		qsort((void*)LC_IC_FO[mask_o], PIPO64_SBOX_CARDINALITY, sizeof(LC_I_CORR_t), descending_IC);
	}

	//Counting the numbers of each information
	/*
	*  1) The number of (Input,Output) with NON-zero correlation.
	*  2) The number of (Input,Output) with the best correlation.
	*  3) The number of (Input)/(Output) with NON-zero correlation and each fixed (Output)/(Input).
	*  4) The number of (Input)/(Output) with the best correlation and each fixed (Output)/(Input).
	*/

	////init
	LC_NUM_IOC_NONZERO = 0;
	LC_NUM_IOC_MAX = 0;
	for (mask_i = 0; mask_i < PIPO64_SBOX_CARDINALITY; mask_i++)
	{
		LC_NUM_OC_FI_NONZERO[mask_i] = 0;
		LC_NUM_OC_FI_MAX[mask_i] = 0;
	}
	for (mask_o = 0; mask_o < PIPO64_SBOX_CARDINALITY; mask_o++)
	{
		LC_NUM_IC_FO_NONZERO[mask_o] = 0;
		LC_NUM_IC_FO_MAX[mask_o] = 0;
	}

	////Counting IO_CORR
	for (idx = 0; idx < PIPO64_SBOX_CARDINALITY * PIPO64_SBOX_CARDINALITY; idx++)
	{
		CORR_t IO_corr = LC_IOC[idx].c;
		CORR_t max_IO_corr = LC_IOC[0].c;
		if (COMP_ABS_CORR(IO_corr, ZERO_CORR) != EQUAL)
		{
			LC_NUM_IOC_NONZERO++;
		}
		if (COMP_ABS_CORR(max_IO_corr, IO_corr) == EQUAL)
		{
			LC_NUM_IOC_MAX++;
		}
	}

	////Counting O_CORR
	for (mask_i = 0; mask_i < PIPO64_SBOX_CARDINALITY; mask_i++)
	{
		for (idx = 0; idx < PIPO64_SBOX_CARDINALITY; idx++)
		{
			CORR_t O_corr = LC_OC_FI[mask_i][idx].c;
			CORR_t max_O_corr = LC_OC_FI[mask_i][0].c;
			if (COMP_ABS_CORR(O_corr, ZERO_CORR) != EQUAL)
			{
				LC_NUM_OC_FI_NONZERO[mask_i]++;
			}
			if (COMP_ABS_CORR(max_O_corr, O_corr) == EQUAL)
			{
				LC_NUM_OC_FI_MAX[mask_i]++;
			}
		}
	}

	////Counting I_CORR
	for (mask_o = 0; mask_o < PIPO64_SBOX_CARDINALITY; mask_o++)
	{
		for (idx = 0; idx < PIPO64_SBOX_CARDINALITY; idx++)
		{
			CORR_t I_corr = LC_IC_FO[mask_o][idx].c;
			CORR_t max_I_corr = LC_IC_FO[mask_o][0].c;
			if (COMP_ABS_CORR(I_corr, ZERO_CORR) != EQUAL)
			{
				LC_NUM_IC_FO_NONZERO[mask_o]++;
			}
			if (COMP_ABS_CORR(max_I_corr, I_corr) == EQUAL)
			{
				LC_NUM_IC_FO_MAX[mask_o]++;
			}
		}
	}

	return 0;
}

int Prep_Lin_Trail_Searching(void)
{
	Compute_LAT();
	Prep_LAT_Info();
	return 0;
}