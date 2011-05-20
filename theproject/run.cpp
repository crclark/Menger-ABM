#include "run.h"
#include "agent.h"
#include "config.h"
#include "statsCollector.h"
#include <utility>
#include <vector>
#include <math.h>
#include <iostream>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/variate_generator.hpp>
#include <lemon/list_graph.h>
#include <lemon/connectivity.h>
#include <lemon/concepts/maps.h>
#include <fstream>
#include <string>
#include <algorithm>
using namespace std;
using namespace lemon;

void run::createAgents(){ //creates nodes and maps agents to them. Also makes list of nodes
	utilityDecreasingTrades = 0;
	
	int count = cfg.N/cfg.M;
	int toEndow = 0;
	for(int i = 0; i < cfg.N; i++){
		agent tempAgent(this); //properly initialized agent 
		ListGraph::Node tempNode = g.addNode(); //node
		tempAgent.setID(i); //give agent a unique integer identifier
		if(i > 0 && (i % count) == 0){ //This line ensures that one block of agents gets first good, one block gets second, etc.
			toEndow++;
		}
		if(toEndow == cfg.M)
			toEndow--;
		assert(toEndow <= (cfg.M - 1));
		tempAgent.endow(toEndow); //endow
		
		if(prob(cfg.indirectTraderPercentage)) //with probability .1, make it an indirect trader
			tempAgent.indirectTrader = true;

		agentMap->set(tempNode, tempAgent); //map the agent to the node
	}

	for(ListGraph::NodeIt i(g); i != INVALID; ++i){ 
		nodesList.push_back(i); //todo: maybe bad idea. (for fast random access to particular nodes)
		agentList.push_back(&(*agentMap)[i]);
	}

}
void run::completeGraphSetup(){
	

	for(ListGraph::NodeIt i(g); i != INVALID; ++i){

		for(ListGraph::NodeIt j(g); j != INVALID; ++j){

			if(i != j && (findEdge(g, i, j) == INVALID)){ //if i != j and there is no edge between them
				g.addEdge(i, j); 
				neighbors((*agentMap)[i], (*agentMap)[j]); //add edge and set neighborness 
			}
		}
	}
			assert(parallelFree(g)); //make sure we don't have more than one edge connecting the same two nodes
			assert(connected(g)); //ensure the graph is connected. If not, something is horribly wrong with this function
}
void run::starGraphSetup(){
	
	ListGraph::NodeIt hub = randomNode();

	for(ListGraph::NodeIt i(g); i != INVALID; ++i){
		if(hub != i){
			g.addEdge(i, hub);
			neighbors((*agentMap)[i],(*agentMap)[hub]);
		}
	}
	assert(parallelFree(g));
	assert(connected(g));
}
void run::ringGraphSetup(){

	int degree = 4; //only number I could find in the Wilhite paper, even though it was referring to another paper

	for(ListGraph::NodeIt i(g); i != INVALID; ++i){ //for all nodes...

		for(ListGraph::NodeIt j(g); j != INVALID; ++j){//for each step, connect to the i+j and i-j agents
			
			for(int k = 1; k <= (degree/2); k++){
				if( ((*agentMap)[j].getID() == ((((*agentMap)[i].getID()-k) + cfg.N)%cfg.N) ) || ((*agentMap)[j].getID() == (((*agentMap)[i].getID() + k)% cfg.N)) ){
					
					if(findEdge(g,i,j) == INVALID){
						g.addEdge(i,j);
						neighbors((*agentMap)[i], (*agentMap)[j]);
					}
				}
				
			}

		}

	}
	
	assert(parallelFree(g));
	assert(connected(g));
}
void run::gridGraphSetup(){

	for(ListGraph::NodeIt i(g); i != INVALID; ++i){

		for(ListGraph::NodeIt j(g); j != INVALID; ++j){

			if( ((*agentMap)[j].getID()==((((*agentMap)[i].getID() + 1) + cfg.N) % cfg.N)) || ((*agentMap)[j].getID()==((((*agentMap)[i].getID() + int(pow(double(cfg.N), .5))) + cfg.N) % cfg.N)) ){
				if(findEdge(g,i,j) == INVALID){
						g.addEdge(i,j);
						neighbors((*agentMap)[i], (*agentMap)[j]);
				}
			}

			if( ((*agentMap)[j].getID()==((((*agentMap)[i].getID() - 1) + cfg.N) % cfg.N)) || ((*agentMap)[j].getID()==((((*agentMap)[i].getID() - int(pow(double(cfg.N), .5))) + cfg.N) % cfg.N)) ){
				if(findEdge(g,i,j) == INVALID){
						g.addEdge(i,j);
						neighbors((*agentMap)[i], (*agentMap)[j]);
				}
			}
		}
	}

	assert(parallelFree(g));
	assert(connected(g));
}
void run::treeGraphSetup(){ //NOTE: broken! todo: fix

	int degree = 4; //number of child nodes per node. 2 would be a binary tree. 4 is a guess. Wilhite does not specify
					//NOTE only works if this is 4. May change later if I have time				

	for(ListGraph::NodeIt i(g); i != INVALID; ++i){
		
		if( (*agentMap)[i].getID() == 0 ){ //case i == 1 from Wilhite 
			
			for(ListGraph::NodeIt j(g); j != INVALID; ++j){ //go through other nodes, connecting them to i if appropriate
				for(int k = 0; k <= degree; k++){
					if( (*agentMap)[j].getID() == (k+2)){
						g.addEdge(i,j);
						neighbors((*agentMap)[i], (*agentMap)[j]);
					}
				}
			}
		}

		else{ //i is greater than 0

			for(ListGraph::NodeIt j(g); j!= INVALID; ++j){

				for(int k = 0; k < degree; k++){
					if( (*agentMap)[j].getID() == ((degree * (*agentMap)[i].getID()) + k) ){ //implements the second condition about j in Wilhite's description of trees
						g.addEdge(i,j);
						neighbors((*agentMap)[i], (*agentMap)[j]);
					}
				}
			}
		}
	}

	assert(parallelFree(g));
	assert(connected(g));
	assert(1==2); //todo: don't use this function until fixed. 
}

