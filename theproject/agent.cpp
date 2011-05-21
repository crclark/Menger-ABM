//contains the implementation of all functions declared in agent.h
#include "config.h"
#include "agent.h"
#include "run.h"
#include "util.h"
#include <math.h>
#include <vector>
#include <iostream>
#include <utility>
#include <fstream>
#include <algorithm>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/variate_generator.hpp>
//constructors:

agent::agent(){

	//nothing much to do here. Make sure to call initialization function
	//before using
}

agent::agent(run * h){
	
	initialize(h); //this constructor is just a wrapper for the real initialization function

	
}

void agent::initialize(run *h){
	numTimesActed = 0;
	lastTradePrice = -1;
	totalTradesInitiated = 0;
	totalSearches = 0;
	totalTradesParticipatedIn = 0;
	static int tempID = 0;
	setID(tempID);
	tempID++;
	home = h;
	gQuantities.resize(home->cfg.M);
	gExponents.resize(home->cfg.M);
	
	
	setgExponents();
	hasActed = false;
	canGainFromTrade = true; //todo: safe to assume true? I think so, but not sure
	indirectExchangeMode = false;

	//set strategy params todo: find good values for these
	indirectExchangeModeProb = home->cfg.indirectExchangeModeProb;
	indirectPrefMult = home->cfg.indirectPrefMult;
	indirectTrader = false; //by default, don't engage in indirect exchange
	tallies.resize(home->cfg.M, 0); //set memories of goods traded to zero
}

//accessors:

double agent::getQuantity(GOOD g) const {

	assert(gQuantities.size() >= g);

	return gQuantities.at(g);

}

double agent::getExponent(GOOD g) const {

		assert(gExponents.size() >= g);

	if(indirectExchangeMode == false){
		return gExponents.at(g);
	}
	else{
		return (gExponents.at(g) + indirectPrefMult*(tallies.at(g)/getTalliesTotal()));
	}
}

double agent::getUtility() const {

	double toReturn = 1.0;

	for(int i =0; i < home->cfg.M; i++){
		toReturn *= pow(getQuantity(i),getExponent(i));
	}

	return toReturn;

}

double agent::getMRS(GOOD a, GOOD b) const {
	
	return (getDUtility(a)/getDUtility(b));

}

double agent::getDUtility(GOOD a) const { //returns the marginal utility of good a

	double toReturn = 1.0;

	for(int i = 0; i < home->cfg.M; i++){

		if(i == a){
			toReturn *= (getExponent(i)*pow(getQuantity(i), (getExponent(i) - 1.0)));
		}
		else {
			toReturn *= pow(getQuantity(i), getExponent(i));
		}
	}

	return toReturn;

}

bool agent::hasConception(const agent& in){

	for(int i = 0; i < otherAgentConceptions.size(); i++){
		if(otherAgentConceptions.at(i).pointer->getID() == in.getID())
			return true;
	}

	return false;
}


//bool agent::hasConception(const agentConception& in){
//
//	for(int i = 0; i < otherAgentConceptions.size(); i++){
//		if(otherAgentConceptions.at(i).getNode() == in.getNode())
//			return true;
//	}
//	return false;
//}

//mutators:

void agent::endow(int option){

	//option determines which good agent will be well-endowed with
	double bigEndowment = 1500;
	double smallEndowment = 150;
	
	assert(option <= (home->cfg.M - 1));
	

	for(int i =0; i < gQuantities.size(); i++){
		if(i == option){
			gQuantities.at(i) = bigEndowment;
		}
		else
		{
			gQuantities.at(i) = smallEndowment;
		}
	}
	
	
	return; 

}

bool agent::tick(){ //returns true if the agent would be able to trade again if tick() is called again 
					//this function does four search-trade iterations, so it need only be called once per agent per timestep
	
	//switch to indirect exchange mode with parameterized probability
	if(home->prob(indirectExchangeModeProb) && indirectTrader  && home->cfg.indirectTrade && numNeighbors() > 1)
		indirectExchangeMode = true;

	numTimesActed++;
	if(canGainFromTrade){
		//if(!neighborsCanGain()){ //todo: this was intended to confer a speedup in some cases, but it may not work
		//	canGainFromTrade = false;
		//	return 0;
		//}
		pair<GOOD, GOOD> mrsPair; //pair of goods to be traded.
		agentConception* tradeTarget; //agent to be traded with
		//ü
		vector<pair<GOOD,GOOD > > MRSList = sortMRS();
		for(int i = 0; i < MRSList.size(); i++){ //start with the highest MRS, search, make trade if possible, if not, go to next
			mrsPair = MRSList.at(i);
		
			if(getMRS(mrsPair.first, mrsPair.second) < 1.0 || kindOfEqual(getMRS(mrsPair.first, mrsPair.second), 1.0))
				break;
			
			tradeTarget = search(mrsPair.first, mrsPair.second); 
			if(tradeTarget != NULL){
				//assert(getMRS(mrsPair.first, mrsPair.second) > 1.0 && tradeTarget->getMRS(mrsPair.second, mrsPair.first) > 1.0);
				trade(*tradeTarget, mrsPair.second, mrsPair.first);
				break;
			}
			
		}
		

		
		
		
		
	}

	//exit indirect exchange mode and return
	indirectExchangeMode = false;
	
	return 0;

}

