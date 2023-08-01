#include "libraries.h"
#include "utilities.h"

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
	printf("# 1.1                                                  #\n");
	printf("# - Risolto un problema con alcuni nomi di directory   #\n");
	printf("# - Aggiunto un nuovo metodo di richiesta HTTP che     #\n");
	printf("#   utilizza i cookie per poter accedere al sito       #\n");
	printf("#   anche in caso di protezione attiva                 #\n");
	printf("#                                                      #\n");
	printf("# 1.2                                                  #\n");
	printf("# - Risolto un problema che faceva crashare il         #\n");
	printf("#   programma se erano presenti alcuni caratteri       #\n");
	printf("#   speciali nel nome dell'anime cercato               #\n");
	printf("#                                                      #\n");
	printf("# 1.4.1                                                #\n");
	printf("# - Link fixed                                         #\n");
	printf("#                                                      #\n");
	printf("# 1.4.2                                                #\n");
	printf("# - Cookie fixed                                       #\n");
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
	sprintf(path, BPATH "%s/Downloads/%s", user, string);

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
		perror("malloc");
		_exit(-2);
	}

	// Apertura file
	FILE *search = fopen(searchFilePath, "r");
	if (search == NULL)
	{
		perror("malloc");
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
	int posix;
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

char* fixDirectoryName(char* name) {
	int i, k;
	char* fix = (char *) malloc(sizeof(char) * strlen(name));

	for (i = 0, k = 0; i < strlen(name); i++) {
		switch(name[i]) {
			case '\\':	case '/':	case ':':	case '\'':
			case '?':	case '"':	case '<':	case '>':
			case '|':
				break;
			
			default:
				fix[k] = name[i];
				k++;
		}
	}

	return fix;
}