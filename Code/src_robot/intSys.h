
/****************************************
 * 	intSys.h
 *
 *	<description>
 *
 *  Created on: Oct, 2013
 *      Author: dbs
 *     Project: S.R.
 ****************************************/


#ifndef INTSYS_H_
#define INTSYS_H_

#include "datatypes.h"

#include <cv.h>
#include <highgui.h>



typedef struct IntSys
{
	struct State *sit;
	struct Rule *rule;

	int sitChange;
	int (*emf)(int key);

	struct BlobTracker *bt;

	int camid;
	CvCapture *capture;
	IplImage *frame;
	CvSize fsize;

	float hdPanAngle, hdTiltAngle;
	byte ltMotorVal, rtMotorVal;

	void *res;
} IntSys;

typedef IntSys IS;

int initIntSys();
int intSysMain();

#endif /* INTSYS_H_ */
