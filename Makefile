
CC=g++

default:: wand wall iif_test

wand: wand.cpp
		${CC} -Wall -g $? -o $@

wall: wall.cpp
		${CC} -Wall -g $? -o $@

clean::
		rm -f wand wall iif_test

iif_test: inetinterface.cpp iif_test.cpp
		${CC} -Wall -g $? -o $@
