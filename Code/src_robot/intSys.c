
/****************************************
 * 	intSys.c
 *
 *	<description>
 *
 *  Created on: Oct, 2013
 *      Author: dbs
 *     Project: S.R.
 ****************************************/


#include "intSys.h"

#include "../src_img_proc/blobTracker.h"
#include "robo.h"
#include "stopwatch.h"
#include "states.h"
#include "rules.h"
#include "pid.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cv.h>


extern IntSys is;

extern IplImage *scrImg;

extern CvCapture *capture;
extern CvSize fsize;

extern IplImage *frame;

extern BlobTracker *bbt;

BlobTracker *intruder;

extern char ruleDesc[256];
extern char *roboThought;

extern int recVideo;
extern CvVideoWriter *writer;

const char *sit[7] = {
						"roaming_greenhouse", "intruder_not_found",
						"searching_intruder", "intruder_found", "moving_to_intruder",
						"", ""
					 };


#define MOD(x) (((x)>0)?(x):(-(x)))

int emf_searchBlob(int key)
{
	static int status = 0;
	static int status2 = 0;
	static int change = 0;
	double tmp;
	static BlobTracker *bt;
	static PID tiltSrvPID = {0.05, 0.035, 0.001, 0.0, 0.0, 0.0};
	static PID panSrvPID = {0.02, 0.009, 0.001, 0.0, 0.0, 0.0};

	double bx, by;
	double errX, errY;
	double resp;
	static CvPoint center;

	static FILE *fp;

	unsigned int sw;

	if (!fp)
	{
		fp = fopen("hist_fb_7.data", "w");
		if (!fp)
			fprintf(stderr, "history file not opened!\n");
	}
	if (key == 0)
	{
		change = 0;
		status = 0;
		status2 = 0;
		if (findState(&is, "intruder_near")||findState(&is, "intruder_found"))
			;
		else
			bt = intruder;
	}
	trackBlobs(bt, frame, 0);
	if (bt->blobs->trackable)
	{
		if (status2 == 3)
		{
			//sendCommand(STOP, is.ltMotorVal=0, is.rtMotorVal=0);
			is.ltMotorVal=0, is.rtMotorVal=0;
			send_command(MOTOR_DIR, MOTOR_STOP);	//stop
			if (bt == intruder)
				addState(&is, "intruder_found");
			return 1;
		}
		if (status2 == 0)
		{
			clearPid(&tiltSrvPID);
			clearPid(&panSrvPID);
			status2 = 1;
			printf("tilt_clear: prev_resp=%g pan_prev_resp=%g.\n", tiltSrvPID.prevResp, panSrvPID.prevResp);
		}
		if (status2 == 1)
		{
			center = bt->blobs->center;
			bx = ((double)center.x/(double)fsize.width)*100.0;
			by = ((double)center.y/(double)fsize.height)*100.0;
			errY = by-50.0;
			errX = bx-50.0;
			printf("errorX %g, Y %g\n", errX, errY);
			if (errY<5.0f)
			{
				tmp = is.hdPanAngle;
				//writeServo(PAN_SRV, is.hdPanAngle=90.0f);
				send_command(SERVO_PAN, (byte)((int)(is.hdPanAngle=90.0f)));
				printf("waiting for 1sec...pan %g\n",tmp);
				sw = getTickCount();
				while ((getTickCount()-sw)<2000)
				{
					printf("time = %dms\n", getTickCount()-sw);
					frame = cvQueryFrame(capture);
				}
				printf("out of 1sec loop...\n");
				status2 = 3;
				if (MOD(tmp-90.0f)<10.0f)
				{
					printf("go straight\n");
					return 1;
				}
				if (tmp>90.0f)
				{
					printf("go right\n");
					is.ltMotorVal=is.rtMotorVal=40;
					send_command(RIGHT_MTR_VAL, 128+is.ltMotorVal/2);
					send_command(LEFT_MTR_VAL, 128-is.rtMotorVal/2);
					send_command(MOTOR_DIR, MOTOR_FWD);
					//sendCommand(LTMTR_SDIR, 1, 0);
					//sendCommand(DIFF_DRV, is.ltMotorVal=175, is.rtMotorVal=80);
				}
				else
				{
					printf("go left\n");
					is.ltMotorVal=is.rtMotorVal=40;
					send_command(LEFT_MTR_VAL, 128+is.rtMotorVal/2);
					send_command(RIGHT_MTR_VAL, 128-is.ltMotorVal/2);
					send_command(MOTOR_DIR, MOTOR_FWD);
					//sendCommand(RTMTR_SDIR, 1, 0);
					//sendCommand(DIFF_DRV, is.ltMotorVal=80, is.rtMotorVal=175);
				}
				return 1;
			}
			//resp = getPidRespVerb(&tiltSrvPID, errY, fp, "Tilt");
			resp = (double)errY*0.05f;
			is.hdTiltAngle -= resp;
			if (is.hdTiltAngle < MAX_SERVO_TILT_DOWN)
				is.hdTiltAngle = MAX_SERVO_TILT_DOWN;
			else
				if (is.hdTiltAngle > MAX_SERVO_TILT_UP)
					is.hdTiltAngle = MAX_SERVO_TILT_UP;
			if (is.hdPanAngle < MAX_SERVO_PAN_LEFT)
				is.hdPanAngle = MAX_SERVO_PAN_LEFT;
			else
				if (is.hdPanAngle > MAX_SERVO_PAN_RIGHT)
					is.hdPanAngle = MAX_SERVO_PAN_RIGHT;
			writePanTiltServos(is.hdPanAngle, is.hdTiltAngle);
			return 1;
		}
	}
	if (status2 == 3)
		return 1;
	if (status2)
		status2 = 0;
	if (!status)
	{
		status = 1;
		writePanTiltServos(is.hdPanAngle=150.0f/*30.0f*/, is.hdTiltAngle=90.0f);
		printf("waiting for 1sec...\n");
		sw = getTickCount();
		while ((getTickCount()-sw)<2000)
		{
			printf("time = %dms\n", getTickCount()-sw);
			frame = cvQueryFrame(capture);
		}
		printf("out of 1sec loop...\n");
		return 1;
	}
	if (!change)
	{
		if (is.hdPanAngle>160.0f||is.hdPanAngle<20.0f)
		{
			if (is.hdTiltAngle<40/*>140.0f*/)
			{
				writePanTiltServos(is.hdPanAngle=90.0f, is.hdTiltAngle=90.0f);
				cleanStates(&is);
				return 1;
			}
			//writeServo(TILT_SRV, is.hdTiltAngle-=20.0f);
			send_command(SERVO_TILT, (byte)((int)(is.hdTiltAngle-=20.0f)));
			status = -status;
			change = 1;
			return 1;
		}
	}
	if (change)
		change = 0;
	if (status==1)
		//writeServo(PAN_SRV, is.hdPanAngle-=5.0f);
		send_command(SERVO_PAN, (byte)((int)(is.hdPanAngle-=5.0f)));
	else
		//writeServo(PAN_SRV, is.hdPanAngle+=5.0f);
		send_command(SERVO_PAN, (byte)((int)(is.hdPanAngle+=5.0f)));
	return 1;
}

