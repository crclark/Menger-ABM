objects = main.o run.o statsCollector.o agent.o

menger-abm : $(objects)
	g++ $(objects)

main.o : main.cpp run.h config.h statsCollector.h agent.h
	g++ -c -Wall main.cpp

run.o : run.cpp run.h agent.h config.h statsCollector.h
	g++ -c -Wall run.cpp

statsCollector.o : statsCollector.h
	g++ -c -Wall statsCollector.cpp

agent.o: config.h agent.h run.h util.h
	g++ -c -Wall agent.cpp

clean: 
	rm -rf $(objects)
