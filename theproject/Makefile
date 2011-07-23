OBJS = main.o run.o statsCollector.o agent.o
CC = g++
CFLAGS = -Wall -c
LFLAGS = -Wall

menger-abm : $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o menger-abm

main.o : main.cpp run.h config.h statsCollector.h agent.h
	$(CC) $(CFLAGS) main.cpp

run.o : run.cpp run.h agent.h config.h statsCollector.h
	$(CC) $(CFLAGS) run.cpp

statsCollector.o : statsCollector.cpp statsCollector.h
	$(CC) $(CFLAGS) statsCollector.cpp

agent.o: agent.cpp config.h agent.h run.h util.h
	$(CC) $(CFLAGS) agent.cpp

clean: 
	rm -rf $(OBJS)
