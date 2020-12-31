#include "libraries.h"
#include "utilities.h"

// For custom getch() only on Linux OS
#ifdef __unix__
	char getch(void)
	{
		char buf = 0;
		struct termios old = {0};
		fflush(stdout);

		if (tcgetattr(0, &old) < 0)
			perror("tcsetattr()");

		old.c_lflag &= ~ICANON;
		old.c_lflag &= ~ECHO;
		old.c_cc[VMIN] = 1;
		old.c_cc[VTIME] = 0;

		if (tcsetattr(0, TCSANOW, &old) < 0)
			perror("tcsetattr ICANON");

		if (read(0, &buf, 1) < 0)
			perror("read()");

		old.c_lflag |= ICANON;
		old.c_lflag |= ECHO;

		if (tcsetattr(0, TCSADRAIN, &old) < 0)
			perror("tcsetattr ~ICANON");

		//	Uncomment for getche()
		//	printf("%c\n", buf);

		return buf;
	}
#endif

// Trasformo getlogin() della unistd.h usata in __unix__ in una Windows API ( GetUserName(char *, DWORD *) )
#ifdef _WIN32
	char *getlogin()
	{
		DWORD us = UNLEN + 1;
		char *username = (char *)malloc(sizeof(char) * 30);
		if (username == NULL) {
			perror("malloc");
			_exit(-2);
		}

		GetUserName(username, &us);
		return username;
	}
#endif

// Other normal function
void changelog()
{
	system(clearScreen);
	printf("########################################################\n");
	printf("#                       Changelog                      #\n");
	printf("########################################################\n");
	printf("#                                                      #\n");
	printf("# 1.0                                                  #\n");
	printf("# - Initial commit                                     #\n");
	printf("#                                                      #\n");
	printf("########################################################\n");
	printf("Premere un tasto per continuare. . .");
	getch();
}

long int findSize(char file_name[])
{
	// Apro il file in lettura
	FILE *fp = fopen(file_name, "r");

	// Controllo se il file esiste
	if (fp == NULL)
	{
		perror("fopen");
		_exit(-1);
	}

	// Metto il puntatore alla fine del file
	fseek(fp, 0L, SEEK_END);

	// Calcolo la dimensione del file
	long int res = ftell(fp);

	// Chiudo il file
	fclose(fp);

	return res;
}

char *createPath(char string[])
{
	char user[50];
	strcpy(user, getlogin());

	char *path = (char *)malloc(sizeof(char) * (sizeof(user) + sizeof(strlen(string)) + 50));
	if (path == NULL)
	{
		perror("malloc");
		_exit(-2);
	}

	// Creazione path in base all'OS
	#ifdef __unix__
		sprintf(path, BPATH "%s/Scaricati/%s", user, string);
	#elif _WIN32
		sprintf(path, BPATH "%s/Downloads/%s", user, string);
	#endif

	return path;
}

char *extractInMemoryFromFile(char searchFilePath[])
{
	char *check;

	// Allocazione matrice
	int size = findSize(searchFilePath);
	char *searchFileData = (char *)malloc(sizeof(char) * (size + 1));
	if (searchFileData == NULL)
	{
		printf("Allocazione fallita\n");
		_exit(-2);
	}

	// Apertura file
	FILE *search = fopen(searchFilePath, "r");
	if (search == NULL)
	{
		printf("Errore nell'apertura del file\n");
		_exit(-2);
	}

	// Lettura dell'intero file
	fread(searchFileData, size, 1, search);

	// Chiusura ed eliminazione
	fclose(search);
	int i = remove(searchFilePath);

	// Return dei dati
	return searchFileData;
}

char **createMatrixByEscapeCharacter(char string[], char escape[], int *line)
{
	// Creo una copia per la variabile intera e creo la matrice
	int posix = *line;
	char **searchDataResult = (char **)malloc(sizeof(char *) * strlen(string));
	if (searchDataResult == NULL)
	{
		perror("malloc");
		_exit(-2);
	}

	// Effettuo la copia token per token
	char *token = strtok(string, escape);

	for (posix = 0; token != NULL; posix++)
	{
		// Copio la stringa
		searchDataResult[posix] = (char *)malloc(sizeof(char) * (strlen(token) + 1));
		if (searchDataResult[posix] == NULL)
		{
			perror("malloc");
			_exit(-2);
		}

		strcpy(searchDataResult[posix], token);

		// Genero il prossimo token
		token = strtok(NULL, escape);
	}

	// Associo il valore intero usato al numero delle righe
	*line = posix;

	return searchDataResult;
}