void agent::addNeighborConception(agent* in){
	agentConception temp(home, in);
	otherAgentConceptions.push_back(temp);

}

double determinePrice(agent one, agent two, GOOD a, GOOD b){ //determine price of a in terms of b
	//the line below is wilhite's version of the price determination. Unfortunately, I am not sure that it is implemented correctly, as wilhite is vague as to how it works
	//return ( (one.getQuantity(a) + two.getQuantity(a)) / (one.getQuantity(b) + two.getQuantity(b)) );
	
	//the current version of the price determination is as suggested by Dr. DalleTezze:
	
	return ( (one.getMRS(a, b) + two.getMRS(a,b))/2);
}



//subclass:



agent::agentConception::agentConception(run* r, agent* a){

	home = r;
	pointer = a;
	
}
double agent::agentConception::getMRS(GOOD a, GOOD b) const{
	
	return pointer->getMRS(a,b);
}

double agent::agentConception::getDUtility(GOOD a) const{

	return pointer->getDUtility(a);
}

void agent::trade(agentConception partner, GOOD a, GOOD b){//trade away a for b. b is always the numeraire.
	// begin wilhite specific code
	//todo: change this function once we extend wilhite. Specifically, a will always be numeraire
	//determine price. g0 is transferred in units of 1, and g1 in whatever is determined by the line below
	double debugU = getUtility(); //for postcondition assert
	double debugU2 = partner.pointer->getUtility();
	double price = 0.0;
	
	assert(a!=b);
	
#ifdef DEBUGLOGS
	fstream fout("tradebug.log", ios::out | ios::app);  //for debugging

	//print info////////////////

	fout<<getID()<<'\t'<<partner.pointer->getID()<<'\t';
	fout<<getQuantity(0)<<'\t'<<getQuantity(1)<<'\t';
	fout<<partner.pointer->getQuantity(0)<<'\t'<<partner.pointer->getQuantity(1)<<'\t';
	////////////////////////////
#endif
	price = determinePrice(*this, * partner.pointer, b, a); //get price of b //NOTE: the a and b arguments to this function were flipped before! Make sure that current call is indeed correct!
	
	gQuantities.at(a) -= price;
	partner.pointer->gQuantities.at(a) += price;
	gQuantities.at(b) += 1.0;
	partner.pointer->gQuantities.at(b) -= 1.0;
	
	//print///////////////
#ifdef DEBUGLOGS
	fout<<price<<'\t';
	fout<<getQuantity(0)<<'\t'<<getQuantity(1)<<'\t';
	fout<<partner.pointer->getQuantity(0)<<'\t'<<partner.pointer->getQuantity(1)<<'\t';
	fout<<getUtility() - debugU<<'\t'<<partner.pointer->getUtility() - debugU2;	
	fout<<endl;
#endif
	//////////////////////


	

	//undo u-decreasing trades and bail out

	if(debugU > getUtility() || debugU2 > partner.pointer->getUtility()){
		gQuantities.at(a) += price;
	partner.pointer->gQuantities.at(a) -= price;
	gQuantities.at(b) -= 1.0;
	partner.pointer->gQuantities.at(b) += 1.0;
	return;
	}

	//strategy stats:
	home->tally.at(a) += price;
	home->tally.at(b) += 1.0;
	if(indirectExchangeMode)
		home->indirectTally.at(b) += 1.0; //updates only when someone trades FOR something while in indirect trade mode
	updateTallies(a, b, price, 1.0, 1);
		//stats stuff:
	home->stats.incTrades();
	lastTradePrice = price;
	totalTradesInitiated++;
	totalTradesParticipatedIn++;
	partner.pointer->totalTradesParticipatedIn++;

	//print more 
#ifdef DEBUGLOGS
	fstream fout2("qaftertrade.log", ios_base::out | ios_base::app);
	
	for(ListGraph::NodeIt i(home->g); i != INVALID; ++i){
		fout2<<(*(home->agentMap))[i].getQuantity(0)<<'\t'<<(*(home->agentMap))[i].getQuantity(1)<<'\t';
	}
	fout2<<endl;
#endif
	///////////


	
}

