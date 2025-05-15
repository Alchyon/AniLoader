// This file will be used only for CFU (Check for Updates) codes.
// It will keep everything inside for dividing the main code from the "DLC".

#include "libraries.h"
#include "function.h"
#include "utilities.h"
#include "cfu.h"

void addOnLoad (char *name, char *pageDirectLink, char *ext, char *downloadDirectory, char *nameEpisode) {
	// Si gestisce l'aggiunta delle informazioni dell'anime selezionato alla lista degli anime che vengono controllati on-boot.
	// Se sono qui, teoricamente ho gia' i dati richiesti per la creazione del file, quindi non dovrebbero servire ulteriori modifiche, al massimo una veloce
	// manipolazione di stringhe su qualche info, che pero' non viene piu' riutilizzata al termine di questa funzione, in quanto void!

	// Step 1:
	//	- Controllare se l'anime e' gia' presente nei preferiti
	// Per fare questo, si usa un formato standard di nomi, ovvero "nomeAnime.cfu", questo file verra' letto e, se il controllo da esito positivo, verra'
	// notificato all'utente. L'eliminazione puo' avvenire solo dal menu' principale.
	char *fileName = (char *) calloc(strlen(APPDATA) + strlen(name) + 30, sizeof(char));
	if (fileName == NULL) {
		perror("calloc");
		exit(2);
	}

	// Creazione stringa per il file dell'anime corrente
	sprintf(fileName, "%s/AniLoader/%s.cfu", APPDATA, fixDirectoryName(name));

	FILE *f = fopen(fileName, "r");
	if (f != NULL) {
		printf(ANSI_COLOR_RED "Errore: questo anime e' gia' stato aggiunto ai preferiti!\n" ANSI_COLOR_RESET);
		system("pause");

		// Riavvio al main pulendo le variabili usate
		fclose(f);
		free(fileName);

		main();
	}

	// Se il controllo viene superato significa che il file NON esiste e che quindi puo' essere creato e scritto
	// Step 2:
	//	- Creazione del file per CFU
	f = fopen(fileName, "w");

	// Step 3:
	//	- Scrittura dati su file anime
	fprintf(f, "6\n%s\n%s%s\n0\n%s\n%s\n", pageDirectLink, pageDirectLink, ext, downloadDirectory, nameEpisode);
	fclose(f);

	// Step 4:
	//	- Scrittura nome su file di elenco
	fileName[0] = '\0';
	sprintf(fileName, "%s/AniLoader/_data.summary", APPDATA);

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
	//	2: 	Variables assignments for step 3 (getLibrary())
	//	3:	foreach summaryData entries
	//	4:	creation cfuFile struct
	//	5:	calls to function to download, based on data

	// #1 & #2
	library *lib = getLibrary();
	char *pageContent;

	// #3
	for (int i = 0; i < lib->libLine; i++) {
		// Read from file, no reason to allocate, elaborated one by one.
		char *filePath = (char *) calloc(strlen(lib->libData[i]) + strlen(APPDATA) + 50, sizeof(char));
		if (filePath == NULL) {
			perror("calloc");
			exit(2);
		}

		// Path del singolo file, verra' resettato ad ogni iterazione, .cfu aggiunto hardcoded
		sprintf(filePath, "%s\\AniLoader\\%s.cfu", APPDATA, lib->libData[i]);

		// Considerato che extractInMemoryFromFile() ha una exit() in caso di file non esistente, verifico adesso che
		// esista e chiamo un continue in caso negativo
		FILE *f = fopen(filePath, "r");
		if (f == NULL)
			continue;

		// Lettura dati dal file, metodo senza struct
		fclose(f);
		int dataLine = 0;
		char **fileAnimeData = createMatrixByEscapeCharacter(extractInMemoryFromFile(filePath, false), "\n", &dataLine);

		// Ora partono le chiamate al main code per riutilizzare le funzioni, alcune strutture verranno ricreate in maniera
		// automatica in quanto i parametri sono gia' presenti nel file di salvataggio (... .cfu)

		// Creazione comando per scaricare la pagina corretta ed esecuzione
		downloadCorrectPage(fileAnimeData[2]);
		char *path = createPath("page.txt");

		// Contenuto della pagina dal file ed eliminazione dello stesso
		pageContent = extractInMemoryFromFile(path, true);
		free(path);

		// Trasforma l'array in una matrice
		int line = 0;
		char **pageDataResult = createMatrixByEscapeCharacter(pageContent, "\n", &line);

		// Struttura contenente le estensioni dei singoli episodi e il loro numero
		animeEpisodeData *lastData = getEpisodeExtension(pageDataResult, line);

		// Stato dell'anime, verra' utilizzato per decidere se l'anime e' da ritenersi completato o meno
		char **animeStatus = getAnimeStatus(pageDataResult, line, lastData->rLine);
		free(pageDataResult);

		if (lastData->numberOfEpisode == -1)
			// Errore, nessun episodio trovato
			// Skippo questo anime e passo a quello dopo
			continue;

		if (lastData->numberOfEpisode == 0) {
			// Server trovato ma nessun episodio disponibile
			printf(ANSI_COLOR_RED "Errore durante il recupero delle informazioni per: %s\n" ANSI_COLOR_RESET, lib->libData[i]);
			continue;
		}

		// Controllo episodi usciti e scaricati fino a questo momento
		int nEpi = atoi(fileAnimeData[3]);

		// Elimino se, e solo se, sono rispettate le seguenti regole:
		//	- lo stato dell'anime risulta "terminato"
		//	- il numero di episodi scaricati corrisponde al numero di episodi stimati
		//	- il numero di episodi scaricati corrisponde al numero di episodi attualmente disponibili
		if (strcmp(animeStatus[1], "finito") == 0 && nEpi >= atoi(animeStatus[0]) && nEpi >= lastData->numberOfEpisode) {
			delLibrary(lib, i);
			printf(ANSI_COLOR_CYAN "%s" ANSI_COLOR_RED " e' stato rimosso dai preferiti in quanto terminato\n" ANSI_COLOR_RESET, lib->libData[i]);
			continue;
		}

		// Anime non terminato ma nessun episodio nuovo uscito
		if (nEpi == lastData->numberOfEpisode) {
			printf("Nessun nuovo episodio per: %s\n", lib->libData[i]);
			continue;
		}

		// Creazione struct dati per downloadFile()
		downloadOption *dwlOpt = (downloadOption *) malloc(sizeof(downloadOption) + 1);
		if (dwlOpt == NULL) {
			perror("malloc");
			exit(2);
		}

		// Controllo valori degli episodi, in caso vi siano valori negativi, allora errore e skip al prossimo
		// Nota: previene la sovrascrittura di quelli gia' scaricati
		if (dwlOpt->firstEpisode < 0 || dwlOpt->secondEpisode < 0 || dwlOpt->secondEpisode - dwlOpt->firstEpisode < 0) {
			printf(ANSI_COLOR_RED "Errore durante il recupero delle informazioni per: " ANSI_COLOR_YELLOW "%s\n" ANSI_COLOR_RESET, lib->libData[i]);
			continue;
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
			exit(2);
		}

		// Alloco il puntatore della directory
		dwlOpt->downloadDirectory = (char *) malloc(sizeof(char) * 501);
		if (dwlOpt->downloadDirectory == NULL) {
			perror("malloc");
			exit(2);
		}

		// Directory
		strcpy(dwlOpt->downloadDirectory, fileAnimeData[4]);
		// Nome episodi
		dwlOpt->nameEpisodeChange = true;
		strcpy(dwlOpt->nameEpisode, fileAnimeData[5]);

		// #4: struct dwlOpt pronta, creazione struct cfuFile
		cfuFile *dataEpisodeCfu = (cfuFile *) malloc(sizeof(cfuFile));
		if (dataEpisodeCfu == NULL) {
			perror("malloc");
			exit(-2);
		}

		dataEpisodeCfu->cfuPath = filePath;
		dataEpisodeCfu->content = fileAnimeData;
		dataEpisodeCfu->dataLine = dataLine;

		// #5: struct pronta, chiamata a funzione e terminazione del codice
		// 	   il printf() serve solo a migliorare l'output grafico
		printf("\n");
		downloadPrepare(lastData, dwlOpt, strdup(fileAnimeData[2]), lib->libData[i], dataEpisodeCfu);
		printf("\n");

		free(filePath);
		free(fileAnimeData);
		free(dwlOpt);
	}

	free(lib);
}

