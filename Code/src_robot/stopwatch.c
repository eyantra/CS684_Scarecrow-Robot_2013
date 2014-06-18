
/****************************************
 * 	stopwatch.c
 *
 *	<description>
 *
 *  Created on: Oct, 2013
 *      Author: dbs
 *     Project: S.R.
 ****************************************/


#include "stopwatch.h"


unsigned int getTickCount()
{
	struct timeval tv;

	if (gettimeofday(&tv, 0)!=0)
		return 0;
	return (tv.tv_sec*1000)+(tv.tv_usec/1000);
}
