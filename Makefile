AniLoader: main.o function.o utilities.o cfu.o
	gcc -o AniLoader main.o function.o utilities.o cfu.o

main.o: main.c function.h utilities.h struct.h libraries.h
	gcc -c main.c

function.o: function.c function.h utilities.h struct.h libraries.h
	gcc -c function.c

utilities.o: utilities.c utilities.h struct.h libraries.h
	gcc -c utilities.c

cfu.o: CFU.c CFU.h function.h utilities.h libraries.h
	gcc -c cfu.c