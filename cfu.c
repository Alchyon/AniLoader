// This file will be used only for CFU (Check for Updates) codes.
// Il will keep everything inside for dividing the main code from the "DLC".

// NOTA: NESSUNA funzione che chiamo deve in nessun modo o caso richiamare il main()!

#include "libraries.h"
#include "function.h"
#include "utilities.h"
#include "cfu.h"

void addOnLoad (char *name, char *pageDirectLink, char *ext, char *downloadDirectory, char *nameEpisode) {
	// Si gestisce l'aggiunta delle informazioni dell'anime selezionato alla lista degli anime che vengono controllati on-boot.
	// Se sono qui, teoricamente ho gia' i dati richiesti per la creazione del file, quindi non dovrebbero servire ulteriori modifiche, al massimo una veloce
	// manipolazione di stringhe su qualche info, che pero' non viene piu' riutilizzata al termine di questa funzione, in quanto void!
	
	// Step 1:
	// 	- Creazione della cartella "AniLoader" in "... AppData/Roaming"
	char *appdataFolder = (char *) calloc(200, sizeof(char));
	if (appdataFolder == NULL) {
		perror("calloc");
		_exit(-2);
	}
	
	sprintf(appdataFolder, "mkdir \"%s/AniLoader\"", getenv("APPDATA"));
	system(appdataFolder);
	system(clearScreen);

	// Step 2:
	//	- Controllare se l'anime e' gia' presente nei preferiti
	// Per fare questo, si usa un formato standard di nomi, ovvero nomeAnime.cfu, questo file verra' letto e, se il controllo da esito positivo, verra' chiesto se
	// eliminarlo, sovrascriverlo o riavviare il programma.
	char *fileName = (char *) calloc(strlen(appdataFolder) + strlen(name) + 50, sizeof(char));
	if (fileName == NULL) {
		perror("calloc");
		_exit(-2);
	}

	// Creazione stringa per il file dell'anime corrente
	sprintf(fileName, "%s/AniLoader/%s.cfu", getenv("APPDATA"), fixDirectoryName(name));
	free(appdataFolder);

	FILE *f = fopen(fileName, "r");
	if (f != NULL) {
		printf(ANSI_COLOR_RED "Errore: questo anime e' gia' stato aggiunto ai preferiti!\n" ANSI_COLOR_RESET);
		system("pause");

		// Riavvio al main pulendo le variabili usate
		fclose(f);
		free(fileName);

		main();
	}
	
	// Se sono qui significa che il file NON esiste e che quindi puo' essere creato e scritto
	// Step 3:
	//	- Creazione del file per CFU
	f = fopen(fileName, "w");

	// Step 4:
	//	- Scrittura dati su file anime
	fprintf(f, "6\n%s\n%s%s\n0\n%s\n%s\n", pageDirectLink, pageDirectLink, ext, downloadDirectory, nameEpisode);
	fclose(f);
	
	// Step 5:
	//	- Scrittura nome su file di elenco
	fileName[0] = '\0';
	sprintf(fileName, "%s/AniLoader/_data.summary", getenv("APPDATA"));
	
	f = fopen(fileName, "a");
	if (f != NULL) {
		// Aggiornamento elenco
		fprintf(f, "%s.cfu\n", fixDirectoryName(name));
		fclose(f);
	}

	free(fileName);

	printf(ANSI_COLOR_GREEN "Anime aggiunto correttamente alla lista dei preferiti.\n" ANSI_COLOR_RESET);
	system("pause");
}