void run::smallWorldGraphSetup(){ //mirrors that of (Wilhite 2006)
	//pseudocode acquired from http://en.wikipedia.org/wiki/Watts_and_Strogatz_model#Algorithm on 12-1-10
	
	ringGraphSetup();
	const double beta = 0.2;

	for(ListGraph::NodeIt i(g); i != INVALID; ++i){ //for every node i

		for(ListGraph::IncEdgeIt e(g, i); e != INVALID; ++e){ //for every edge connected to node i
			//cout<<(g.u(e)!=i)<<endl;
			if( (g.u(e) != i) && ((*agentMap)[g.u(e)].getID() > (*agentMap)[i].getID()) ){ //these two ifs are for the requirement that i < j, stipulated in the wikipedia article's pseudocode
				//cout<<"in the big if"<<endl;
				if(prob(beta)){ //with probability of beta, rewire
					//cout<<"prob beta happened"<<endl;		
					//keep trying to add an edge from i to a random node j such that j is not i and there is not an edge between i and j already
					for(bool done = false; done != true;){
						ListGraph::NodeIt j = randomNode();

						if( (j != i) && (findEdge(g,i,j) == INVALID)){
							g.addEdge(i, j);
							neighbors((*agentMap)[i], (*agentMap)[j]);
							done = true;
							g.erase(e); //erase is only done if we add a new edge, as there is some possibility that otherwise we could erase without having a new edge to make
						}
					
					}
				}
			}

			if( (g.v(e) != i) && ((*agentMap)[g.v(e)].getID() > (*agentMap)[i].getID()) ){

				if(prob(beta)){ //with probability of beta, rewire
					
					//keep trying to add an edge from i to a random node j such that j is not i and there is not an edge between i and j already
					for(bool done = false; done != true;){
						ListGraph::NodeIt j = randomNode();

						if( (j != i) && (findEdge(g,i,j) == INVALID)){
							g.addEdge(i, j);
							neighbors((*agentMap)[i], (*agentMap)[j]);
							done = true;
							g.erase(e);
						}
					}
				}
			}

		}
	}


	assert(parallelFree(g));
	assert(connected(g));
}
void run::powerGraphSetup(){ 

	assert(cfg.N >= 3); //can't be too small

	//todo: track the nodes that have been connected so you can connect new nodes to them
	vector<ListGraph::NodeIt> addedNodes;
	//connect the first three nodes to each other completely
	int count = 0;
	for(ListGraph::NodeIt i(g); i != INVALID; ++i){
		
		//if you are agent 0, 1, or 2:
		if((*agentMap)[i].getID() == 0 || (*agentMap)[i].getID() == 1 || (*agentMap)[i].getID() == 2){
			addedNodes.push_back(i);
			//connect to agents 0, 1, and 2, but not yourself

			for(ListGraph::NodeIt j(g); j !=INVALID; ++j){
				if((*agentMap)[j].getID() == 0 || (*agentMap)[j].getID() == 1 || (*agentMap)[j].getID() == 2){
					if((*agentMap)[i].getID() != (*agentMap)[j].getID() && (findEdge(g,i,j) == INVALID)){
						g.addEdge(i,j);
						neighbors((*agentMap)[i], (*agentMap)[j]);
					}
				}
			}
		}
	}

	//add nodes one by one, attaching to other nodes preferentially

	for(ListGraph::NodeIt i(g); i!=INVALID; ++i){
		bool wasAdded = false;
		if((*agentMap)[i].getID() != 0 && (*agentMap)[i].getID() != 1 && (*agentMap)[i].getID() != 2){

			for(int j = 0; j < addedNodes.size(); j++){

				if(prob(double(getDegree(addedNodes.at(j)))/double(totalEdgeCount()))){
					g.addEdge(i,addedNodes.at(j));
					neighbors((*agentMap)[i], (*agentMap)[addedNodes.at(j)]);
					wasAdded = true;	
				}
			}
			
			if(wasAdded){
				addedNodes.push_back(i);
			}
			else{ //if the agent didn't manage to get added to anything, duct tape solution-- attach to one of the original 3 nodes
				g.addEdge(i,addedNodes.at(0));
				neighbors((*agentMap)[i], (*agentMap)[addedNodes.at(0)]);
			}
			wasAdded = false;
		}
	}


	//postconditions:
	assert(parallelFree(g));
	assert(connected(g));
}

