
/****************************************
 * 	rules.h
 *
 *	<description>
 *
 *  Created on: Oct, 2013
 *      Author: dbs
 *     Project: S.R.
 ****************************************/


#ifndef RULES_H_
#define RULES_H_

#include "datatypes.h"
#include "intSys.h"


/* Size: 16b */
typedef struct Rule
{
	struct Rule *next;
	word numsit;
	word respid;
	char **sit;
	char *respdesc;
} Rule;


Rule *findRule(IntSys *is);
Rule *createRule(IntSys *is, word numsit, word respid, char *respdesc);
int saveRules(IntSys *is, char *filename);
int loadRules(IntSys *is, char *filename);

#endif /* RULES_H_ */
