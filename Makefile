
CC=g++

wand:: wand.cpp
		${CC} -Wall $? -o $@


clean::
		rm -f wand