void updateCfuFile (cfuFile *cfu, int numberOfEpisode) {
	FILE *f;

	// Eliminazione del file e successiva creazione, piu' veloce di editare una porzione di testo precisa
	// e usare puntatori alle righe
	remove(cfu->cfuPath);
	f = fopen(cfu->cfuPath, "w");

	// Scrittura delle informazioni richieste nel file .cfu
	fprintf(f, "%d\n%s\n%s\n%d\n%s\n%s\n", cfu->dataLine, cfu->content[1], cfu->content[2], numberOfEpisode, cfu->content[4], cfu->content[5]);
	fclose(f);
}

library *getLibrary () {
	// "libLine" verra' sovrascritto con il numero esatto di righe, che corrisponde
	// al numero di file presenti, ovvero al numero di preferiti esistenti
	library *lib = (library *) calloc(1, sizeof(library));
	if (lib == NULL) {
		perror("calloc");
		exit(2);
	}

	lib->summaryPath = (char *) calloc(strlen(APPDATA) + 50, sizeof(char));
	if (lib->summaryPath == NULL) {
		perror("calloc");
		exit(2);
	}

	sprintf(lib->summaryPath, "%s\\AniLoader\\_data.summary", APPDATA);

	lib->libLine = 0;
	lib->libData = createMatrixByEscapeCharacter(extractInMemoryFromFile(lib->summaryPath, false), "\n", &lib->libLine);

	// Elimina .cfu dalla fine di ogni file
	for (int i = lib->libLine; i != 0; lib->libData[--i][strlen(lib->libData[i]) - 4] = '\0');
//	for (int i = 0; i < lib->libLine; i++)
//		lib->libData[i][strlen(lib->libData[i]) - 4] = '\0';

	return lib;
}

