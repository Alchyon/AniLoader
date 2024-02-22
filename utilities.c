#include "libraries.h"
#include "utilities.h"

// Other normal function
void changelog () {
	printf("########################################################\n");
	printf("#                     "ANSI_COLOR_YELLOW" Changelog "ANSI_COLOR_RESET"                      #\n");
	printf("########################################################\n");
	printf("#                                                      #\n");
	printf("#"ANSI_COLOR_CYAN" 1.6 "ANSI_COLOR_RESET"                                                 #\n");
	printf("# - Implementata una funzionalita' che permette di     #\n");
	printf("#   aggiungere anime ai preferiti, in modo che questi  #\n");
	printf("#   siano controllati all'avvio del programma, per     #\n");
	printf("#   verificare la presenza di nuovi episodi che        #\n");
	printf("#   verranno scaricati in automatico nella cartella    #\n");
	printf("#   prestabilita.                                      #\n");
	printf("#   Utile per chi guarda gli anime in simulcast.       #\n");
	printf("#                                                      #\n");
	printf("#"ANSI_COLOR_CYAN" 1.7 "ANSI_COLOR_RESET"                                                 #\n");
	printf("# - Rimosso il sistema di cookie in quanto troppo      #\n");
	printf("#   hardcoded e, di conseguenza, poco efficace.        #\n");
	printf("# - Adesso la ricerca degli aggiornamenti per i nuovi  #\n");
	printf("#   episodi usciti, dovra' essere avviata manualmente  #\n");
	printf("#   dal classico menu' per prevenire possibili bug     #\n");
	printf("#   durante la lettura dei file.                       #\n");
	printf("# - Aggiunta la possibilita' di visualizzare ed        #\n");
	printf("#   eliminare tutti i preferiti attualmente salvati.   #\n");
	printf("#                                                      #\n");
	printf("#"ANSI_COLOR_CYAN" 1.7.1 "ANSI_COLOR_RESET"                                               #\n");
	printf("# - Risolti diversi problemi di input che portavano    #\n");
	printf("#   ad un errore nella prima ricerca effettuata.       #\n");
	printf("# - Risolto un problema che salvava i file degli       #\n");
	printf("#   anime preferiti con una directory erronea.         #\n");
	printf("# - Ottimizzato l'uso della memoria in modo che sia    #\n");
	printf("#   il meno esoso possibile.                           #\n");
	printf("# - Migliorie minori al codice sorgente.               #\n");
	printf("#                                                      #\n");
	printf("#"ANSI_COLOR_CYAN" 1.7.2 "ANSI_COLOR_RESET"                                               #\n");
	printf("# - Fixato un valore di return di una funzione.        #\n");
	printf("# - Fixato un input che richiedeva un INVIO di troppo. #\n");
	printf("# - Aggiornato il README.                              #\n");
	printf("#                                                      #\n");
	printf("########################################################\n");
}

long int findSize (char *file_name) {
	// Apro il file in lettura
	FILE *fp = fopen(file_name, "r");

	// Controllo se il file esiste
	if (fp == NULL) {
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

char *createPath (char *string) {
	char *path = (char *) calloc(sizeof(strlen(string)) + 100, sizeof(char));
	if (path == NULL) {
		perror("calloc");
		_exit(-2);
	}

	// Creazione path
	sprintf(path, BPATH "%s/Downloads/%s", getenv("USERNAME"), string);

	return path;
}

char *extractInMemoryFromFile (char *searchFilePath, bool del) {
	// Allocazione matrice
	int size = findSize(searchFilePath);
	char *searchFileData = (char *) calloc(size + 10, sizeof(char));
	if (searchFileData == NULL) {
		perror("calloc");
		_exit(-2);
	}

	// Apertura file
	FILE *search = fopen(searchFilePath, "r");
	if (search == NULL) {
		perror("malloc");
		_exit(-2);
	}

	// Lettura dell'intero file
	fread(searchFileData, size, 1, search);

	// Chiusura ed eliminazione
	fclose(search);
	
	// Controllo per l'eliminazione del file, aggiunto per CFU
	if (del)
		remove(searchFilePath);

	// Return dei dati
	return searchFileData;
}

char **createMatrixByEscapeCharacter (char *string, char *escape, int *line) {
	// Creo una copia per la variabile intera e creo la matrice
	int posix;
	char **searchDataResult = (char **) malloc(sizeof(char *) * strlen(string));
	if (searchDataResult == NULL) {
		perror("malloc");
		_exit(-2);
	}

	// Effettuo la copia token per token
	char *token = strtok(string, escape);

	for (posix = 0; token != NULL; posix++) {
		// Copio la stringa
		searchDataResult[posix] = (char *) calloc(strlen(token) + 1, sizeof(char));
		if (searchDataResult[posix] == NULL) {
			perror("calloc");
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

char *fixDirectoryName (char *name) {
	unsigned int i, k;
	char *fix = (char *) calloc(strlen(name), sizeof(char));

	for (i = 0, k = 0; i < strlen(name); i++) {
		switch(name[i]) {
			case '\\':	case '/':	case ':':	case '*':
			case '?':	case '\"':	case '<':	case '>':
			case '|':	case '{':	case '}':	case '~':
				break;

			default:	fix[k] = name[i];
						fix[++k] = '\0';
						break;
		}
	}

	return fix;
}