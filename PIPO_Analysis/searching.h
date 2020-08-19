

#ifndef __SEARCHING_H__
#define __SEARCHING_H__

#include "pipo_spec.h"


int Prep_Dif_Trail_Searching(void);
int Prep_Lin_Trail_Searching(void);


/*in utils*/
enum time_flag { START_TIME = 1, END_TIME, ONLY_PRINT };
void Realtime_Print(int check_flag);





#define UNDER_BOUND		1
#define EXCEED_BOUND   -1


#define ROUND_START			-3
#define LAST_ROUND_START    -2
#define THE_LAST_WORD		-1

int Best_Trail_Prob_Only(int target_round);
int Best_Trail_Corr_Only(int target_round);

#endif /*__SEARCHING_H__*/



