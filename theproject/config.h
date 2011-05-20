//this file will contain macro definitions and a struct for storing the parameters
//for individual runs of the model

#ifndef CONFIG_H
#define CONFIG_H
#include <assert.h>
#include <cstdlib> 
#include <string>
using namespace std;
//#define DEBUGLOGS //uncomment this line when you want to print tons of logs instead of just normal logs
#define GOOD unsigned int //type def for goods
enum topology {complete, star, ring, grid, tree, smallWorld, power}; //for config processing of topology parameter
struct config{

	int N; //number of agents
	int M; //number of goods
	topology T; //topology of the graph
	unsigned int seed; //PRNG seed
	string filename;
	bool indirectTrade;
	double indirectTraderPercentage;
	double indirectExchangeModeProb;
	double indirectPrefMult;

	string tString(){

		switch(T){

			case complete:
				return "complete";
			case star:
				return "star";
			case ring:
				return "ring";
			case grid:
				return "grid";
			case tree:
				return "tree";
			case smallWorld:
				return "small world";
			case power:
				return "power";
		}

		return "ERROR";
	}

};



#endif