
/****************************************
 * 	states.c
 *
 *	<description>
 *
 *  Created on: Oct, 2013
 *      Author: dbs
 *     Project: S.R.
 ****************************************/


#include "states.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


char *printStates(IntSys *is, char *str)
{
	char *ptr = str;
	int i;
	State *state = is->sit;

	if (!state)
	{
		sprintf(str, "%s", "[Null]");
		return str;
	}
	for (i=1; ; ++i)
	{
		sprintf(str, "[%d]", i);
		strcat(str, (char *)state->sit);
		if ((state=state->next))
			strcat(str, ", ");
		else
			return ptr;
		while (*str)
			++str;
	}
	return ptr;
}

State *findState(IntSys *is, char *sit)
{
	State *state = is->sit;

	if (!state)
		return 0;
	do
	{
		if (strcmp(state->sit, sit)==0)
			return state;
	}
	while ((state=state->next));
	return 0;
}

State *addState(IntSys *is, char *sit)
{
	State *state, *newState;

	if (!(newState=(State *)malloc(sizeof(State))))
		return 0;
	newState->next = 0;
	newState->data = 0;
	newState->sit = sit;
	if (state=is->sit)
	{
		while (state->next)
			state = state->next;
		state->next = newState;
	}
	else
		is->sit = newState;
	is->sitChange = 1;
	return newState;
}

void removeState(IntSys *is, char *sit)
{
	State *state, *prevState;

	if (!(state=is->sit))
		return;
	prevState=0;
	do
	{
		if (strcmp(state->sit, sit)==0)
			goto gotState;
		prevState = state;
	}
	while ((state=state->next));
	return;
gotState:
	if (prevState)
		prevState->next = state->next;
	else
		is->sit = state->next;
	is->sitChange = 1;
	free(state);
}

void cleanStates(IntSys *is)
{
	State *state, *tmp;

	if (!(state=is->sit))
		return;
	is->sit = 0;
	is->sitChange = 1;
	do
	{
		tmp = state;
		state = state->next;
		free(tmp);
	}
	while (state);
}
