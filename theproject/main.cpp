/*************************************************
Entry point of the program defined herein. 
All that main() needs to do is get a config file, and for every line, get the params of
the model and store them in config objects. Then, for each config object, instantiate a run
object and give it that config. Then run the run object. run should ONLY need to have its
execute function called-- all output etc. should be handled inside the object. Then, quit.
**************************************************/
#include "run.h"
#include "config.h"
#include "statsCollector.h"
#include "agent.h"
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_01.hpp>
#include <iostream>
#include <iomanip>
#include <time.h>
#include <boost/program_options.hpp>
#include <string>
#include <stdlib.h> 
namespace po = boost::program_options;
using namespace std;

topology intToTopo(int in);

int main(int ac, char* av[]){
	int mainN;
	int mainM;
	int mainSeed;
	int mainTopology;
	int mainRuns;
	string mainFilename;
	bool mainIndirectTrade;
	double mainIndirectTraderPercentage;
	double mainIndirectExchangeModeProb;
	double mainIndirectPrefMult;

	//todo: low priority, but, if possible, scrap this lame command line system
	//and use the boost one commented out below. Not using Boost currently because
	//of weird errors.
	if(ac < 10){
		cout<<"command line args wrong";
		return 1;
	}
	mainM = atoi(av[1]);
	mainN = atoi(av[2]);
	mainSeed = atoi(av[3]);
	mainTopology = atoi(av[4]);
	mainRuns = atoi(av[5]);
	mainFilename = av[6];
	mainIndirectTrade = bool(atoi(av[7]));
	mainIndirectTraderPercentage = atof(av[8]);
	mainIndirectExchangeModeProb = atof(av[9]);
	mainIndirectPrefMult = atof(av[10]);
	/*try {

        po::options_description desc("Allowed options");
        desc.add_options()
            ("help", "produce help message")
            ("goods", po::value<int>(), "set number of goods")
			("agents", po::value<int>(), "set number of agents")
			("seed", po::value<int>(), "set initial value of random seed")
			("topology", po::value<int>(), "set topology of trade network. They are numbered from 0 to 6 in the following order: complete, star, ring, grid, tree, smallWorld, power")
			("runs", po::value<int>(), "number of runs to do for these settings (seed will be varied each run)")
			("file", po::value<string>(), "file to output results to.")
        ;

        po::variables_map vm;        
        po::store(po::parse_command_line(ac, av, desc), vm);
        po::notify(vm);    

        if (vm.count("help")) {
            cout << desc << "\n";
            return 1;
        }

        if (vm.count("goods")) {
            mainM = vm["goods"].as<int>();
        } else {
            cout << "Number of goods not specified\n";
			return 1;
        }
		if (vm.count("agents")){
			mainN = vm["agents"].as<int>();
		} else {
			cout<< "Number of agents not specified \n";
			return 1;
		}
		if (vm.count("seed")){
			mainSeed = vm["seed"].as<int>();
		} else {
			cout<< "seed not specified. assuming default seed-- 1337. \n";
			return 1;
		}
		if (vm.count("topology")){
			mainTopology = vm["topology"].as<int>();
		} else {
			cout<<"topology not specified \n";
			return 1;
		}
		if (vm.count("runs")){
			mainRuns = vm["runs"].as<int>();
		} else {
			cout<<"number of runs not specified \n";
			return 1;
		}
		if (vm.count("file")){
			mainFilename = vm["file"].as<string>();
		} else {
			cout<<"output filename not specified. \n";
			return 1;
		}
    }
    catch(exception& e) {
        cerr << "error: " << e.what() << "\n";
        return 1;
    }
    catch(...) {
        cerr << "Exception of unknown type!\n";
		return 1;
    }*/

	for(int i = 0; i <mainRuns; i++){
		config loopCfg;
		loopCfg.M = mainM;
		loopCfg.N = mainN;
		loopCfg.seed = mainSeed;
		loopCfg.T = intToTopo(mainTopology);
		loopCfg.filename = mainFilename;
		loopCfg.indirectTrade = mainIndirectTrade;
		loopCfg.indirectTraderPercentage = mainIndirectTraderPercentage;
		loopCfg.indirectExchangeModeProb = mainIndirectExchangeModeProb;
		loopCfg.indirectPrefMult = mainIndirectPrefMult;

		run loopRun(loopCfg);
		loopRun.execute();
		mainSeed++;
	}
	
	/*time_t start,end;
	time (&start);
	cout << setiosflags(ios::fixed) << setprecision(2);
	config Nellie;
	Nellie.M = 2;
	Nellie.N = 10;
	Nellie.seed = 1337;
	Nellie.T = complete;
	run Georg(Nellie);
	Georg.execute();
	time (&end);

	cout<<"Rounds: "<<Georg.stats.getTicks()<<endl;
	cout<<"Trades: "<<Georg.stats.getTrades()<<endl;
	cout<<"Searches: "<<Georg.stats.getSearches()<<endl;
	cout<<"Time: "<<difftime(end,start)<<endl;
	*/
	
	return 0;

}

topology intToTopo(int in) { //complete, star, ring, grid, tree, smallWorld, power

	switch (in){

		case 0:
			return complete;
			break;
		case 1:
			return star;
			break;
		case 2:
			return ring;
			break;
		case 3:
			return grid;
			break;
		case 4:
			return tree;
			break;
		case 5:
			return smallWorld;
			break;
		case 6:
			return power;
			break;
	}

	return complete;
}