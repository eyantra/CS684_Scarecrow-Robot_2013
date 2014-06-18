/*
 * main.c
 *
 *  Created on: 19-Oct-2013
 *      Author: dbs
 */


#include "robo.h"

#include <stdio.h>

#include <cv.h>
#include <highgui.h>

#include "intSys.h"

#include "../src_img_proc/blobTracker.h"

#include "rules.h"
#include "states.h"

#include "pid.h"

#include "stopwatch.h"

IntSys is;

int mode = 0;
int action = 0;
int prevAction;

char msgBuf[256];

CvCapture *capture = 0;
IplImage *scrImg = 0;
IplImage* frame = 0;
CvSize fsize;

BlobTracker *bbt;

int testAction = 0;
int selectObj = 0;
CvRect selection1;

char ruleDesc[256];
char *roboThought = 0;

int recVideo = 0;
CvVideoWriter *writer = 0;

int ptt = 0;

int bkPrjBall=0, bkPrjGp=0;

dword fCount = 0;

void panTiltTrack()
{
	static BlobTracker *bt = 0;

	CvPoint center;

	double bx, by;
	double errX, errY;
	double resp;

	static PID tiltSrvPID = {0.2, 0.001, 0.0001, 0.0, 0.0, 0.0};
	static PID panSrvPID = {0.2, 0.001, 0.0001, 0.0, 0.0, 0.0};

	if (ptt==1)
	{
		writePanTiltServos(is.hdPanAngle=90.0f, is.hdTiltAngle=90.0f);
		bt = getBlobTracker(bbt, 1);
		++ptt;
		clearPid(&tiltSrvPID);
		clearPid(&panSrvPID);
	}
	trackBlobs(bt, frame, 0);
	if (!bt->blobs->trackable)
	{
		fprintf(stderr, "Blob not found!\n");
		return;
	}
	center = bt->blobs->center;
	bx = ((double)center.x/(double)fsize.width)*100.0;
	by = ((double)center.y/(double)fsize.height)*100.0;
	errX = bx-50.0;
	errY = by-50.0;
	//resp = getPidResp(&tiltSrvPID, errY);
	resp = (double)errY*0.2f;
	is.hdTiltAngle -= resp;
	printf("tilt resp %g ", resp);
	//resp = getPidResp(&panSrvPID, errX);
	resp = (double)errX*0.2f;
	printf("pan resp %g\n", resp);
	is.hdPanAngle += resp;
	if (is.hdPanAngle < MAX_SERVO_PAN_LEFT)
		is.hdPanAngle = MAX_SERVO_PAN_LEFT;
	else
		if (is.hdPanAngle > MAX_SERVO_PAN_RIGHT)
			is.hdPanAngle = MAX_SERVO_PAN_RIGHT;
	if (is.hdTiltAngle < MAX_SERVO_TILT_DOWN)
		is.hdTiltAngle = MAX_SERVO_TILT_DOWN;
	else
		if (is.hdTiltAngle > MAX_SERVO_TILT_UP)
			is.hdTiltAngle = MAX_SERVO_TILT_UP;

	writePanTiltServos(is.hdPanAngle, is.hdTiltAngle);
	printf("PanTilt: errX=%g, errY=%g, pan=%g, tilt=%g\n",
		   errX, errY, is.hdPanAngle, is.hdTiltAngle);
}

extern IplImage *backproject;