void agent::setgExponents(){//todo: determine parameters

	for(int i = 0; i<home->cfg.M; i++){ //todo: make real function instead of placeholder
		gExponents[i] = 1.0;
	}

}

void neighbors(agent& one, agent& two){
	
	if(!one.hasConception(two)) //if they are not already aware of each other
		one.addNeighborConception(&two);
		
	
	if(!two.hasConception(one))
		two.addNeighborConception(&one);

}

agent::agentConception* agent::search(GOOD toBuy, GOOD toSell){ //return conception of the best trade partner. inlined because of incomprehensible error otherwise
		home->stats.incSearches(); //collect statistic
		totalSearches++;


		vector<agentConception*> candidates; //all who have MRS(toSell, toBuy) >1
		for(int i = 0; i < otherAgentConceptions.size(); i++){
			if(/*otherAgentConceptions.at(i).pointer->getMRS(toSell, toBuy) > 1.0 &&*/  otherAgentConceptions.at(i).pointer->getMRS(toBuy, toSell) < getMRS(toBuy, toSell) && determinePrice(*this, * otherAgentConceptions.at(i).pointer, toBuy, toSell) < getMRS(toBuy,toSell))
				candidates.push_back(&otherAgentConceptions.at(i));
		}
		
		if(candidates.size() == 0){
			
			return NULL;     //todo: temporary way to deal with no trade partners being found
		}
		
		//ü
		//boost::uniform_int<> intDist(0,(candidates.size()-1)); //pick random candidate boilerplate

		//boost::variate_generator<boost::mt19937&, boost::uniform_int<> >
  //       die(home->prng, intDist);


		//
		//return candidates.at(die()); //returns random candidate

		//find best candidate
		int bestCandidateIndex = 0;
		for(int i=1; i < candidates.size(); i++){
			
			if( determinePrice(*this,  (*(*candidates.at(i)).pointer), toBuy, toSell) < determinePrice(*this,  (*(*candidates.at(bestCandidateIndex)).pointer), toBuy, toSell))
				bestCandidateIndex = i;
		}

		return candidates.at(bestCandidateIndex);


	}

pair<GOOD, GOOD> agent::maxMRS(){ //of all MRS's, pick the maximum

	pair<GOOD, GOOD> highestYet;
	highestYet.first = 0;
	highestYet.second = 1;

	for(int i = 0; i < home->cfg.M; i++){

		for(int j = 0; j < home->cfg.M; j++){
			
			if(i != j){ //MRS(a,a) makes no sense
				if(getMRS(i,j) > getMRS(highestYet.first, highestYet.second)){
					highestYet.first = i;
					highestYet.second = j;
				}
			}
		}
	}

	return highestYet;
}

bool agent::neighborsCanGain(){

	for(int i = 0; i < otherAgentConceptions.size(); i++){
		if(otherAgentConceptions.at(i).pointer->canGainFromTrade)
			return true;
	}

	return false;
}

vector<pair<GOOD,GOOD > > agent::sortMRS(){

	vector<pair<GOOD,GOOD > > toReturn;

	//todo: sort MRS's from highest to lowest and return as vector of good pairs

	//first, get all the possible combinations in there

	for(int i = 0; i < home->cfg.M; i++){

		for(int j = 0; j <home->cfg.M; j++){

			if(i!=j){
				pair<GOOD, GOOD> temp;
				temp.first = i;
				temp.second = j;
				toReturn.push_back(temp);
			}
		}
	}

	//now sort them
	sortCriterion crit;
	crit.me = this;
	sort(toReturn.begin(),toReturn.end(), crit);
	
	return toReturn;
	
}

void agent::updateTallies(GOOD a, GOOD b, double aQuant, double bQuant, unsigned int recurse){
	
	//update tallies
	tallies.at(a) += aQuant;
	tallies.at(b) += bQuant;

	//if recurse != 0, recurse to neighbors so they are updated, too

	if(recurse !=0){

		for(int i = 0; i < otherAgentConceptions.size(); i++){
			otherAgentConceptions.at(i).pointer->updateTallies(a, b, aQuant, bQuant, (recurse - 1));
		}
	}

}

double agent::getTalliesTotal() const {

	double toReturn = 0;

	for(int i = 0; i < tallies.size(); i++){
		toReturn += tallies.at(i);
	}

	return toReturn;

}
