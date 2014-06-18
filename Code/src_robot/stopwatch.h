
/****************************************
 * 	stopwatch.h
 *
 *	<description>
 *
 *  Created on: Oct, 2013
 *      Author: dbs
 *     Project: S.R.
 ****************************************/


#ifndef STOPWATCH_H_
#define STOPWATCH_H_

//#include "time.h"
#include <sys/time.h>


#define INIT_SW() clock()
#define NEW_SW(sw) clock_t sw=0
#define START_SW(sw) sw=clock()
#define STOP_SW(sw) ((sw)=clock()-(sw))
#define GET_SW_TIME(sw) ((float)((sw)*(1000.0f/(float)CLOCKS_PER_SEC)))
#define GET_SW_RATE(sw) ((float)((float)CLOCKS_PER_SEC/(sw)))

#define SW_SLEEP_LOOP(sw, tm) while(((float)(clock()-(sw))/((float)CLOCKS_PER_SEC))<(tm))


unsigned int getTickCount();

#endif /* STOPWATCH_H_ */
