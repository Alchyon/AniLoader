#include "libraries.h"
#include "utilities.h"

// Other normal functions
void changelog () {
	printf("Il "ANSI_COLOR_YELLOW"changelog"ANSI_COLOR_RESET" piu' recente e' visionabile su " ANSI_COLOR_CYAN "GitHub" ANSI_COLOR_RESET ". \n");
	printf("Tieni premuto CTRL mentre premi sul link sottostante per aprire la pagina web corrispondente: \n");
	printf(ANSI_COLOR_CYAN "https://github.com/Alchyon/AniLoader/releases/latest " ANSI_COLOR_RESET " \n\n");
}

long int findSize (char *file_name) {
	// Apro il file in lettura
	FILE *fp = fopen(file_name, "r");

	// Controllo se il file esiste
	if (fp == NULL) {
		perror("fopen");
		exit(-1);
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
		exit(2);
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
		exit(2);
	}

	// Apertura file
	FILE *search = fopen(searchFilePath, "r");
	if (search == NULL) {
		perror("fopen");
		exit(2);
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
		exit(2);
	}

	// Effettuo la copia token per token
	char *token = strtok(string, escape);

	for (posix = 0; token != NULL; posix++) {
		// Copio la stringa
		searchDataResult[posix] = (char *) calloc(strlen(token) + 1, sizeof(char));
		if (searchDataResult[posix] == NULL) {
			perror("calloc");
			exit(2);
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
	char *fix = (char *) calloc(strlen(name) + 1, sizeof(char));

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

// # Funzioni di libreria aggiunte manualmente
char *strndup(const char *s, size_t n) {
	char *p;
	size_t n1;

	for (n1 = 0; n1 < n && s[n1] != '\0'; n1++)
		continue;

	p = malloc(n + 1);
	if (p != NULL) {
		memcpy(p, s, n1);
		p[n1] = '\0';
	}

	return p;
}