int emf_movetoBlob(int key)
{
	static int status=0;

	unsigned int sw;

	int blobfound;
	double bx, by;
	double error;
	double resp;
	CvPoint center;

	static double maxTiltAngle;
	static BlobTracker *bt;
	static PID driveMtrPID = {5.0, 0.1, 0.01, 0.0, 0.0, 0.0};
	//static PID driveMtrPID = {1.0, 0.1, 0.05, 0.0, 0.0, 0.0};
//	static PID driveMtrPID = {3.0, 2.0, 1.0, 0.0, 0.0, 0.0};
	static PID tiltSrvPID = {0.05, 0.035, 0.001, 0.0, 0.0, 0.0};

	static FILE *fp;


	if (key == 0)
	{
		status = 0;
		clearPid(&driveMtrPID);
		clearPid(&tiltSrvPID);
		maxTiltAngle = /*145.0f*/35;
		bt = intruder;
	}
	if (!fp)
	{
		fp = fopen("hist8.data", "w");
		if (!fp)
			fprintf(stderr, "MoveToBlob history file not opened!\n");
	}
	trackBlobs(bt, frame, 0);
	blobfound = bt->blobs->trackable;
	center = bt->blobs->center;
	if (!blobfound)
	{
		removeState(&is, "intruder_found");
		removeState(&is, "gp_found");
		//sendCommand(STOP, is.ltMotorVal=0, is.rtMotorVal=0);
		is.ltMotorVal=0, is.rtMotorVal=0;
		send_command(MOTOR_DIR, MOTOR_STOP);

	}
	if (status==1&&blobfound)
	{
		if (bt == intruder)
		{
			removeState(&is, "intruder_found");
			addState(&is, "intruder_near");
		}
		return 1;
	}
	if (is.hdTiltAngle<45.0f/*145.0f*/)
	{
		//sendCommand(STOP, is.ltMotorVal=0, is.rtMotorVal=0);
		is.ltMotorVal=0, is.rtMotorVal=0;
		send_command(MOTOR_DIR, MOTOR_STOP);
		fprintf(fp, "Got intruder \n");
		printf("waiting for 2sec to stabilize intruder...\n");
		sw = getTickCount();
		while ((getTickCount()-sw)<2000)
		{
			printf("time = %dms\n", getTickCount()-sw);
			frame = cvQueryFrame(capture);
		}
		status = 1;
		return 1;
	}
	else
	{
		if (blobfound)
		{
			bx = ((double)center.x/(double)fsize.width)*100.0;
		    by = ((double)center.y/(double)fsize.height)*100.0;

		    printf("pos bx=%g, by=%g\n", bx, by);
		    fprintf(fp, "pos bx=%g, by=%g\n", bx, by);

		    error = bx-50.0;
		    //resp = getPidRespVerb(&driveMtrPID, error, fp, "DriveMtr");
resp = (double)error*1.0;
		    if (resp>120.0)
		    	resp = 120.0;
		    else
		    	if (resp<-120.0)
		        	resp = -120.0;
		    if (resp > 0)
		    {
		        is.ltMotorVal = /*120*/240;
		        is.rtMotorVal = /*120-(int)resp*/0;
		    }
		    else
		    {
		    	is.ltMotorVal = /*120+(int)resp*/0;
		    	is.rtMotorVal = 240;
		    }

		    error = by-50.0;
//		    resp = getPidRespVerb(&tiltSrvPID, error, fp, "TiltSrv");
resp = (double)error*0.05f;
			is.hdTiltAngle -= resp;
		    if (is.hdTiltAngle < MAX_SERVO_TILT_DOWN)
		    	is.hdTiltAngle = MAX_SERVO_TILT_DOWN;
		    else
		    	if (is.hdTiltAngle > MAX_SERVO_TILT_UP)
		    		is.hdTiltAngle = MAX_SERVO_TILT_UP;
		    //writeServo(TILT_SRV, is.hdTiltAngle);
		    send_command(SERVO_TILT, (byte)((int)is.hdTiltAngle));
		    //sendCommand(DIFF_DRV, is.ltMotorVal, is.rtMotorVal);
		    send_command(LEFT_MTR_VAL, 128-is.ltMotorVal/2);
		    send_command(RIGHT_MTR_VAL, 128-is.rtMotorVal/2);
		    send_command(MOTOR_DIR, MOTOR_FWD);
		    fprintf(fp, "CMD: x=%d y=%d pan=%g, tilt=%g lmotor=%d rmotor=%d\n\n",
		    		center.x, center.y, is.hdPanAngle, is.hdTiltAngle,
		    		128-is.ltMotorVal/2, 128-is.rtMotorVal/2);
		}
	}
	return 1;
}

