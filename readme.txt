PRELIMINARY README

Context and authorship:
----------------------
This project was originally created over the course of the 2010-2011 school year at Grove City College as an independent research project supervised by Dr. Jeremy DalleTezze, who also assisted with the microeconomic math and reasoning through the model's behavior, as well as significantly guiding the pace and direction of the investigation.

Concept, design, and all code by Connor Clark.

Overview:
---------

The goal of this project is to implement a barter economy in an agent-based model, and to add some heuristic to the agents' trading strategy similar to that described by Carl Menger in "On the Origins of Money". (Plain text translation to English here:
http://socserv.mcmaster.ca/~econ/ugcm/3ll3/menger/money.txt). The differences in allocative efficiency and individual agent outcomes are to be assessed with respect to a naive barter trading strategy versus a more informed, speculative heuristic similar to the one described by Menger. These differences are also to be assessed across different trade network topologies.

The overall structure of the model is heavily inspired by Wilhite 2006, with the addition of an arbitrary number of goods and the roughly Mengerian trade heuristic.

Agents are connected in a trade network, where an edge between two agents indicates their ability to trade with each other. At each iteration of the model, the agents take turns in random order to initiate trades with those around them, selecting the best deal based on their endowments and preferences. They follow a simple Cobb-Douglas utility function where all goods have an exponent of 1. The model stops when all agents are at consumer equilibrium or cannot find trade partners.

Initial endowments are such that agents need each other to maximize their utility. Each agent is endowed with a large amount of one good and a small amount of all other goods. These different endowments are scattered non-randomly across the network (if they were randomly distributed, the effect of a network on allocative efficiency would be obscured).

The heuristic currently being experimented with is to have the agents, with a given probability, weight their utility function for deciding who to trade with in a given turn. This weighting makes goods that the agent has seen trade more often (currently, if your neighbor trades in that good, you have seen that good be traded) more desirable. After trading with that weighted utility function, the agent's utility function returns to normal so that he can sell off excess goods that he doesn't really want. The idea is that this works as an indirect exchange similar to Menger's theory of money evolving out of more marketable goods. Attempting to buy up goods that you don't want but that appear to be in demand is inherently speculative, of course, and the question is how individual agents perform when they try this strategy. The current goal is to determine what parameter values for this strategy are best. The strength of the weighting and the frequency with which the agents use the weighted utility function are currently the only parameters.

Technical stuff:
---------------
This code depends on Boost 1.38 and LEMON 1.2.1. While it has only been compiled on Windows, I am not aware of any particular difficulties that would prevent it from easily being ported to any OS.

Using:
------
The command line arguments necessary to run the model are all numerical. They are as follows:

1. Number of goods. 
2. Number of agents.
3. Seed for the PRNG
4. Topology of the trade network. They are numbered from 0 to 6 in the following order: complete, star, ring, grid, tree, smallWorld, power. See Wilhite 2006 for a description of each. NOTE tree is currently broken.
5. Number of times to run the model. All other settings remain the same, except the random seed, which is incremented by 1 with each run. This is so data can be averaged across runs for better reporting.
6. filename to base the primary log file on.
7. bool (0 or 1) turning indirect trade (i.e. the Menger heuristic) on or off.
8. the share of the traders who will be allowed to use the Menger heuristic. range 0 (none) to 1.0 (all).
9. First parameter for the heuristic -- the probability a trader will use the heuristic on a given turn.
10. Second parameter for the heuristic -- the multiplier to weight the trader's preferences when using the heuristic. In this case, the exponents for the goods in the Cobb-Douglas function are 1 + this parameter * (number of times  you have seen this good traded/number of times you have seen all goods traded)

Thus, running this with:

theproject.exe 2 50 1337 2 10 example 1 .8 0.1 1.0

will do 10 runs of the model with 2 goods, 50 agents, with the trade heuristic turned on and 80% of agents using it 10% of the time and weighting their exponents by the share of each good's transactions in the total number of transactions.

Logs:
-----
are tab-separated columns that can only really be interpreted by reading the source code. Some logs can be turned on and off and give per-agent information every time step. The main log, yourlognamehere.log gives human-readable overviews of each run. yourlognamehere.log_details gives the end state of each agent for the last run ran. The most useful log is named after the topology used. e.g. yourlognamehere.logstar. Each row is a single run's results. The columns from left to right are:

-total number of trades
-total number of searches for trade partners
-trades/searches
-the gini coefficient
-average utility of the agents
-average utility of the agents who do not use the Menger heuristic
-average utility of the agents who use the Menger heuristic
-the share of heuristic-based trades of the most frequently heuristic-traded good out of the total number of heuristic-based trades
-the share of trades of the good that was most used in all trades

Short bibliography of stuff helpful for the coding of the model:
-------------------

Menger, Carl. 2009. On the Origins of Money. Auburn, Alabama: Ludwig von Mises Institute.


Rothbard, Murray N. 2004. Man, Economy, and State. In Man, Economy, and State with Power and Market. 2nd ed. Ludwig von Mises Institute.


Wilhite, A. 2001. “Bilateral trade and ‘small-world’networks.” Computational Economics 18 (1): 49–64.


———. 2006. “Economic activity on fixed networks.” Handbook of computational economics 2: 1013–1045.
