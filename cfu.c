// This file will be used only for CFU (Check for Updates) codes.
// Il will keep everything inside for dividing the main code from the "DLC".

#include "libraries.h"
#include "function.h"
#include "utilities.h"
#include "cfu.h"

void CheckForUpdatesRoutine() {
	const char *appdata = getenv("APPDATA");

	// Steps:
	//	1:	Read summary
	//	2:  Variables assignments for step 3
	//	3:	foreach summaryData entried
	//	4:	calls to function to download, based on data

	// #1
	char *summaryFile = (char *) malloc(sizeof(char) * (strlen(appdata) + 50));
	if (summaryFile == NULL) {
		perror("malloc");
		_exit(-2);
	}

	sprintf(summaryFile, "%s\\AniLoader\\_data.summary", appdata);

	int summaryLine = 0;
	char **summaryData = createMatrixByEscapeCharacter(extractInMemoryFromFile(summaryFile, false), "\n", &summaryLine);

	// #3
	for (int i = 0; i < summaryLine; i++) {
		// Variabile del nome richiesta
		char *name = (char *) malloc(sizeof(char) * (strlen(summaryData[i]) + 10));
		if (name == NULL) {
			perror("malloc");
			_exit(-2);
		}

		strcpy(name, summaryData[i]);
		int k;
		for(k = strlen(name); name[k] != '.'; k--);
		name[k] = '\0';

		// Read from file, no reason to allocate, elaborated one by one.
		char *filePath = (char *) malloc(sizeof(char) * (strlen(summaryData[i]) + strlen(appdata) + 50));
		if (filePath == NULL) {
			perror("malloc");
			_exit(2);
		}

		// Path del singolo file, verra' resettato ad ogni iterazione
		sprintf(filePath, "%s\\AniLoader\\%s", appdata, summaryData[i]);

		// Considerato che extractInMemoryFromFile() ha una _exit() in caso di file non esistente, verifico adesso che
		// esista e chiamo un break in caso negativo
		FILE *f = fopen(filePath, "r");
		if (f == NULL)
			continue;

		// Lettura dati dal file, metodo senza struct
		int dataLine = 0;
		char **fileAnimeData = createMatrixByEscapeCharacter(extractInMemoryFromFile(filePath, false), "\n", &dataLine);

		// Ora partono le chiamate al main code per riutilizzare le funzioni, alcune strutture verranno ricreate in maniera
		// automatica in quanto i parametri sono gia' presenti nel file di salvataggio (... .cfu)
		
		// Creo il comando per scaricare la pagina corretta e lo eseguo
		cookie biscottino = { 0, "" };
		char *path = createPath(downloadCorrectPage(biscottino, fileAnimeData[2]));

		// Ottengo il contenuto della pagina dal file ed elimino lo stesso
		char *pageContent = extractInMemoryFromFile(path, true);
		free(path);

		// Trasformo l'array in una matrice
		int line = 0;
		char **pageDataResult = createMatrixByEscapeCharacter(pageContent, "\n", &line);

		// Ottengo una struttura contenente le estensioni dei singoli episodi e il numero delle stesse
		animeEpisodeData *lastData = getEpisodeExtension(pageDataResult, line);
		free(pageDataResult);

		if (lastData->numberOfEpisode == -1)
			// Errore, nessun episodio trovato
			// Skippo questo anime e passo a quello dopo
			continue;

		// Controllo episodi usciti
		int nEpi = atoi(fileAnimeData[3]);
		if (nEpi >= lastData->numberOfEpisode)
			continue;

		// Creazione struct dati per downloadFile()
		downloadOption *dwlOpt = (downloadOption *) malloc(sizeof(downloadOption));
		if (dwlOpt == NULL) {
			perror("malloc");
			_exit(-2);
		}

		// Numero anime da scaricare
		if (nEpi == 0)
			// Li scarico tutti
			dwlOpt->option = 0;
		else if (lastData->numberOfEpisode - nEpi == 1) {
			dwlOpt->option = 1;
			dwlOpt->singleEpisode = nEpi;
		}
		else {
			dwlOpt->option = 2;
			dwlOpt->firstEpisode = nEpi;
			dwlOpt->secondEpisode = lastData->numberOfEpisode - 1;
		}

		// Alloco il puntatore del nome
		dwlOpt->nameEpisode = (char *) malloc(sizeof(char) * 101);
		if (dwlOpt->nameEpisode == NULL) {
			perror("malloc");
			_exit(-2);
		}

		// Alloco il puntatore della directory
		dwlOpt->downloadDirectory = (char *) malloc(sizeof(char) * 501);
		if (dwlOpt->downloadDirectory == NULL) {
			perror("malloc");
			_exit(-2);
		}

		// Directory
		strcpy(dwlOpt->downloadDirectory, fileAnimeData[4]);
		// Nome episodi
		dwlOpt->nameEpisodeChange = true;
		strcpy(dwlOpt->nameEpisode, fileAnimeData[5]);

		// Devo crearmi una copia per fileAnimeData[2] perchè downloadPrepare() la sovrascrive
		char *copy = (char *) malloc(sizeof(char) * (strlen(fileAnimeData[2]) + 1));
		if (copy == NULL) {
			perror("malloc");
			_exit(-2);
		}
		strcpy(copy, fileAnimeData[2]);

		// Struct pronta, chiamata a funzione e terminazione del codice
		downloadPrepare(lastData, dwlOpt, biscottino, copy, name, 0, NULL);

		// Aggiornamento del file .cfu
		// Per comodita', delete into recreate
		remove(filePath);
		f = fopen(filePath, "w");

		fprintf(f, "6\n%s\n%s\n%d\n%s\n%s\n", fileAnimeData[1], fileAnimeData[2], lastData->numberOfEpisode, fileAnimeData[4], fileAnimeData[5]);
		fclose(f);

		free(name);
		free(filePath);
		free(fileAnimeData);
		free(dwlOpt);
		free(copy);
	}
	
	// NOTA: NESSUNA funzione che chiamo deve in nessun modo o caso richiamare il main() in quanto andrebbe a creare un loop
	//		 infinito che sovrascrive il codice
}