void printLibrary () {
	library *lib = getLibrary();

	if (lib->libLine == 0)
		printf("Attualmente non ci sono preferiti salvati!\n");
	else {
		printf(ANSI_COLOR_GREEN "Elenco anime preferiti:\n\n" ANSI_COLOR_RESET);
		for (int i = 0; i < lib->libLine; i++) {
			// Spazio aggiuntivo per allineamento, improbabile si arrivi alla terza cifra
			printf(ANSI_COLOR_YELLOW "%2d" ANSI_COLOR_RESET "] %s\n", i, lib->libData[i]);
		}
	}
}

void libraryOption () {
	// Mi istanzio una libreria
	library *lib = getLibrary();

	// Stampo la libreria
	printLibrary();

	// Nessun anime tra i preferiti
	if (lib->libLine == 0)
		return;

	char choice;
	do {
		printf("\nVuoi eliminare degli anime dai preferiti?\n");
		printf("Scelta [" ANSI_COLOR_GREEN "Y" ANSI_COLOR_RESET "/" ANSI_COLOR_RED "N" ANSI_COLOR_RESET "]: ");
		choice = toupper(getch());
	} while (choice != 'Y' && choice != 'N');

	// Evito un'annidamento escludendo a priori la possibilita' di N
	if (choice == 'N') {
		printf("\n");
		return;
	}

	// Quando so chi eliminare, chiamo la funzione che mi elimina quell'elemento
	// e lo rendo void cosi' da poterlo utilizzare anche in CFU e in future applicazioni
	int toDel = 0;
	while (true) {
		do {
			system(clearScreen);
			lib = getLibrary();
			printLibrary();

			printf("\nDigita il numero a lato dell'anime da eliminare o -1 per uscire: ");
			printf(ANSI_COLOR_YELLOW);
			scanf("%d", &toDel);
			printf(ANSI_COLOR_RESET);
		} while (toDel < -1 || toDel > lib->libLine - 1);

		// Dopo lo scanf(), intercetto il '\n' che rimane in memoria per prevenire errori nella ricerca
		getchar();

		// L'utente non vuole piu' eliminare preferiti
		if (toDel == -1)
			return;

		// Aggiornamento del file .cfu
		// Per comodita', delete into recreate
		delLibrary(lib, toDel);

		// Se line = 1 e sono qui, significa che ora il file e' vuoto, esco per evitare crash
		if (lib->libLine == 1) {
			system(clearScreen);
			printf(ANSI_COLOR_GREEN "Tutti i preferiti sono stati cancellati.\n" ANSI_COLOR_RESET);
			return;
		}

		free(lib);
	}

	return;
}

void delLibrary(library *lib, int toDel) {
	remove(lib->summaryPath);
	FILE *f = fopen(lib->summaryPath, "w");

	for (int i = 0; i < lib->libLine; i++) {
		if (i != toDel)
			fprintf(f, "%s.cfu\n", lib->libData[i]);
	}

	fclose(f);

	char *delFile = (char *) calloc(strlen(APPDATA) + 50, sizeof(char));
	if (delFile == NULL) {
		perror("calloc");
		exit(2);
	}

	sprintf(delFile, "%s\\AniLoader\\%s.cfu", APPDATA, lib->libData[toDel]);
	remove(delFile);

	free(delFile);
}