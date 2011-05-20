//This file declares a class that holds, tracks, and outputs all statistics for a run. 
//Some data will be kept primarily in the graph, however.
//This class will be notified by agents or the run object when it needs to be updated; it will not actively
//ask for information.

#ifndef STATSCOLLECTOR_H
#define STATSCOLLECTOR_H
#include <string>
using namespace std;
class statsCollector{

public:

	//constructors:

	statsCollector(){
		searches = 0;
		trades = 0;
		ticks = 0;
	}

	//accessors:
	long int getSearches(){return searches;};
	long int getTrades(){return trades;};
	long int getTicks(){return ticks;};
	
	//mutators:
	void incSearches(){searches++;};
	void incTrades(){trades++;};
	void incTicks(){ticks++;};
	
private:

	//data:
		long int searches;
		long int trades;
		long int ticks;

	//functions:

};

#endif