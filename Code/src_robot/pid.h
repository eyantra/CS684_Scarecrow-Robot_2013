
/****************************************
 * 	pid.h
 *
 *	<description>
 *
 *  Created on: Oct, 2013
 *      Author: dbs
 *     Project: S.R.
 ****************************************/


#ifndef PID_H_
#define PID_H_

#include <stdio.h>

typedef struct
{
	double kp, ki, kd;
	double prevErr1, prevErr2;
	double prevResp;
} PID;


// Functions
// Verbose pid function for debugging
double getPidRespVerb(PID *pid, double error, FILE *fp, char *pidName);
// Fast pid function
double getPidResp(PID *pid, double error);
void clearPid(PID *pid);


#endif /* PID_H_ */
