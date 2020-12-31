AniLoader: function.o utilities.o
	gcc -o AniLoader function.o utilities.o

function.o: function.c function.h utilities.h struct.h libraries.h
	gcc -c function.c

utilities.o: utilities.c utilities.h struct.h libraries.h
	gcc -c utilities.c