void run::print(){ //outputs the graph as a GraphML file, so I can visually check correctness
	//cout<<"printing!"; 
	fstream fout("output.xml", fstream::out);
	fout<<"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n <graphml xmlns=\"http://graphml.graphdrawing.org/xmlns\"\nxmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\nxsi:schemaLocation=\"http://graphml.graphdrawing.org/xmlns\nhttp://graphml.graphdrawing.org/xmlns/1.0/graphml.xsd\">\n";
	fout<<"<graph id=\"G\" edgedefault=\"undirected\">\n";
	
	for(ListGraph::NodeIt i(g); i != INVALID; ++i){
		fout<<"<node id=\""<<(*agentMap)[i].getID() <<"\"/>"<<endl;
	}

	for(ListGraph::EdgeIt e(g); e != INVALID; ++e){
		fout<<"<edge source=\""<<(*agentMap)[g.u(e)].getID()<<"\" target=\""<<(*agentMap)[g.v(e)].getID()<<"\"/>"<<endl;
		
	}
	fout<<"</graph>\n </graphml>"<<endl;
}
ListGraph::NodeIt run::randomNode(){ //returns iterator to random node

	boost::uniform_int<> range(0,(cfg.N-1));
	boost::variate_generator<boost::mt19937&, boost::uniform_int<> > gen(prng, range);
	int x = gen();
	ListGraph::NodeIt toreturn = nodesList.at(x);
	assert(toreturn != INVALID);
	return toreturn;

	
}

