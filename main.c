#include "libraries.h"
#include "function.h"
#include "utilities.h"
#include "cfu.h"

// Esecuzione del codice prima del main(), utilizzata per inizializzare il cookie
void __attribute__ ((constructor)) premain() {
	#if !_WIN32_WINNT_WIN10
		printf("Programma non compatibile con il sistema operativo.\n");
		printf("Supportati: Windows 10, Windows 11.\n");
		system("pause");
		exit(-3);
	#endif

	// Creazione della cartella "AniLoader" in "... AppData/Roaming" se non pre-esistente.
	char *appdataFolder = (char *) calloc(strlen(APPDATA) + 20, sizeof(char));
	if (appdataFolder == NULL) {
		perror("calloc");
		exit(2);
	}

	sprintf(appdataFolder, "%s/AniLoader", APPDATA);
	mkdir(appdataFolder);

	// Genera il cookie se non esistente, false significa che vi e' stato un errore
	if (generateCookieFile() == false) {
		printf("Impossibile generare un file di cookie, server non raggiungibile o mancanza di permessi di scrittura\n");
		system("pause");
		exit(4);
	}
}

int main () {
	// Funzione con il solo scopo grafico
	starting();
	// Menu' iniziale
	optionMenu();

	// Inserimento nome per ricerca
	char *name = insertName();

	// Variabili utilizzate nella parte sottostante per il funzionamento base del programma, dichiarate qui per questioni di scope
	char *path;
	char *searchData;
	int line = 0;
	char **searchDataResult;
	int divFound = 0;

	// Creazione ed esecuzione del comando per scaricare la pagina di ricerca
	// URL ... /search?keyword= ...
	searchAnimeByName(name);

	// Inserisco in una stringa il path assoluto del file scaricato poi...
	// Leggo tutto il file e lo inserisco in un array
	path = createPath("search.txt");
	searchData = extractInMemoryFromFile(path, true);
	free(path);

	// Creo una matrice contenente tutte le righe del file splittando i '\n' dal file
	line = 0;
	searchDataResult = createMatrixByEscapeCharacter(searchData, "\n", &line);
	free(searchData);

	// Cerco il div contenente gli anime
	divFound = searchAnimeDiv(searchDataResult, line);
	if (divFound == -1) {
		fflush(stdin);
		printf(ANSI_COLOR_RED "Errore, nessun anime trovato!" ANSI_COLOR_RESET "\n");
		printf("Premere un tasto per riavviare il programma . . .");
		getch();

		// Se non ci sono anime, questo puntatore e' vuoto
		free(searchDataResult);
		main();
	}

	// Estraggo i nomi degli anime dall'HTML
	animeSearchData *baseData = extractAnimeName(searchDataResult, line, divFound);
	free(searchDataResult);

	// Correggo eventuali nomi sbagliati (correzione caratteri speciali dovuti all'HTML)
	convertAnimeName(baseData);

	// Stampo i nomi degli anime trovati (massimo 40 risultati = 1a pagina HTML)
	if (!printFindAnime(baseData)) {
		// Se non vengono trovati anime riavvio il programma dall'inizio
		free(baseData);
		main();
	}

	// Ottengo la posizione dell'array che contiene l'anime selezionato
	int selected = selectAnime(baseData);
	// Controllo che l'utente non voglia chiudere il programma
	if (selected == -1) {
		// baseData ha dei dati al suo interno, visto che gli anime sono stati salvati, mi assicuro di svuotare interamente
		// la memoria occupata essendoci un riavvio
		for (; baseData->numberOfAnime != 0; baseData->numberOfAnime--, free(baseData->correctAnimeName[baseData->numberOfAnime]), free(baseData->findAnimeLink[baseData->numberOfAnime]));
		free(baseData);
		main();
	}

	// Scarico la pagina di redirect dell'anime selezionato, e' un path che identifica l'anime ma non gli episodi di esso
	// URL ... /search?keyword?= ... / nome_Anime /
	path = createPath(downloadRedirectPage(baseData, selected));

	// Leggo tutto il file e lo inserisco in un array
	char *redirectContent = extractInMemoryFromFile(path, true);
	free(path);

	// Ottengo il link diretto alla pagina dell'anime, ovvero quello con l'ID del primo episodio
	// URL ... /search?keyword?= ... / nome_Anime / ID
	char *pageDirectLink = getPageLink(redirectContent);
	free(redirectContent);

	// Creo il comando per scaricare la pagina corretta e lo eseguo
	path = createPath(downloadCorrectPage(pageDirectLink));

	// Ottengo il contenuto della pagina dal file ed elimino il file residuo
	char *pageContent = extractInMemoryFromFile(path, true);
	free(path);

	// Trasformo l'array in una matrice
	line = 0;
	char **pageDataResult = createMatrixByEscapeCharacter(pageContent, "\n", &line);

	// Ottengo una struttura contenente le estensioni dei singoli episodi e il loro numero
	animeEpisodeData *lastData = getEpisodeExtension(pageDataResult, line);
	// Controllo buona uscita
	if (lastData->numberOfEpisode == -1)
		// Errore, nessun episodio trovato, probabilmente server ID differente da quello usato nella ricerca
		main();

	// Leggo lo stato dell'anime, ampliabile liberamente in maniera relativamente semplice
	// Valori attuali: episodi totali [0], stato [1], data di uscita [2]
	char **animeStatus = getAnimeStatus(pageDataResult, line, lastData->rLine);
	free(pageDataResult);

	// Creo una struct con tutte le opzioni necessarie per il download
	downloadOption *settings = downloadMenu(baseData->correctAnimeName[selected], lastData->numberOfEpisode, animeStatus);

	// Chiamo l'ultima funzione del main che si occuperà di richiamare a sua volta funzioni annidate
	// in un ciclo per gestire in maniera completa il download, il printf() di avviso del download e' inserito
	// qui per evitare contrasti con CFU
	system(clearScreen);
	printf("Download in corso con le impostazioni fornite, questo processo potrebbe richiedere molto tempo.\n");
	printf("Non chiudere il programma o il download verra' interrotto e sara' irrecuperabile.\n");
	printf(ANSI_COLOR_GREEN "Avvio. . ." ANSI_COLOR_RESET "\n\n");
	downloadPrepare(lastData, settings, pageDirectLink, baseData->correctAnimeName[selected], NULL);

	printf(ANSI_COLOR_GREEN "\nDownload completato!\n" ANSI_COLOR_RESET);
	printf("Grazie per aver usato AniLoader, premi INVIO per chiudere il programma...");
	while (divFound = getch() != 13 && divFound != EOF);

	// Free finali 'incompleti' dato che segue la terminazione del codice
	free(pageDirectLink);
	free(baseData);
	free(lastData);
	free(animeStatus);
	free(settings);

	free(COOKIECMD);
	COOKIECMD = NULL;

	return 0;
}