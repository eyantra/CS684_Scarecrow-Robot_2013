
/****************************************
 * 	pid.c
 *
 *	<description>
 *
 *  Created on: Oct, 2013
 *      Author: dbs
 *     Project: S.R.
 ****************************************/


#include "pid.h"
#include <stdio.h>

double getPidRespVerb(PID *pid, double error, FILE *fp, char *pidName)
{
	/*register */double prevErr1 = pid->prevErr1;
	/*register */double resp;

	resp = pid->prevResp +
		   (pid->kp)*(error-prevErr1) +
		   (pid->ki)*(error+prevErr1)/2 +
		   (pid->kd)*(error-2*prevErr1+(pid->prevErr2));
	pid->prevResp = resp;
	pid->prevErr2 = prevErr1;
	pid->prevErr1 = error;
	fprintf(fp, "%s PID Err=%g, P=%g, I=%g, D=%g, Resp=%g\n", pidName,
			error, (error-prevErr1), (error+prevErr1)/2,
			(error-2*prevErr1+pid->prevErr2), resp);
	return resp;
}

double getPidResp(PID *pid, double error)
{
	/*register */double prevErr1 = pid->prevErr1;
	/*register */double resp;

	resp = pid->prevResp +
		   (pid->kp)*(error-prevErr1) +
		   (pid->ki)*(error+prevErr1)/2 +
		   (pid->kd)*(error-2*prevErr1+(pid->prevErr2));
	pid->prevResp = resp;
	pid->prevErr2 = prevErr1;
	pid->prevErr1 = error;
	return resp;
}

void clearPid(PID *pid)
{
	pid->prevErr1=pid->prevErr2=0.0f;
	pid->prevResp = 0.0f;
}