void run::execute(){ 

	//don't think this needs to do all that much, except loop and call tick()
#ifdef DEBUGLOGS
	debugMRS();
#endif
	print();
	int lastTickTrades = 0;
	int curTickTrades = 0;

	do
	/*for(int i = 0; i <1000; i++)*/{ 
		lastTickTrades = curTickTrades;
		tick();  //do what the run does every time step
		if((stats.getTicks()%100) == 0){
			cout<<stats.getTicks()<<' '<<stats.getTrades()<<' '<<stats.getSearches()<<' '<<double(stats.getTrades())/double(stats.getSearches())<<' '<<totalUtility()<<' '<<double(utilityDecreasingTrades)/double(stats.getTrades())<<' ';
			if(cfg.N < 4){
				for(ListGraph::NodeIt i(g); i != INVALID; ++i){
					for(int j = 0; j < cfg.M; j++){
					cout<<(*agentMap)[i].getQuantity(j)<<' ';
					}
				}
			}
			cout<<endl;
		}
		curTickTrades = stats.getTrades();

		
	}
	while(/*atLeastOneCanTrade()*/curTickTrades > lastTickTrades); //currently, the only difference between these two is that it will take a few extra rounds of inactivity for all the agents to realize that they cannot gain from trade. Otherwise, results are identical in terms of searches, trades, and agent outcomes.
	
	createLog();
	cout<<"Done"<<endl;
	cout<<"Trades: "<<stats.getTrades()<<endl;
	cout<<"Searches: "<<stats.getSearches()<<endl;
	cout<<"Gini: "<<gini()<<endl;

	for(ListGraph::NodeIt i(g); i != INVALID; ++i){
		cout<<(*agentMap)[i].getID()<<' ';
	}
	cout<<endl;
	
#ifdef DEBUGLOGS
	debugMRS();
#endif
}

void run::tick(){
	
	vector<agent*> tempList = agentList;
	//ü
	//shuffle tempList:
	
	for(int i = tempList.size()-1; i > 0; i--){
		//random j in range 0 to i, inclusive
		boost::uniform_int<> range(0,i);
		boost::variate_generator<boost::mt19937&, boost::uniform_int<> > gen(prng, range);
		int j = gen();

		//swap
		agent * temp = tempList.at(i);
		tempList.at(i) = tempList.at(j);
		tempList.at(j) = temp;
	}

	//go through the agents and call tick() on them
	for(int i = 0; i <tempList.size(); i++){
		tempList.at(i)->tick();
		tempList.at(i)->hasActed = true; //this variable not used for anything as of V .2
	}

	//OBSOLETE V.1 version
	//todo: make sure each agent goes once and only once, in random order each time, and one search and potential initiated trade per agent per round
	//for(int i = 0; i < cfg.N; i++){ 
	//	agent* selected = wilhitePickNextAgent();//pick an agent
	//	selected->tick(); //call tick()
	//	selected->hasActed = true;
	//}
	
	stats.incTicks();

	//below is for debugging and such:
#ifdef DEBUGLOGS
	fstream fout("MRSovertime.log", ios_base::out | ios_base::app);
	
	for(ListGraph::NodeIt i(g); i != INVALID; ++i){
		pair<int, int> tempPair;
		tempPair = (*agentMap)[i].maxMRS();
		fout<<(*agentMap)[i].getMRS(tempPair.first,tempPair.second)<<'\t';
	}
	for(ListGraph::NodeIt i(g); i != INVALID; ++i){
		fout<<(*agentMap)[i].lastTradePrice<<'\t';
	}
	fout<<endl;

	fstream fout2("Uovertime.log", ios_base::out | ios_base::app);

	for(ListGraph::NodeIt i(g); i != INVALID; ++i){
		fout2<<(*agentMap)[i].getUtility()<<'\t';
	}
	fout2<<endl;

	fstream fout3("tradesovertime.log", ios_base::out | ios_base::app);

	for(ListGraph::NodeIt i(g); i != INVALID; ++i){
		fout3<<(*agentMap)[i].totalTradesInitiated<<'\t';
	}

	for(ListGraph::NodeIt i(g); i != INVALID; ++i){
		fout3<<double((*agentMap)[i].totalTradesInitiated)/double((*agentMap)[i].totalSearches)<<'\t';
	}
	fout3<<endl;

	fstream fout4("totaltradeparticipation.log", ios_base::out | ios_base::app);
	
	for(ListGraph::NodeIt i(g); i != INVALID; ++i){
		fout4<<(*agentMap)[i].totalTradesParticipatedIn<<'\t';
	}

	for(ListGraph::NodeIt i(g); i != INVALID; ++i){
		fout4<<double((*agentMap)[i].totalTradesParticipatedIn)/double((*agentMap)[i].totalSearches)<<'\t';
	}
	fout4<<endl;

	fstream fout5("quantitiesovertime.log", ios_base::out | ios_base::app);

	for(ListGraph::NodeIt i(g); i != INVALID; ++i){
		for(int j = 0; j < cfg.M; j++){
		fout5<<(*agentMap)[i].getQuantity(j)<<'\t';
		}
	}
	fout5<<endl;
#endif

	fstream fout6("indirectTradeQuantities.log", ios_base::out | ios_base::app);

	for(int i = 0; i < indirectTally.size(); i++){
		fout6<<indirectTally.at(i)<<'\t';
	}
	fout6<<endl;
}

