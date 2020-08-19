#include "searching.h"
#include <stdio.h>


extern int PIPO64_OFFSET[8];


#ifdef _MSC_VER

#include <Windows.h>

char* optarg = NULL;
int optind = 1;

int getopt(int argc, char *const argv[], const char *optstring)
{
	if ((optind >= argc) || (argv[optind][0] != '-') || (argv[optind][0] == 0))
	{
		return -1;
	}

	int opt = argv[optind][1];
	const char *p = strchr(optstring, opt);

	if (p == NULL)
	{
		return '?';
	}
	if (p[1] == ':')
	{
		optind++;
		if (optind >= argc)
		{
			return '?';
		}
		optarg = argv[optind];
		optind++;
	}
	return opt;
}
#elif __GNUC__
#include <unistd.h>
#endif




void help(void)
{
	printf("=================[OPTION]=========\n");
	printf("  -o [RotationOffset]  : Each Rotation Offset\n");
	printf("  -c [D/L]             : Choose DC('D'), LC('L')\n");
	printf("  -r [Round]           : Round\n");
	printf("==================================\n");
#ifdef WIN32
	printf("  Example(Windows)	   : .\\PIPO_Analysis.exe -o 07436512 -c D -r 8\n");
	printf("  Example(Windows)	   : .\\PIPO_Analysis.exe -o 07436512 -c L -r 8\n");
#elif __linux__
	printf("  Example(Linux)	   : ./PIPO_Analysis.out -o 07436512 -c D -r 8\n");
	printf("  Example(Linux)	   : ./PIPO_Analysis.out -o 07436512 -c L -r 8\n");
#endif
	exit(0);
}



typedef struct
{
	int menu_choose;
}ANALYSIS_PARAM_t;

int opt_check_flag[3] = { -1, -1, -1 };
void init_analysis_param(int * numround, ANALYSIS_PARAM_t * param, int argc, char ** argv)
{

	//setting the information from arg vector
	int opt;
	char rotoffset[512];
	size_t rotoffset_len;
#define DC 1
#define LC 0
	int set_dif_o_flag = TRUE;
	int offset_idx;
	int get_round;
	int iii;
	while ((opt = getopt(argc, argv, "o:c:r:")) != -1)
	{
		switch (opt)
		{
		case 'o':
			opt_check_flag[0] = 1;
			memcpy(rotoffset, optarg, strlen(optarg));
			rotoffset_len = strlen(optarg);
			if (rotoffset_len != 8)
			{
				return;
			}

			for (offset_idx = 0; offset_idx < 8; offset_idx++)
			{
				char temp[2] = { 0, };
				temp[0] = rotoffset[offset_idx];
				PIPO64_OFFSET[offset_idx] = atoi(temp) % 8;
			}
			break;
		case 'c':
			opt_check_flag[1] = 1;
			if (optarg[0] == 'D')
			{
				param->menu_choose = DC;
			}
			else if (optarg[0] == 'L')
			{
				param->menu_choose = LC;
			}
			break;
		case 'r':
			get_round = atoi(optarg);
			if ((get_round <= 0) || get_round > 15)
			{
				printf("Max num of rounds is 15 for PIPO64-256\n");
				get_round = 15;
			}
			opt_check_flag[2] = 1;
			*numround = get_round;
			break;
		default:
			break;
		}
	}

	for (iii = 0; iii < 3; iii++)
	{
		if (opt_check_flag[iii] == -1)
		{
			printf("!!!!!Check Input Options!!!!!\n");
			help();
			exit(0);
		}
	}
}

int main(int argc, char ** argv)
{
	ANALYSIS_PARAM_t analysis;
	int TARGET_ROUND;
	printf("Searching Trails for PIPO\n");

	if (argc > 1)
	{
		int row_idx;
		init_analysis_param(&TARGET_ROUND, &analysis, argc, argv);
		for (row_idx = 0; row_idx < 8; row_idx++)
		{
			if (PIPO64_OFFSET[row_idx] == -1)
			{
				printf("Error! : Rotation offsets are not set correctly\n");
				help();
				return -1;
			}
		}

		printf("Rotation Offsets : ");
		for (row_idx = 0; row_idx < 8; row_idx++)
		{
			printf("%d ", PIPO64_OFFSET[row_idx]);
		}
		printf("\n");

		switch (analysis.menu_choose)
		{
		case DC:
			Prep_Dif_Trail_Searching();
			Best_Trail_Prob_Only(TARGET_ROUND);
			break;
		case LC:
			Prep_Lin_Trail_Searching();
			Best_Trail_Corr_Only(TARGET_ROUND);
			break;
		default:
			printf("Error! : Unknown Menu\n");
			help();
			return -1;
			break;
		}
	}
	else
	{
		help();
	}





	return 0;
}