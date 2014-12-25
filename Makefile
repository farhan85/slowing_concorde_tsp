#
# A simple Makefile for the Final Year (Software) Engineering Project -- Slowing Concorde,
# completed by Farhan Ahammed (farhan.ahammed@studentmail.newcastle.edu.au), 2007
#



CC = gcc -m32



##################################################################################################################
# Executables
##################################################################################################################

all:
	make evoalg
	make agent
	make analysels
	make ls2tsp
	make clean


evoalg: ./memetic/evoalg.c ./ls/ls.o ./ls/ls_pop.o ./util/upper_bound.o ./mt19937ar/t_mt19937ar.o ./lsys.o ./sortalg/mergesort_ls.o
	$(CC) -lm -o ./bin/evoalg.exe ./memetic/evoalg.c ./ls/ls.o ./ls/ls_pop.o ./util/upper_bound.o ./mt19937ar/t_mt19937ar.o ./lsys.o ./sortalg/mergesort_ls.o

agent: ./memetic/agent.c ./lsys.o ./util/upper_bound.o ./mt19937ar/t_mt19937ar.o ./ls/ls.o
	$(CC) -lm -o ./bin/agent.exe ./memetic/agent.c ./lsys.o ./util/upper_bound.o ./mt19937ar/t_mt19937ar.o ./ls/ls.o

analysels: ./util/analyse_ls.c ./lsys.o ./util/upper_bound.o ./mt19937ar/t_mt19937ar.o
	$(CC) -lm -o ./bin/analysels.exe ./util/analyse_ls.c ./lsys.o ./util/upper_bound.o ./mt19937ar/t_mt19937ar.o

ls2tsp: ./util/ls2tsp.c ./lsys.o ./mt19937ar/t_mt19937ar.o
	$(CC) -lm -o ./bin/ls2tsp.exe ./util/ls2tsp.c ./lsys.o ./mt19937ar/t_mt19937ar.o

tspsol2mp: ./util/tspsol2mp.c
	$(CC) -o ./bin/tspsol2mp.exe ./util/tspsol2mp.c

##################################################################################################################



##################################################################################################################
# Libraries
##################################################################################################################

./ls/ls_pop.o: ./ls/ls_pop.c ./ls/ls_pop.h
	$(CC) -o ./ls/ls_pop.o -c ./ls/ls_pop.c

./ls/ls.o: ./ls/ls.c ./ls/ls.h
	$(CC) -o ./ls/ls.o -c ./ls/ls.c

./util/upper_bound.o: ./util/upper_bound.c ./util/upper_bound.h
	$(CC) -o ./util/upper_bound.o -c ./util/upper_bound.c

./sortalg/mergesort_ls.o: ./sortalg/mergesort_ls.c ./sortalg/mergesort_ls.h
	$(CC) -o ./sortalg/mergesort_ls.o -c ./sortalg/mergesort_ls.c

./mt19937ar/t_mt19937ar.o: ./mt19937ar/t_mt19937ar.c ./mt19937ar/t_mt19937ar.h
	$(CC) -o ./mt19937ar/t_mt19937ar.o -c ./mt19937ar/t_mt19937ar.c

./lsys.o: ./lsys.c
	$(CC) -o ./lsys.o -c ./lsys.c

##################################################################################################################



.PHONY: clean
clean:
	-rm -f ./bin/*.stackdump ./memetic/*.o ./ls/*.o ./sortalg/*.o ./util/*.o ./mt19937ar/*.o ./*.o
