#pragma once
#include <stdio.h>

#define QUEUE_MAX 500

struct IndexWeightPair
{
	int index;
	int weight;
};

struct PriorityQueue
{
	IndexWeightPair pairs[QUEUE_MAX];
	int numInQueue = 0;

	void init()
	{
		for (int i = 0; i < QUEUE_MAX; i++)
		{
			pairs[i].index = -1;
			pairs[i].weight = -1;
		}

		numInQueue = 0;
	}

	bool isEmpty()
	{
		return numInQueue <= 0;
	}

	void push(int index, int weight)
	{
		for (int i = 0; i < QUEUE_MAX; i++)
		{
			if (pairs[i].index == -1)
			{
				pairs[i].index = index;
				pairs[i].weight = weight;
				numInQueue++;
				return;
			}
		}

		printf("QUEUE MAX hit!! Ahhhhh.");
	}

	void pop(int *index, int *weight)
	{
		*index = -1;
		*weight = -1;

		int pairIndexToDelete = -1;

		int minWeight = 2000;

		for (int i = 0; i < QUEUE_MAX; i++)
		{
			if (pairs[i].index != -1 && pairs[i].weight < minWeight)
			{
				*index = pairs[i].index;
				*weight = pairs[i].weight;
				minWeight = pairs[i].weight; 
				pairIndexToDelete = i;
			}
		}

		if (*index != -1)
		{
			numInQueue--;
			pairs[pairIndexToDelete].index = -1;
			pairs[pairIndexToDelete].weight = -1;
		}
	}
};