agent* run::wilhitePickNextAgent(){//picks random agent to act next

	
	
	boost::uniform_int<> range(0,cfg.N-1);
	boost::variate_generator<boost::mt19937&, boost::uniform_int<> > gen(prng, range);
	int tempRandom = gen();
	int counter = 0;	
	for(ListGraph::NodeIt i(g); i != INVALID; ++i){
		
		
		if(counter==tempRandom){
			return &(*agentMap)[i];
			}
		counter++;
		
	}
	
	assert(1 == 2); //if we got down here, we didn't find an agent and this function is broken
}

bool run::atLeastOneCanTrade(){

	for(ListGraph::NodeIt i(g); i != INVALID; ++i){
		if((*agentMap)[i].canGainFromTrade)
			return true;
	}
	
	return false;
}

void run::debugMRS(){

	cout<<"MRS DEBUG: "<<endl;
	cout<<"------"<<endl;

	for(ListGraph::NodeIt i(g); i != INVALID; ++i){
		cout<<"ID: "<<(*agentMap)[i].getID()<<endl;
		cout<<"MRS debug: "<<(*agentMap)[i].getMRS(0,1)<<' '<<(*agentMap)[i].getMRS(1,0)<<endl;
		cout<<"Total utility: "<<(*agentMap)[i].getUtility()<<endl;
		cout<<"partial d of utility: "<<(*agentMap)[i].getDUtility(0)<<' '<<(*agentMap)[i].getDUtility(1)<<endl;
		cout<<"quantities: ";
		for(int j = 0; j < cfg.M; j++){
		cout<<(*agentMap)[i].getQuantity(j)<<' ';
		}
		cout<<endl;
		cout<<"can gain from trade: "<<(*agentMap)[i].canGainFromTrade<<endl;
		cout<<"number of neighbors: "<<(*agentMap)[i].numNeighbors()<<endl;
		cout<<"indirect trader: "<<(*agentMap)[i].indirectTrader<<endl;
		cout<<"---"<<endl;
	}

}

void run::createLog(){
	
	fstream fout(cfg.filename.c_str(), fstream::out | fstream::app);
	string fout2filename = cfg.filename + cfg.tString();
	fstream fout2(fout2filename.c_str(), fstream::out | fstream::app); //fstream for summary stats- for quick comparisons
	//parameters:
	fout<<"Model version: "<<getVer()<<endl;
	fout<<"Seed: "<<cfg.seed<<endl;
	fout<<"Agents: "<<cfg.N<<endl;
	fout<<"Goods: "<<cfg.M<<endl;
	fout<<"Network type: "<<cfg.tString()<<endl;
	fout<<"Indirect Trade: "<<cfg.indirectTrade<<endl;
	fout<<"--------------------"<<endl<<endl;
	fout<<"Summary Statistics: "<<endl;
	fout<<"Total trades: "<<stats.getTrades()<<endl;
	fout2<<stats.getTrades()<<'\t';
	fout<<"Total searches: "<<stats.getSearches()<<endl;
	fout2<<stats.getSearches()<<'\t';
	fout<<"Total trades / Total searches: "<<(double(stats.getTrades())/double(stats.getSearches()))<<endl;
	fout2<<(double(stats.getTrades())/double(stats.getSearches()))<<'\t';
	//get number of edges:
	int edgeCount = 0;
	for(ListGraph::EdgeIt i(g); i != INVALID; ++i){
		edgeCount++;
	}
	fout<<"Edges: "<<edgeCount<<endl;

	fout<<"Gini: "<<gini()<<endl;
	fout2<<gini()<<'\t';

	fout<<"Average utility: "<<totalUtility()/double(cfg.N)<<endl;
	fout2<<totalUtility()/double(cfg.N)<<'\t';
	fout<<"Average utility of direct traders: "<<avgUtilDirect()<<endl;
	fout2<<avgUtilDirect()<<'\t';
	fout<<"Average utility of indirect traders: "<<avgUtilIndirect()<<endl;
	fout2<<avgUtilIndirect()<<'\t';
	fout<<"Largest share of indirect trades: "<<greatestPercentOfIndirectTally()<<endl;
	fout2<<greatestPercentOfIndirectTally()<<'\t';
	fout<<"Largest share of all trades: "<<greatestPercentOfTotalTally()<<endl;
	fout2<<greatestPercentOfTotalTally()<<'\t';
	fout<<"--------------------------"<<endl<<endl<<endl;
	fout2<<endl;
	

	//make a detailed log of each agent's total utility and quantities of goods
	string detailedLog = cfg.filename + "_details.log";
	fstream details(detailedLog.c_str(), fstream::out);

	for(ListGraph::NodeIt i(g); i != INVALID; ++i){
		details<<(*agentMap)[i].getUtility()<<'\t';
		
		for(int j = 0; j < cfg.M; j++){
			details<<(*agentMap)[i].getQuantity(j)<<'\t';
		}

		details<<endl;
	}


}

