
CC=g++

default:: wand wall

wand: wand.cpp
		${CC} -Wall -g $? -o $@

wall: wall.cpp
		${CC} -Wall -g $? -o $@

clean::
		rm -f wand wall