int emf_shootIntruder(int key)
{
	static int status = 0;
	unsigned int sw;
	static BlobTracker *bt;

	int blobfound;
	double bx, by;
	double error;
	double resp;
	CvPoint center;


	if (key == 0)
	{
		status = 0;
		bt = intruder;
		is.hdPanAngle = 90;
		send_command(SERVO_PAN, (byte)((int)is.hdPanAngle));
	}
	if (status == 0)
	{
		/*sendCommand(LTMTR_SDIR, 1, 0);
		sendCommand(RTMTR_SDIR, 1, 0);
		sendCommand(DIFF_DRV, is.ltMotorVal=150, is.rtMotorVal=165);*/
		int i;

		for (i=0; i<5; ++i)
		{
			while (1)
			{
				frame = cvQueryFrame(capture);
				trackBlobs(bt, frame, 0);
				blobfound = bt->blobs->trackable;
				center = bt->blobs->center;
				if (blobfound)
				{
					bx = ((double)center.x/(double)fsize.width)*100.0;
		    		by = ((double)center.y/(double)fsize.height)*100.0;

		    		printf("pos bx=%g, by=%g\n", bx, by);

		    		error = bx-50.0;
					printf("SHOOT PAN ERR= %g.\n", error);
					if ((error>0&&error<10.0) || (error<0&&error>-10.0))
						break;
		    	//resp = getPidRespVerb(&driveMtrPID, error, fp, "DriveMtr");
					resp = (double)error*0.2;
					is.hdPanAngle += resp;
					if (is.hdPanAngle < MAX_SERVO_PAN_LEFT)
						is.hdPanAngle = MAX_SERVO_PAN_LEFT;
					else
						if (is.hdPanAngle > MAX_SERVO_PAN_RIGHT)
							is.hdPanAngle = MAX_SERVO_PAN_RIGHT;
					send_command(SERVO_PAN, (byte)((int)is.hdPanAngle));
				}
				else
				{
					status = 1;
					return 1;
				}
			}
			printf("in laser_shoot: %d.\n", i);
			send_command(LASER_CMD, LASER_ON);
			//sleep(2);
			sw = getTickCount();
			while ((getTickCount()-sw)<2000)
				frame = cvQueryFrame(capture);
			send_command(LASER_CMD, LASER_OFF);
			sw = getTickCount();
			while ((getTickCount()-sw)<2000)
				frame = cvQueryFrame(capture);
			trackBlobs(bt, frame, 0);
			if (!bt->blobs->trackable)
				break;
		}
		if (bt->blobs->trackable)
			status = 2;
		else
			status = 1;
	}
	if (status == 1)
	{
		sw = getTickCount();
		while ((getTickCount()-sw)<5000)
		{
			frame = cvQueryFrame(capture);
		}
		//sendCommand(BUZ_TOGGLE, 0, 0);
		//++status;
		removeState(&is, "intruder_near");
		printf("Starting again.\n");
	}
	if (status == 2)
	{
		int status = 0;

		//sendCommand(BUZ_TOGGLE, 0, 0);
		send_command(ROBOT_STATUS, SET_LED(status, 3));
		/*sw = getTickCount();
		while ((getTickCount()-sw)<1000)
		{
			frame = cvQueryFrame(capture);
		}
		//sendCommand(BUZ_TOGGLE, 0, 0);
		//++status;*/
		removeState(&is, "intruder_near");
		//removeState(&is, "gp_near");
		addState(&is, "done_shooting");
		printf("Finished\n");
	}
	return 1;
}