void CheckForUpdatesRoutine () {
	// Steps:
	//	1:	Read summary (getLibrary())
	//	2:  Variables assignments for step 3 (getLibrary())
	//	3:	foreach summaryData entries
	//	4:	calls to function to download, based on data
	//  5:	update .cfu file
	const char *appdata = getenv("APPDATA");

	int summaryLine = 0;
	char **summaryData = getLibrary(&summaryLine);

	// #3
	for (int i = 0; i < summaryLine; i++) {
		// Read from file, no reason to allocate, elaborated one by one.
		char *filePath = (char *) calloc(strlen(summaryData[i]) + strlen(appdata) + 50, sizeof(char));
		if (filePath == NULL) {
			perror("calloc");
			_exit(2);
		}

		// Path del singolo file, verra' resettato ad ogni iterazione, .cfu aggiunto hardcoded
		sprintf(filePath, "%s\\AniLoader\\%s.cfu", appdata, summaryData[i]);

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
		char *path = createPath(downloadCorrectPage(fileAnimeData[2]));

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

		// Numero anime da scaricare, modificato in seguito ai cambiamenti
		// apportati al main code
		switch (nEpi) {
			case 0:	// Li scarico tutti
					dwlOpt->option = 0;
					dwlOpt->firstEpisode = 0;
					dwlOpt->secondEpisode = lastData->numberOfEpisode;
					break;
			
			case 1:	// Scarico l'ultimo, ovvero e' uscito un nuovo episodio dall'ultimo controllo
					dwlOpt->option = 1;
					dwlOpt->firstEpisode = lastData->numberOfEpisode;
					dwlOpt->secondEpisode = dwlOpt->firstEpisode + 1;
					break;
		}

		// Numero anime da scaricare
		if (nEpi == 0) {
			// Li scarico tutti
			dwlOpt->option = 0;
			dwlOpt->firstEpisode = 0;
			dwlOpt->secondEpisode = lastData->numberOfEpisode;
		}
		else if (lastData->numberOfEpisode - nEpi == 1) {
			dwlOpt->option = 1;
			dwlOpt->firstEpisode = nEpi;
			dwlOpt->secondEpisode = dwlOpt->firstEpisode + 1;
		}
		else {
			dwlOpt->option = 2;
			dwlOpt->firstEpisode = nEpi;
			dwlOpt->secondEpisode = lastData->numberOfEpisode;
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
		char *copy = (char *) calloc(strlen(fileAnimeData[2]) + 1, sizeof(char));
		if (copy == NULL) {
			perror("calloc");
			_exit(-2);
		}
		strcpy(copy, fileAnimeData[2]);

		// #4: struct pronta, chiamata a funzione e terminazione del codice
		downloadPrepare(lastData, dwlOpt, copy, summaryData[i]);

		// #5: Aggiornamento del file .cfu
		// Per comodita', delete into recreate
		remove(filePath);
		f = fopen(filePath, "w");

		fprintf(f, "6\n%s\n%s\n%d\n%s\n%s\n", fileAnimeData[1], fileAnimeData[2], lastData->numberOfEpisode, fileAnimeData[4], fileAnimeData[5]);
		fclose(f);

		free(filePath);
		free(fileAnimeData);
		free(dwlOpt);
		free(copy);
	}

	for (; summaryLine != 0; free(summaryData[summaryLine--]));
	free(summaryData);
}

char **getLibrary (int *liner) {
	// "liner" verra' sovrascritto con il numero esatto di righe, che corrisponde
	// al numero di file presenti, ovvero al numero di preferiti esistenti
	const char *appdata = getenv("APPDATA");

	char *summaryFile = (char *) calloc(strlen(appdata) + 50, sizeof(char));
	if (summaryFile == NULL) {
		perror("calloc");
		_exit(-2);
	}

	sprintf(summaryFile, "%s\\AniLoader\\_data.summary", appdata);

	int summaryLine = 0;
	char **summaryData = createMatrixByEscapeCharacter(extractInMemoryFromFile(summaryFile, false), "\n", &summaryLine);
	*liner = summaryLine;

	// Elimino .cfu dalla fine
	for (; summaryLine != 0; summaryData[--summaryLine][strlen(summaryData[summaryLine]) - 4] = '\0');
//	for (int i = 0; i < summaryLine; i++)
//		summaryData[i][strlen(summaryData[i]) - 4] = '\0';
	
	return summaryData;
}

char **printLibrary (int *liner) {
	int line = 0;
	char **lib = getLibrary(&line);

	if (line == 0) {
		printf("Attualmente non ci sono preferiti salvati!");
		return NULL;
	}
	
	printf(ANSI_COLOR_GREEN "Elenco anime preferiti:\n\n" ANSI_COLOR_RESET);
	for (int i = 0; i < line; i++)
		printf(ANSI_COLOR_YELLOW "%d" ANSI_COLOR_RESET "] %s\n", i, lib[i]);

	*liner = line;
	return lib;
}

int delLibrary () {
	int line = 0;
	char **lib = printLibrary(&line);
	
	// Nessun anime tra i preferiti
	if (lib == NULL)
		return 0;

	char choice;
	do {
		printf("\nVuoi eliminare degli anime dai preferiti?\n");
		printf("Scelta [" ANSI_COLOR_GREEN "Y" ANSI_COLOR_RESET "/" ANSI_COLOR_RED "N" ANSI_COLOR_RESET "]: ");
		choice = toupper(getch());
	} while (choice != 'Y' && choice != 'N');

	// Evito un'annidamento escludendo a priori la possibilita' di N
	if (choice == 'N') {
		printf("\n");
		return 0;
	}

	// Aggiornamento del file
	const char *appdata = getenv("APPDATA");
	char *summaryFile = (char *) calloc(strlen(appdata) + 50, sizeof(char));
	if (summaryFile == NULL) {
		perror("calloc");
		_exit(-2);
	}

	sprintf(summaryFile, "%s\\AniLoader\\_data.summary", appdata);

	// Informazioni su chi eliminare
	int toDel = 0;
	while (true) {	
		do {
			system(clearScreen);
			lib = printLibrary(&line);

			printf("\nDigita il numero a lato dell'anime da eliminare o -1 per uscire: ");
			scanf("%d", &toDel);
		} while (toDel < -1 || toDel > line);

		// Dopo lo scanf(), intercetto il '\n' che rimane in memoria per prevenire errori nella ricerca
		getchar();
		
		// L'utente non vuole piu' eliminare preferiti
		if (toDel == -1)
			return 0;

		// Aggiornamento del file .cfu
		// Per comodita', delete into recreate
		remove(summaryFile);
		FILE *f = fopen(summaryFile, "w");

		for (int i = 0; i < line; i++) {
			if (i != toDel)
				fprintf(f, "%s.cfu\n", lib[i]);
		}
		
		fclose(f);
		
		char *delFile = (char *) calloc(strlen(appdata) + 50, sizeof(char));
		if (delFile == NULL) {
			perror("calloc");
			_exit(-2);
		}

		sprintf(delFile, "%s\\AniLoader\\%s.cfu", appdata, lib[toDel]);
		remove(delFile);
		
		// Se line = 1 e sono qui, significa che ora il file e' vuoto, esco per evitare crash
		if (line == 1) {
			system(clearScreen);
			printf(ANSI_COLOR_GREEN "Tutti i preferiti sono stati cancellati." ANSI_COLOR_RESET);
			return 0;
		}

		free(lib);
		free(delFile);
	}
}