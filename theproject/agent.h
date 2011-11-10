//this file contains the declarations for the agent class

#ifndef AGENT_H
#define AGENT_H
#include "config.h"
#include "run.h"
#include <vector>
#include <utility>
#include <lemon/smart_graph.h>
#include <boost/random/uniform_int.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/random/mersenne_twister.hpp>
using namespace lemon;
using namespace std;
class run;
class agent{

public:
	
	//per agent stats, for logging purposes:
	int numTimesActed;
	double lastTradePrice;
	int totalTradesInitiated;
	int totalSearches;
	int totalTradesParticipatedIn;

	//related to the function of the model:
	bool hasActed; //for tracking whether the agent has acted for a given turn
	bool canGainFromTrade; //for tracking whether the agent wants to trade further
	bool neighborsCanGain();
	bool indirectExchangeMode; //should ONLY be turned on for the duration of a agent::tick(). should NEVER be on when outside of agent::tick() function -- if it were, would mess up reporting utility to user
	
	//strategy parameters:
	bool indirectTrader;
	double indirectExchangeModeProb; //probability of going into indirectExchangeMode for the duration of a tick
	double indirectPrefMult; //coefficient to control strength of preference for indirect exchange
	vector<double> tallies; //records how many of each good the agent has seen get traded

	//constructors and initialization:
	agent(); //this appears to be needed for one of lemon's data structures. I don't use it myself 
	agent(run * h); //this should be the constructor used, as it calls the initialize function
	void initialize(run * h); //always called inside the constructor
	//accessors:
	double getQuantity(GOOD g) const; //reports how many of g the agent has
	double getExponent(GOOD g) const; //reports the exponent associated with g in the agent's Cobb-Douglas utility function
	double getUtility() const;   //reports the agent's total utility
	double getMRS(GOOD a, GOOD b) const; //returns the MRS of good a with respect to b
	double getDUtility(GOOD a) const; //returns the value of the partial derivative of the utility function with respect to a
	vector<double> getgQuantities() const {return gQuantities;}
	vector<double> getgExponents() const {return gExponents;}
	int getID() const {return ID;}
	bool hasConception(const agent& in);
	int numNeighbors() {return otherAgentConceptions.size();}
	double getTalliesTotal() const;
	//mutators:
	void endow(int option); //this is the responsibility of the user, not the agent. option determines which good the agent has lots of
	bool tick(); //returns true if the agent sees another potential trade partner 
	void addNeighborConception(agent * in);
	void setID(int in) {ID = in;}
	void updateTallies(GOOD a, GOOD b, double aQuant, double bQuant, unsigned int recurse);
	//friend functions:
	friend double determinePrice(agent one, agent two, GOOD a, GOOD b);
	pair<GOOD, GOOD> maxMRS(); // finds the max MRS for this agent, and puts the two goods in the pair
	
private:

	//nested classes:

	struct agentConception{ //used to store beliefs about other agents. Kind of obsolete, since we never have agents have inaccurate beliefs. We could easily add that, though, with this system.
		
		//constructors:
		
		agentConception(run* r, agent* a);

		//accessors:
		double getMRS(GOOD a, GOOD b) const;
		double getDUtility(GOOD a) const;
		//mutators:
		void update();
		//data:
		agent* pointer;
		run * home;
		
		

	};


	//functions:
	void trade(agentConception partner, GOOD a, GOOD b); //trades a for b with partner
	void setgExponents(); //exponents are always all set to 1
	vector<pair<GOOD,GOOD > > sortMRS();
	struct sortCriterion {
		agent* me;
		bool operator()(pair<GOOD,GOOD> i, pair<GOOD,GOOD> j){return(me->getMRS(i.first,i.second) > me->getMRS(j.first,j.second));}
	};
	//data:
	vector<double> gQuantities; //stores the quantities of the various goods the actor owns
	vector<double> gExponents;  //stores the exponents of the agent's Cobb-Douglas function
	vector<agentConception> otherAgentConceptions;
	run * home; //pointer to the model we are living in
	int ID; //unique agent ID

public: //second public section because I am tired of fighting the compiler on forward declarations
	
	agentConception* search(GOOD toBuy, GOOD toSell);
};

//other functions:

void neighbors(agent& one, agent& two); //sets the two agents so that they are aware they are neighbors

#endif