int emf_finish(int key)
{
	return 1;
}

int initIntSys()
{
	char str[128];

	is.fsize=fsize;
	is.hdPanAngle = is.hdTiltAngle = 90.0f;
	is.ltMotorVal = is.rtMotorVal = 0;
	//is.rule = 0;
	is.sit = 0;
	intruder = getBlobTracker(bbt, 1);
	
	if (!(addState(&is, "roaming_greenhouse")))
	{
		cleanStates(&is);
		return 0;
	}
	writePanTiltServos(is.hdPanAngle, is.hdTiltAngle);
	INIT_SW();
	printStates(&is, str);
	printf("%s\n", str);
	return 1;
}



#define NUM_EMF 4
extern char *emfName[NUM_EMF];



void *emf[NUM_EMF] = {
						emf_searchBlob,
						emf_movetoBlob,
						emf_shootIntruder,
						emf_finish
};

int intSysMain()
{
	Rule *rule;
	char str[128];
	int key;
	int i;

	if (is.sitChange)
		key = 0;
	else
		key = 1;
	if (is.sitChange)
	{
		is.sitChange = 0;
		rule = findRule(&is);
		printf("%s\n", printStates(&is, str));
		if (!rule)
		{
			is.emf = 0;
			printf("No rule found\n");
			strcpy(ruleDesc, "Suitable response rule not found!");
			roboThought = "Damn it! I don't know what to do!!!";
			return 1;
		}
		else
		{
			printf("Response - %d %s\n", rule->respid, emfName[rule->respid]);
		}
		is.emf = emf[rule->respid];
		*ruleDesc = 0;
		for (i=0; i<rule->numsit; ++i)
		{
			strcat(ruleDesc, rule->sit[i]);
			if (i < rule->numsit-1)
				strcat(ruleDesc, ", ");
		}
		strcat(ruleDesc, " --> ");
		strcat(ruleDesc, emfName[rule->respid]);
		roboThought = rule->respdesc;
	}
	if (is.emf)
		is.emf(key);

	return 1;
}
