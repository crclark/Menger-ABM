menger-abm : main.o run.o statsCollector.o agent.o
	g++ main.o run.o statsCollector.o agent.o -o menger-abm

main.o : main.cpp run.h config.h statsCollector.h agent.h
	g++ -c main.cpp

run.o : run.cpp run.h agent.h config.h statsCollector.h
	g++ -c run.cpp

statsCollector.o : statsCollector.h
	g++ -c statsCollector.cpp

agent.o: config.h agent.h run.h util.h
	g++ -c agents.cpp
