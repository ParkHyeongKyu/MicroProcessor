#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>
#include "cachelab.h"

//define struct
typedef struct {
	bool v;
	int tag;
	int LRU;
} line;

typedef struct {
	line* lines;
} set;

typedef struct {
	set* sets;
	size_t setnum;
	size_t linenum;
} cache;

//define and initalize global variable
cache c = {};
int s = 0;
int b = 0;
size_t hitnum = 0;
size_t missnum = 0;
size_t evicnum = 0;

void process(int address);

int main(int argc, char* argv[])
{
	FILE* fp = 0;
	int op;
	char operation;
	int address;

	//get options
	while ((op = getopt(argc, argv, "s:E:b:t:")) != EOF)
	{
		switch (op)
		{
		case 's':
			s = atoi(optarg);
			if (s == 0)
				return 1;
			c.setnum = 2 << s;
			break;
		case 'E':
			c.linenum = atoi(optarg);
			if (c.linenum == 0)
				return 1;
			break;
		case 'b':
			b = atoi(optarg);
			if (b == 0)
				return 1;
			break;
		case 't':
			if (!(fp = fopen(optarg, "r")))
				return 1;
			if (fp == 0)
				return 1;
			break;
		default:
			return 1;
		}
	}

	//Dynamic allocation and initalize the cache
	c.sets = malloc(sizeof(set) * c.setnum);
	for (int i = 0; i < c.setnum; i++)
	{
		c.sets[i].lines = malloc(sizeof(line) * c.linenum);
		for (int j = 0; j < c.linenum; j++)
		{
			c.sets[i].lines->v = 0;
			c.sets[i].lines->tag = 0;
			c.sets[i].lines->LRU = 0;
		}
	}

	//use process function
	while (fscanf(fp, " %c %x%*c%*d", &operation, &address) != EOF)
	{
		if (operation == 'I')
			continue;
		process(address);
		if (operation == 'M')
			process(address);
	}
	printSummary(hitnum, missnum, evicnum);

	//clear file
	fclose(fp);
	for (size_t i = 0; i < c.setnum; i++)
		free(c.sets[i].lines);
	free(c.sets);
	return 0;
}

void update(set * ss, size_t linenum);

void process(int address)
{
	size_t si = (0x7fffffff >> (31 - s)) & (address >> b);
	int t = 0xffffffff & (address >> (s + b));

	set * ss = &c.sets[si];

	//find if hit occur
	for (size_t i = 0; i < c.linenum; i++)
	{
		line* l = &ss->lines[i];
		if (!l->v)
			continue;
		if (l->tag != t)
			continue;

		hitnum++;
		update(ss, i);
		return;
	}

	missnum++;

	//find if they have empty line
	for (size_t i = 0; i < c.linenum; i++)
	{
		line* l = &ss->lines[i];
		if (l->v)
			continue;
		l->v = true;
		l->tag = t;
		update(ss, i);
		return;
	}

	evicnum++;

	//find the least recently used line in the set
	for (size_t i = 0; i < c.linenum; i++)
	{
		line* l = &ss->lines[i];
		if (l->LRU)
			continue;
		l->v = true;
		l->tag = t;
		update(ss, i);
		return;
	}
}

//LRU part update function
void update(set * ss, size_t linenum)
{
	line* l = &ss->lines[linenum];

	for (size_t i = 0; i < c.linenum; i++)
	{
		line* temp = &ss->lines[i];
		if (!temp->v)
			continue;
		if (temp->LRU <= l->LRU)
			continue;

		temp->LRU--;
	}

	l->LRU = c.linenum - 1;
}
