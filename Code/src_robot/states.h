
/****************************************
 * 	states.h
 *
 *	<description>
 *
 *  Created on: Oct, 2013
 *      Author: dbs
 *     Project: S.R.
 ****************************************/


#ifndef STATES_H_
#define STATES_H_

#include "datatypes.h"
#include "intSys.h"

/* Size: 16b */
typedef struct State
{
	struct State *next;
	char *sit;
	int id;
	void *data;
} State;


char *printStates(IntSys *is, char *str);
State *findState(IntSys *is, ConceptName *sit);
State *addState(IntSys *is, ConceptName *sit);
void removeState(IntSys *is, ConceptName *sit);
void cleanStates(IntSys *is);

#endif /* STATES_H_ */
