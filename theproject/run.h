//this file contains the declaration of the run class, which will handle the logic of a
//single run of the model. The current idea is to do many runs per execution, so this class
//will help to keep each run properly separate from every other.
/*


*/
#ifndef RUN_H
#define RUN_H
//#include "util.h"

#include "agent.h"
#include "config.h"
#include "statsCollector.h"

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_01.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/random/uniform_int.hpp>
#include <lemon/list_graph.h>
#include <lemon/graph_to_eps.h>
#include <string>
//#include <lemon/dimacs.h>
#include <vector>
#include <iostream>
using namespace std;
using namespace lemon;	
class agent;
class run{

public:
	int utilityDecreasingTrades;
	int returnConfigSeed(){return cfg.seed;}
	void debugMRS();
	vector<double> tally;
	vector<double> indirectTally; //stores only what is traded FOR while in indirect mode
	//constructors:
	run(config c){
		cfg =c;
		activeAgents = cfg.N; //assume all agents want to trade at beginning
		tally.resize(cfg.M, 0);
		indirectTally.resize(cfg.M,0);
		assert(getTallyTotal() == 0);
		agentMap = new ListGraph::NodeMap<agent>(g);
		prng.seed(returnConfigSeed()); //seed the prng
		createAgents(); //creates nodes and maps agents to them. Also makes list of nodes for fast random access	
		switch(cfg.T){ //the calls in this switch statement should set up topologies AND set up neighbor relations
			case complete:
				completeGraphSetup();
				break;
			case star:
				starGraphSetup();
				break;
			case ring:
				ringGraphSetup();
				break;
			case grid:
				gridGraphSetup();
				break;
			case tree:
				treeGraphSetup();
				break;
			case smallWorld:
				smallWorldGraphSetup();
				break;
			case power:
				powerGraphSetup();
				break;
		}
		
		
	}
	//destructors:
	~run() {
		delete agentMap;
	}
	//accessors:
	double gini();
	long double avgUtilIndirect(); //return avg utility of indirect traders
	double avgUtilDirect();  //return avg utility of direct traders
	double getTallyTotal();
	double greatestPercentOfIndirectTally(); //returns the share of indirect trades of the most commonly indirectly traded good
	double greatestPercentOfTotalTally();
	int getDegree(ListGraph::NodeIt in);
	int totalEdgeCount();
	string getVer(){return "0.4.5";} //todo: keep this updated
	bool atLeastOneCanTrade();
	bool prob(double p){ 
		//assert(p <= 1);

		if(p>=1) //todo: figure out how to successfully include util.h in this file and fix this
			return true;
		
		static boost::uniform_01<boost::mt19937, double> dist(prng); //see http://www.bnikolic.co.uk/blog/cpp-boost-uniform01.html 
		if(p >= dist())
			return true;
		else
			return false;
	}

	
	
	
	void print();
	void createLog();
	long double totalUtility();
	//mutators:
	void execute(); //contains all model run logic. Think of it as main() for the run
	agent* wilhitePickNextAgent(); //for the tick steps, returns next agent to act
	//data:
	
	config cfg;
	statsCollector stats;
	boost::mt19937 prng;
	ListGraph g;
	ListGraph::NodeMap<agent> * agentMap;
	vector<ListGraph::NodeIt> nodesList;
	vector<agent*> agentList;
private:

	//functions:
	void createAgents(); //creates nodes and maps agents to them
	//For setting up topologies. Will also tell agents who their neighbors are
	void completeGraphSetup();
	void starGraphSetup();
	void ringGraphSetup();
	void gridGraphSetup();
	void treeGraphSetup();
	void smallWorldGraphSetup();
	void powerGraphSetup();
	void tick(); //everything that needs be done in one iteration of the simulation
	
	//utility functions:
	ListGraph::NodeIt randomNode(); //returns an iterator to a random node
	//data:
	int activeAgents; //number of agents who still desire to trade
};



#endif