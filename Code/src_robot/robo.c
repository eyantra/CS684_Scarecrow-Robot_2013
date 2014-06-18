
/****************************************
 * 	robo.c
 *
 *	<description>
 *
 *  Created on: Oct, 2013
 *      Author: dbs
 *     Project: S.R.
 ****************************************/


#include "robo.h"

#include "common.h"

#include <stdio.h>		// Standard input/output definitions
#include <string.h>		// String function definitions
#include <unistd.h>		// UNIX standard function definitions
#include <fcntl.h>		// File control definitions
#include <errno.h>		// Error number definitions
#include <termios.h>	// POSIX terminal control definitions


// Global variables
int fd = -1;	// File descriptor for the port
byte buf[16];	// communication buffer

int initRobo(char *serialdev)
{
	struct termios options;

	if (fd >= 0)
	{
		fprintf(stderr, "Previous fd closed!\n");
		close(fd);
	}
	fd = open(serialdev, O_RDWR|O_NOCTTY|O_NDELAY);
	if (fd < 0)
	{
		sprintf(msgBuf, "Unable to open port '%s': %s.\n", serialdev, strerror(errno));
		return 0;
	}

	fcntl(fd, F_SETFL, FNDELAY);			// Configure port reading
	tcgetattr(fd, &options);				// Get the current options for the port
	cfsetispeed(&options, B9600);			// Set the baud rates to 9600
	cfsetospeed(&options, B9600);

	options.c_cflag |= (CLOCAL|CREAD);		// Enable the receiver and set local mode
	options.c_cflag &= ~PARENB;				// Mask the character size to 8 bits, no parity
	options.c_cflag &= ~CSTOPB;
	options.c_cflag &= ~CSIZE;
	options.c_cflag |=  CS8;				// Select 8 data bits
	options.c_cflag &= ~CRTSCTS;			// Disable hardware flow control
	options.c_lflag &= ~(ICANON|ECHO|ISIG);	// Enable data to be processed as raw input

	tcsetattr(fd, TCSANOW, &options);		// Set the new options for the port

	tcflush(fd, TCIOFLUSH);					// Flush all buffers

	return 1;
}


int send_command(byte cmd, byte val)
{
	int reply = 0;
	int n = 1;

	if (fd < 0)
	{
		fprintf(stderr, ">> Sending command failed (cmd=%d, val=%d). "
				"Port not initialized.\n", cmd, val);
		return -1;
	}
	buf[0] = 'N';
	buf[1] = 'E';
	buf[2] = 'X';
	buf[3] = cmd;
	buf[4] = val;

	/*if ((reply=(cmd>=REPLY_CMDS)))
		tcflush(fd, TCIFLUSH);*/
	if (write(fd, buf, 5) < n)
	{
		fprintf(stderr, ">> Sending command failed (cmd=%d, val=%d)\n",
				(int)cmd, (int)val);
		return -1;
	}
	printf("Command sent cmd=%d val=%d.\n", (int)cmd, (int)val);
	
	return 1;
}


int read_reply(int data_size, byte *buffer)
{
	return read(fd, buffer, data_size);
}

int get_robot_status(byte *status)
{
	int n;

	//tcflush(fd, TCIFLUSH);
	send_command(0x0b, *status);
	n = read(fd, buf, 4);
	if (n != 4)
	{
		*status = *status&0xf0;
		printf("Robot didn't respond properly to status check.\n");
	}
	else
		*status = buf[3];
	//printf("Robot status %d.\n", *status);
	return n;
}

int writePanTiltServos(float panAngle, float tiltAngle)
{
	word pWidth;
	int pan_angle, tilt_angle;

	if (fd < 0)
	{
		fprintf(stderr, ">> Sending pan-tilt servo command failed (panAngle=%g, tiltAngle=%g). "
						"Port not initialized.\n", panAngle, tiltAngle);
		return -1;
	}

	if (panAngle < 0.0f)
		panAngle = 0.0f;
	else
		if (panAngle > 180.0f)
			panAngle = 180.0f;
	if (tiltAngle < 0.0f)
		tiltAngle = 0.0f;
	else
		if (tiltAngle > 180.0f)
			tiltAngle = 180.0f;
	pan_angle = (int)panAngle;
	tilt_angle = (int)tiltAngle;

	send_command(6, (byte)pan_angle);
	send_command(8, (byte)tilt_angle);

	return 1;
}
/*
float getRoboBatteryInfo()
{
	int volt;

	if ((volt=sendCommand(BATT_ENQ, 0, 0)) < 0)
		return -1;

	return (((volt/255.0f)*5.0f)*(1+47.0f/22.0f));
}*/

int closeRobo()
{
	//close(fd);
}

