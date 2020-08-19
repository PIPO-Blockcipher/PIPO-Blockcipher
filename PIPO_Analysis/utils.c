#include <stdio.h>
#include <time.h>
#include "searching.h"

time_t start_time;

void Realtime_Print(int check_flag)
{
	time_t cur_time;
	time_t duration;
	struct tm * t;

	cur_time = time(NULL);

	t = localtime(&cur_time);

	switch (check_flag)
	{
	case START_TIME:
		start_time = cur_time;
		printf("%02d-%02d-%02d %02d:%02d:%02d [Set_WallClock]-> ",
			(t->tm_year + 1900) % 100, t->tm_mon + 1, t->tm_mday,
			t->tm_hour, t->tm_min, t->tm_sec);
		break;
	case END_TIME:
		duration = cur_time - start_time;
		printf("%02d-%02d-%02d %02d:%02d:%02d [%8llu secs]-> ",
			(t->tm_year + 1900) % 100, t->tm_mon + 1, t->tm_mday,
			t->tm_hour, t->tm_min, t->tm_sec, duration);
		break;
	default:
		printf("%02d-%02d-%02d %02d:%02d:%02d                -> ",
			(t->tm_year + 1900) % 100, t->tm_mon + 1, t->tm_mday,
			t->tm_hour, t->tm_min, t->tm_sec);
		break;
	}
}