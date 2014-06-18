
/****************************************
 * 	robo.h
 *
 *	<description>
 *
 *  Created on: Oct, 2013
 *      Author: dbs
 *     Project: S.R.
 ****************************************/


#ifndef ROBO_H_
#define ROBO_H_

#include "datatypes.h"


// Commands

#define MOTOR_DIR		0x94
#define MOTOR_FWD		0x01
#define MOTOR_BWD		0x02
#define MOTOR_LEFT		0x03
#define MOTOR_RIGHT		0x04
#define MOTOR_STOP		0x06

#define LEFT_MTR_VAL	0x95
#define RIGHT_MTR_VAL	0x96

#define SERVO_PAN		0x06
#define SERVO_TILT		0x08

#define MAX_SERVO_PAN_LEFT		5
#define MAX_SERVO_PAN_RIGHT		175
#define MAX_SERVO_TILT_DOWN		5
#define MAX_SERVO_TILT_UP		110

#define LASER_CMD				0xc
#define LASER_ON				0
#define LASER_OFF				1

#define ROBOT_STATUS	0x0b

#define IS_SWITCH_PRESSED(status, switch_id)	((status)&(0x01<<(switch_id)))

#define SET_LED(status, led_id)		((status)=((status)|(0x10<<(led_id))))
#define TOGGLE_LED(status, led_id)	((status)=((status)^(0x10<<(led_id))))
#define CLEAR_LED(status, led_id)	((status)=((status)&(~(0x10<<(led_id)))))
#define CLEAR_ALL_LED(status)		((status)=((status)&0x0f))


// Functions
int initRobo(char *serialdev);
int writePanTiltServos(float panAngle, float tiltAngle);
float getRoboBatteryInfo();
int closeRobo();

int send_command(byte cmd, byte val);
//////////BLOCKING CALLS
int read_reply(int data_size, byte *buffer);
int get_robot_status(byte *status);

#endif /* ROBO_H_ */
