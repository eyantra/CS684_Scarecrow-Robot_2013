
/****************************************
 * 	rules.c
 *
 *	<description>
 *
 *  Created on: Jan 13, 2012
 *      Author: dbs
 *     Project: soccerRobo
 ****************************************/


#include "rules.h"

#include "states.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


#define NUM_EMF 4
char *emfName[NUM_EMF] = {
							"find_blob",
							"reach_blob",
							"place_ball",
							"finish"
						  };


Rule *findRule(IntSys *is)
{
	int i, j;
	int count=0;
	int match;
	State *sit;
	State *curSit = is->sit;
	ConceptName **cnc, *tmp;
	Rule *rule = is->rule;
	Rule *tmprl = 0;

	if (!curSit)
		return 0;
	while (rule)
	{
		for (sit=curSit, match=1, j=0; sit; sit=sit->next)
		{
			tmp = sit->sit;
			for (i=rule->numsit, cnc=rule->sit; i; --i, ++cnc)
			{
				if (strcmp(tmp, *cnc)==0)
					break;
			}
			if (i)
				++j;
			else
				match = 0;
		}
		if (match)
			return rule;
		if (j > count)
		{
			count = j;
			tmprl = rule;
		}
		rule = rule->next;
	}
	return tmprl;
}

Rule *createRule(IntSys *is, word numsit, word respid, char *respdesc)
{
	int len;
	char *str;
	Rule *rule, *newrl;
	ConceptName **cnclist;

	len = strlen(respdesc);
	if (!(numsit && (newrl=(Rule *)malloc(sizeof(Rule)
											+(sizeof(ConceptName *)*numsit)
											+(len+1)))))
		return 0;
	cnclist = (ConceptName **)(newrl+1);
	newrl->next = 0;
	newrl->numsit = numsit;
	newrl->sit = cnclist;
	newrl->respid = respid;
	if (len)
	{
		str = (char *)(cnclist+numsit);
		newrl->respdesc = str;
	}
	else
		newrl->respdesc = 0;
	if ((rule=is->rule))
	{
		while (rule->next)
			rule = rule->next;
		rule->next = newrl;
	}
	else
		is->rule = newrl;
	return newrl;
}

int saveRules(IntSys *is, char *filename)
{
	int i;
	Rule *rule = is->rule;
	ConceptName **cnc;
	FILE *fp;

	if (!(fp=fopen(filename, "wb")))
		return 0;
	while (rule)
	{
		fwrite(rule, sizeof(Rule), 1, fp);
		fprintf(fp, "%s", rule->respdesc);
		for (i=rule->numsit, cnc=rule->sit; i; --i, ++cnc)
			fprintf(fp, "%s", (char *)(*cnc));
		rule = rule->next;
	}
	return 1;
}

int loadRulesBin(IntSys *is, char *filename)
{
	int i;
	char str[64];
	ConceptName **cnc;
	FILE *fp;
	Rule *newrl, *prevrl=is->rule;
	Rule tmprl;

	if (!(fp=fopen(filename, "rb")))
		return 0;

	while (fread(&tmprl, sizeof(Rule), 1, fp) > 0)
	{
		fscanf(fp, "%s", str);
		if (!(newrl=(Rule *)malloc(sizeof(Rule)+(sizeof(ConceptName *)*tmprl.numsit)
								   +(strlen(str)+1))))
			return -1;
		for (i=tmprl.numsit, cnc=tmprl.sit; i; --i, ++cnc)
		{
			fscanf(fp, "%s", str);
			if (!(*cnc=malloc(strlen(str)+1)))
				return -1;
			strcpy(*cnc, str);
		}
		*newrl = tmprl;
		if (prevrl)
			prevrl->next = newrl;
		else
			is->rule = newrl;
		newrl->next = 0;
		prevrl = newrl;
	}
	return 1;
}

ConceptName *getSit(Rule *rule, ConceptName *sit, int len)
{
	int i;
	ConceptName **cnc;

	while (rule)
	{
		for (i=rule->numsit, cnc=rule->sit; i; --i, ++cnc)
		{
			if (strncmp(sit, *cnc, len)==0)
				return *cnc;
		}
		rule = rule->next;
	}
	return 0;
}

char *getWord(char **code, Rule *rule)
{
	char *str = *code;
	char *ptr;
	int len = 0;
	int isComment = 0;

	if (!*str)
		return 0;
	while (isspace(*str)||(ispunct(*str)&&*str!='_'&&*str!='"'))
		++str;
	if (*str == '"')
	{
		isComment = 1;
		++str;
	}
	ptr = str;
	if (isComment)
		while (*str&&*str!='\n')
			++str, ++len;
	else
		while (*str&&!(ispunct(*str)&&*str!='_')&&!isspace(*str))
			++str, ++len;
	if (!len)
		return 0;
	while (*str && !(ispunct(*str)&&*str!='_'))
		++str;
	*code = str;
	if ((str=getSit(rule, ptr, len)))
	{
		printf("got previous\n");
		return str;
	}
	if (!(str=malloc(len+1)))
		return 0;
	strncpy(str, ptr, len);
	*(str+len) = '\0';
	return str;
}

int loadRules(IntSys *is, char *filename)
{
	int i;
	int line=1;
	int numWord;
	char strBuf[256];
	char *word[16];
	char *resp;
	char *str;
	char *desc;
	ConceptName **cnc;
	FILE *fp;
	Rule *newrl, *prevrl=is->rule;
	Rule *rule = is->rule;

	if (!(fp=fopen(filename, "r")))
		return 0;
	while (fgets(str=strBuf, 256, fp) != 0)
	{
		numWord = 0;
		while (1)
		{
			if (numWord > 15)
			{
				fprintf(stderr, "Num word error!\n");
				return 0;
			}
			if (!(word[numWord]=getWord(&str, rule)))
				goto nextLine;
			++numWord;
			printf("Word no %d - %s\n", numWord, word[numWord-1]);
			if (!*str)
			{
				fprintf(stderr, "Response not indicated (line: %d).\n", line);
				return 0;
			}
			if (*str=='-'||*str=='>')
				break;
		}
		if (!(resp=getWord(&str, rule)))
		{
			fprintf(stderr, "Response not indicated (line: %d).\n", line);
			return 0;
		}
		desc = getWord(&str, rule);
		printf("emf name - %s decs -- %s\n", resp, desc);
		if (!(newrl=(Rule *)malloc(sizeof(Rule)+(sizeof(ConceptName *)*numWord))))
			return -1;
		newrl->next = 0;
		newrl->numsit = numWord;
		newrl->respdesc = desc;
		newrl->sit=cnc=(ConceptName **)(newrl+1);
		for (i=0; i<NUM_EMF; ++i)
		{
			if (strcmp(resp, emfName[i])==0)
			{
				newrl->respid = i;
				break;
			}
		}
		if (i == NUM_EMF)
		{
			fprintf(stderr, "Emf name invalid (line: %d).\n", line);
			return 0;
		}
		free(resp);
		for (i=0; i<numWord; ++i, ++cnc)
			*cnc = word[i];
		if (prevrl)
			prevrl->next = newrl;
		else
			rule=is->rule=newrl;
		prevrl = newrl;
nextLine:
		++line;
	}
	fclose(fp);
	return 1;
}