int doProc()
{
	char str[256];
	int i;
	static unsigned int sw = 0;
	unsigned int dt, dtActual;

	if (recVideo && !writer)
		writer=cvCreateVideoWriter("robo_vid8.avi", CV_FOURCC('P','I','M','1'),
				                   25, cvSize(640,480), 1);
	i = 0;
	do
	{
		++i;
		dtActual = getTickCount();
		frame = cvQueryFrame(capture);
		dtActual = getTickCount()-dtActual;
	}
	while ((getTickCount()-sw)<120);
	dt = getTickCount()-sw;
	sw = getTickCount();
	if (action==1||action==2)
		++fCount;
	sprintf(str, "FPS: %2.2f  FrameTime: %4dms  CameraFPS: %2.2f  CameraFrameTime: %4dms  FSC: %d  PFC: %d",
			1000.0f/((float)dt), dt, 1000.0f/((float)dtActual), dtActual, i, fCount);
	printf("%s\n", str);
	//gtk_label_set_text(GTK_LABEL(lblFps), str);
	if (!frame)
		return 0;
	if (action == 1)
		intSysMain();
	else
		if (action == 2)
			panTiltTrack();
	if (recVideo && writer)
		cvWriteFrame(writer, frame);

	/*cvCvtColor(frame, scrImg, CV_BGR2RGB);
	GdkPixbuf* pix = gdk_pixbuf_new_from_data((guchar*)scrImg->imageData,
		            GDK_COLORSPACE_RGB, FALSE, scrImg->depth, scrImg->width,
		            scrImg->height, (scrImg->widthStep), NULL, NULL);
	gdk_draw_pixbuf(screen->window,
					screen->style->fg_gc[GTK_WIDGET_STATE(screen)], pix, 0, 0, 0, 0,
		            scrImg->width, scrImg->height, GDK_RGB_DITHER_NONE, 0, 0);*/
	cvShowImage("ROBOT VIEW", frame);
	if (backproject)
	{
	//	cvShowImage("BACKPROJECT", backproject);
	}
	sprintf(str, "Left Motor:   %d,   Right Motor:   %d,   Head Servo Pan Angle:   %g,   Tilt Angle:   %g",
		   is.ltMotorVal, is.rtMotorVal, is.hdPanAngle, is.hdTiltAngle);
	printf("%s\n", str);
	//gtk_label_set_text(GTK_LABEL(lblRobo), str);
	strcpy(str, "Current Situation:   ");
	printStates(&is, &str[21]);
	//gtk_label_set_text(GTK_LABEL(lblSit), str);
	printf("%s\n", str);
	strcpy(str, "Current Rule:   ");
	strcat(str, ruleDesc);
	//gtk_label_set_text(GTK_LABEL(lblRule), str);
	printf("%s\n", str);
	if (roboThought)
	{
		strcpy(str, "\"");
		strcat(str, roboThought);
		strcat(str, "\"");
	}
	else
		*str = 0;
	//gtk_label_set_text(GTK_LABEL(lblThought), str);
	printf("%s\n", str);
	cvWaitKey(1);
	return 1;
}


int main(int argc, char** argv)
{
	char msg[256];
	int error = 0;
	char *serial_device = "/dev/ttyUSB0";
	int camera_id = 0;

	if (argc == 3)
	{
		camera_id = atoi(argv[1]);
		serial_device = argv[2];
	}
	*msg = 0;
	if (!(capture=cvCaptureFromCAM(camera_id)))
	{
		strcat(msg, "Unable to initiate capture.\n");
		++error;
	}
	else
	{
		frame = cvQueryFrame(capture);
		fsize = cvGetSize(frame);
		if (frame)
			scrImg = cvCreateImage(fsize, 8, 3);
		initBlobTrackers(fsize);
	}
	if (!loadBlobTrackers(&bbt, "bt.bt"))
	{
		strcat(msg, "Unable to load blob trackers.\n");
		++error;
	}
	if (!initRobo(serial_device))
	{
		strcat(msg, msgBuf);
		++error;
	}
	//sendCommand(PING, 0, 0);
	is.rule = 0;
	if (!loadRules(&is, "rule.rl"))
	{
		strcat(msg, "Loading rules failed.\n");
		++error;
	}
	if (error)
		printf("%s\n", msg);

	INIT_SW();

	byte robot_status = 0;
	int stat_check_count=0;

	send_command(ROBOT_STATUS, SET_LED(robot_status, 0));

	while (1)
	{
		while (1)
		{
			if (action)
				doProc();
			if (++stat_check_count > 50)
			{
				if (action==0)
					sleep(2);
				stat_check_count = 0;
				robot_status &= 0xf0;
				TOGGLE_LED(robot_status, 0);
				get_robot_status(&robot_status);
				//send_command(ROBOT_STATUS, TOGGLE_LED(robot_status, 0));
				/*if (IS_SWITCH_PRESSED(robot_status, 0))
				{
					printf("sw0 pressed.\n");
					send_command(MOTOR_DIR, MOTOR_STOP);
					action = 0;
				}
				else*/ if (IS_SWITCH_PRESSED(robot_status, 2))
				{
					printf("sw2 pressed.\n");
					send_command(MOTOR_DIR, MOTOR_STOP);
					action = 2;
					ptt = 1;
				}
				else if (IS_SWITCH_PRESSED(robot_status, 3))
				{
					printf("sw3 pressed.\n");
					send_command(MOTOR_DIR, MOTOR_STOP);
					action = 1;
					if (!initIntSys())
					{
						fprintf(stderr, "IntSys Error");
						return -1;
					}
					fCount = 0;
				}
			}
		}
	}
    return 0;
}



