compile:
	gcc -Wall -o eth-frame eth-frame.c

run: compile
	./eth-frame
