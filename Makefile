
CC=g++

default:: wand wall iif_test mcastsend_test

wand: wand.cpp
		${CC} -Wall -g $? -o $@

wall: wall.cpp
		${CC} -Wall -g $? -o $@

clean::
		rm -f wand wall iif_test

iif_test:: inetinterface.cpp iif_test.cpp
		${CC} -Wall -g $? -o $@

mcastsend_test:: inetinterface.cpp mcastsend_test.cpp udpmcastsender.cpp
		${CC} -Wall -g $? -o $@