long double run::totalUtility(){

	long double toReturn = 0;
	for(ListGraph::NodeIt i(g); i !=INVALID; ++i){
		toReturn += (*agentMap)[i].getUtility();
	}

	return toReturn;
}

double run::gini(){

	//step 1: get utility of all agents
	vector<double> utils;
	for(ListGraph::NodeIt i(g); i!= INVALID; ++i){
		utils.push_back((*agentMap)[i].getUtility());
	}
	
	//step 2: sort
	sort(utils.begin(), utils.end());
	
	//step 3: cumulative column
	vector<long double> cumulative(utils.size());

	for(int i = 0; i < cumulative.size(); i++){
		
		if(i == 0){
			cumulative.at(i) = utils.at(i);
		}
		else {
			cumulative.at(i) = (utils.at(i) + cumulative.at(i-1));
		}
	}
	//step 4: get T
	long double T = cumulative.at(cumulative.size()-1);

	//step 5: sum all but last value of cumulative
	long double sum = 0;

	for(int i = 0; i < cumulative.size() - 1; i++){
		sum += cumulative.at(i);
	}

	//step 6: return Gini Coefficient

	return 1 - ((((2 / T) * sum) + 1)/utils.size());
}

double run::getTallyTotal(){

	double toReturn = 0;

	for(int i = 0; i <tally.size(); i++){
		toReturn += tally.at(i);
	}

	return toReturn;
}

long double run::avgUtilIndirect(){ //returns avg utility of traders that use the indirect trade heuristic

	long double sum = 0;
	long double count = 0;
	for(ListGraph::NodeIt i(g); i != INVALID; ++i){
		if((*agentMap)[i].indirectTrader){
			sum += (*agentMap)[i].getUtility();
			count++;
		}
	}

	return sum/count;
}

double run::avgUtilDirect(){ //returns avg utility of traders that do NOT use the indirect trade heuristic

	double sum = 0;
	double count = 0;
	for(ListGraph::NodeIt i(g); i != INVALID; ++i){
		if(!(*agentMap)[i].indirectTrader){
			sum += (*agentMap)[i].getUtility();
			count++;
		}
	}

	return sum/count;
}

int run::getDegree(ListGraph::NodeIt in){

	int toReturn = 0;

	for(ListGraph::IncEdgeIt i(g, in); i != INVALID; ++i){
		toReturn++;
	}

	return toReturn;

}

int run::totalEdgeCount(){

	int toReturn = 0;

	for(ListGraph::EdgeIt i(g); i !=INVALID; ++i){
		toReturn ++;
	}

	return toReturn;
}
 

//returns the share of indirect trades of the most commonly indirectly traded good
double run::greatestPercentOfIndirectTally(){

	double max = indirectTally.at(0);
	double sum = indirectTally.at(0);
	//find max of indirectTally while summing it. Then return max/sum
	for(int i = 1; i < cfg.M; i++){
		if(indirectTally.at(i) > max)
			max = indirectTally.at(i);
		sum += indirectTally.at(i);
	}

	return max/sum;
}
	
//returns the share of total trades of the most often traded good
double run::greatestPercentOfTotalTally(){

	double max = tally.at(0);
	double sum = tally.at(0);

	for(int i = 1; i < cfg.M; i++){
		if(tally.at(i) > max)
			max = tally.at(i);
		sum += tally.at(i);
	}

	return max/sum;
}