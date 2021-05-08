#include "libraries.h"
#include "function.h"
#include "utilities.h"

// _exit() values:
// -1 -> errore apertura file
// -2 -> errore allocazione memoria
// -3 -> OS non supportato
// -4 -> scelta utente

int main()
{
	#if !_WIN32_WINNT_WIN10 && !__unix__
		printf("Programma non compatibile con il sistema operativo.\n");
		printf("Supportati: Windows 10, Linux\n");
		_exit(-3);
	#endif

	// Variabile per i futuri controlli sui cookie, necessaria per ogni richiesta
	cookie biscottino = { 0, "" };

	// Funzione con il solo scopo grafico, non modifica in alcun modo il funzionamento
	starting();
	// Menu' iniziale
	optionMenu();

	// Inserisco il nome da cercare, procedura separata per gestire i cookie
	char* name = insertName();
	
	// Variabili usate nella parte sottostante per ottenere i cookie se necessari, dichiarate qui per questioni di scope
	char* path;
	char* searchData;
	int line = 0;
	char** searchDataResult;
	int divFound = 0;

	// Cookie part
	for (int i = 0; i <= biscottino.enable; i++) {
		// Creazione ed esecuzione del comando per scaricare la pagina di ricerca
		searchAnimeByName(biscottino, name);

		// Creo la directory in cui e' stato salvato il file
		// Leggo tutto il file e lo inserisco in un array
		path = createPath("search.txt");
		searchData = extractInMemoryFromFile(path);
		free(path);

		// Creo una matrice contenente tutte le righe del file splittando i '\n' dal file
		line = 0;
		searchDataResult = createMatrixByEscapeCharacter(searchData, "\n", &line);
		free(searchData);
		
		// Cerco il div contenente gli anime
		divFound = 0;
		divFound = searchAnimeDiv(searchDataResult, line);
		// Se non e' stato trovato il div contenente gli anime
		if (divFound == -1)
		{
			// Se i cookie non sono abilitati e la stringa li contiene, li abilito, altrimenti errore nella ricerca
			if (biscottino.enable == 0 && strstr(searchDataResult[0], "AWCookietest") && line == 1) {
				fflush(stdin);
				printf(ANSI_COLOR_RED "Errore, accesso al sito concesso solo tramite cookie!" ANSI_COLOR_RESET "\n");
				retrieveCookie(&biscottino, searchDataResult, line);

				printf(ANSI_COLOR_GREEN "Cookie creato, nuova richiesta in corso..." ANSI_COLOR_RESET "\n");
				free(searchDataResult);
			}
			else
			{
				// Programma standard. Errore in caso di anime non trovati
				fflush(stdin);
				printf(ANSI_COLOR_RED "Errore, nessun anime trovato!" ANSI_COLOR_RESET "\n");
				printf("Premere un tasto per riavviare il programma . . .");
				getch();

				free(searchDataResult);
				main();
			}
		}
	}

	// Ottengo i nomi degli anime
	animeSearchData *baseData = extractAnimeName(searchDataResult, line, divFound);
	free(searchDataResult);

	// Correggo eventuali nomi sbagliati (correzione caratteri speciali dovuti all'HTML)
	convertAnimeName(baseData);

	// Stampo i nomi degli anime trovati (massimo 40 risultati = 1a pagina HTML)
	if (!printFindAnime(baseData))
	{
		// Se non vengono trovati anime riavvio il programma dall'inizio
		free(baseData);
		main();
	}

	// Ottengo la posizione dell'array che contiene l'anime selezionato
	int selected = selectAnime(baseData);
	// Controllo che l'utente non voglia chiudere il programma
	if (selected == -1)
	{
		// Riavvio il programma se -1
		free(baseData);
		main();
	}
	
	// Scarico la pagina di redirect dell'anime selezionato
	// Il return e' il nome del file creato che viene usato per creare il path del file da leggere
	path = createPath(downloadRedirectPage(baseData, biscottino, selected));

	// Leggo tutto il file e lo inserisco in un array
	char *redirectContent = extractInMemoryFromFile(path);
	free(path);

	// Ottengo il link diretto alla pagina dell'anime (estratto del redirect)
	char *pageDirectLink = getPageLink(redirectContent);
	free(redirectContent);

	// Creo il comando per scaricare la pagina corretta e lo eseguo
	path = createPath(downloadCorrectPage(biscottino, pageDirectLink));

	// Ottengo il contenuto della pagina dal file ed elimino lo stesso
	char *pageContent = extractInMemoryFromFile(path);
	free(path);

	// Trasformo l'array in una matrice
	line = 0;
	char **pageDataResult = createMatrixByEscapeCharacter(pageContent, "\n", &line);

	// Ottengo una struttura contenente le estensioni dei singoli episodi e il numero delle stesse
	animeEpisodeData *lastData = getEpisodeExtension(pageDataResult, line);
	// Controllo buona uscita
	free(pageDataResult);
	if (lastData->numberOfEpisode == -1)
	{
		// Errore, nessun episodio trovato
		// Catturo il carattere nell'stdin e riavvio il main()
		getchar();
		main();
	}

	// Creo una struct con tutte le opzioni necessarie per il download
	downloadOption *settings = downloadMenu(baseData->correctAnimeName[selected], lastData->numberOfEpisode);

	// Chiamo l'ultima funzione del main che si occuperà di richiamare a sua volta funzioni annidate
	// in un ciclo per gestire in maniera completa il download
	downloadPrepare(lastData, settings, biscottino, pageDirectLink, baseData->correctAnimeName[selected]);
	
	// Free finali
	free(pageDirectLink);
	free(baseData);
	free(lastData);
	free(settings);

	// '\n' finale per Linux allo scopo di mandare a capo il terminale
	return 0;
}

void starting()
{
	system(clearScreen);
	printf("########################################################\n");
	printf("#                                                      #\n");
	printf("#                      " ANSI_COLOR_GREEN "Ani-Loader" ANSI_COLOR_RESET "                      #\n");
	printf("#                      Stable 1.2                      #\n");
	printf("#                                                      #\n");
	printf("#            </>     - " ANSI_COLOR_GREEN "By Alcyon_" ANSI_COLOR_RESET " -     </>            #\n");
	printf("#                                                      #\n");
	printf("########################################################\n");
	printf("\n");
}

void optionMenu()
{
	printf("########################################################\n");
	printf("#" ANSI_COLOR_GREEN "                Impostazioni generali                 " ANSI_COLOR_RESET "#\n");
	printf("########################################################\n");
	printf("#                                                      #\n");
	printf("#" ANSI_COLOR_YELLOW "  INVIO" ANSI_COLOR_RESET " -- > Cerca un anime                           #\n");
	printf("#" ANSI_COLOR_YELLOW "  1" ANSI_COLOR_RESET " -- > Controlla l'ultimo changelog                 #\n");
	printf("#" ANSI_COLOR_YELLOW "  QUALSIASI ALTRO TASTO" ANSI_COLOR_RESET " -- > Esci dal programma       #\n");
	printf("#                                                      #\n");
	printf("########################################################\n");
	printf("Scegliere opzione: ");

	char option = getch();
	fflush(stdin);

	switch (option)
	{
	case '\n':
		break;
	case '\r':
		break;
	case '1':
		changelog();
		system(clearScreen);
		optionMenu();
		break;
	default:
		exit(-4);
	}
}

void retrieveCookie(cookie* biscottino, char** searchDataResult, int line) {
	// Imposto l'uso dei cookie
	biscottino->enable = 1;

	// Splitto la stringa con "= "
	char* token = strtok(searchDataResult[0], "= ");
	while (token != NULL) {
		// La riga successiva ad "AWCookietest" contiene il token necessario
		if (strstr(token, "AWCookietest")) {
			token = strtok(NULL, "= ");
			strcpy(biscottino->code, token);
			break;
		}

		token = strtok(NULL, "= ");
	}
}

char* insertName() {
	// Allocazione memoria per il nome
	char *name = (char *)malloc(sizeof(char) * 101);
	if (name == NULL) {
		perror("malloc");
		_exit(-2);
	}

	// Svuoto lo schermo per l'inserimento del nome
	system(clearScreen);

	// Inserimento del nome dell'anime [max 100 char]
	printf("Inserire il nome dell'anime da ricercare: " ANSI_COLOR_YELLOW);
	fgets(name, 100, stdin);
	printf(ANSI_COLOR_RESET);

	// Elimino il '\n' letto con fgets()
	name[strlen(name) - 1] = '\0';

	// Svuoto il buffer in ingresso a causa di fgets()
	fflush(stdin);

	// Essendo una richiesta HTTP, il nome non puo' contenere spazi che verranno quindi rimpiazzati dal segno '+'
	for (int i = 0; i < strlen(name); i++)
	{
		if (name[i] == ' ')
			name[i] = '+';
	}

	return name;
}

void searchAnimeByName(cookie biscottino, char* name)
{
	// Creazione comando
	char *command = (char *)malloc(sizeof(char) * (strlen(name) + strlen(biscottino.code) + 100));
	if (command == NULL) {
		perror("malloc");
		_exit(-2);
	}
	
	// Controllo per i cookie
	if (!biscottino.enable)
		sprintf(command, "curl -s \"https://www.animeworld.tv/search?keyword=%s\" > \"%s\"", name, createPath("search.txt"));
	else
		sprintf(command, "curl -s \"https://www.animeworld.tv/search?keyword=%s\" -b \"AWCookietest=%s\"> \"%s\"", name, biscottino.code, createPath("search.txt"));

	system(command);
	free(command);
}

int searchAnimeDiv(char **searchDataResult, int line)
{
	// Ricerco la linea della matrice in cui si trova l'inizio dell'HTML contenente i nomi degli anime
	for (int i = 0; i < line; i++)
	{
		char *div = strstr(searchDataResult[i], "<div class=\"film-list\">");

		if (div != NULL)
			return i + 3;
	}

	return -1;
}

animeSearchData *extractAnimeName(char **searchDataResult, int line, int divFound)
{
	int posUno = 0, posDue = 0;
	char *extract = (char *)malloc(sizeof(char));
	if (extract == NULL) {
		perror("malloc");
		_exit(-2);
	}

	// Allocazione struttura e puntatori interni, controllo errori
	animeSearchData *asd = (animeSearchData *)malloc(sizeof(animeSearchData));
	if (asd == NULL)
	{
		perror("malloc");
		_exit(-2);
	}

	asd->findAnimeLink = (char **)malloc(sizeof(char *) * 40);
	if (asd->findAnimeLink == NULL)
	{
		perror("malloc");
		_exit(-2);
	}

	asd->correctAnimeName = (char **)malloc(sizeof(char *) * 40);
	if (asd->correctAnimeName == NULL)
	{
		perror("malloc");
		_exit(-2);
	}

	// Inizializzo a 0 il numero di anime trovati
	asd->numberOfAnime = 0;

	// Parto a controllare la matrice dalla posizione divFound
	for (int i = divFound; i < line; i++)
	{
		// Controllo se nella riga in cui mi trovo inizia il div corretto
		char *poster = strstr(searchDataResult[i], "\" class=\"poster\"");
		if (poster != NULL)
		{
			// Creo una variabile di appoggio dinamica
			extract = (char *)malloc(sizeof(char) * (strlen(searchDataResult[i]) + 1));
			if (extract == NULL) {
				perror("malloc");
				_exit(-2);
			}

			strcpy(extract, searchDataResult[i]);

			for (int k = 0; k < strlen(extract); k++)
			{
				// Ottengo il punto di inizio e il punto di fine del link
				if (posUno == 0 && extract[k] == '\"')
				{
					posUno = k + 1;
					k = posUno + 1;
				}

				if (posDue == 0 && extract[k] == '\"')
					posDue = k;
			}

			// Allocazione variabile
			asd->findAnimeLink[asd->numberOfAnime] = (char *)malloc(sizeof(char) * (posDue - posUno + 1));
			if (asd->findAnimeLink[asd->numberOfAnime] == NULL)
			{
				perror("malloc");
				_exit(-2);
			}

			// Estrazione link
			for (int k = 0; posUno < posDue; k++, posUno++)
				asd->findAnimeLink[asd->numberOfAnime][k] = extract[posUno];

			// Resetto le posizioni
			posUno = 0;
			posDue = 0;

			// Controllo che nella riga successiva ci sia l'effettivo nome dell'anime
			char *alter = strstr(searchDataResult[i + 1], "alt=\"");
			if (alter != NULL)
			{
				// Uso sempre la variabile di appoggio
				extract = (char *)realloc(extract, sizeof(char) * (strlen(searchDataResult[i + 1]) + 1));
				strcpy(extract, alter);

				for (int k = 0; k < strlen(extract); k++)
				{
					// Ottengo il punto di inizio e la fine del nome dell'anime
					if (posUno == 0 && extract[k] == '\"')
					{
						posUno = k + 1;
						k = posUno + 1;
					}

					if (posDue == 0 && extract[k] == '\"')
						posDue = k;
				}

				// Allocazione variabile
				asd->correctAnimeName[asd->numberOfAnime] = (char *)malloc(sizeof(char) * (posDue - posUno + 1));
				if (asd->correctAnimeName[asd->numberOfAnime] == NULL)
				{
					perror("malloc");
					_exit(-2);
				}

				// Copia del nome
				for (int k = 0; posUno < posDue; k++, posUno++)
				{
					// Mi assicuro che la stringa termini perfettamente
					asd->correctAnimeName[asd->numberOfAnime][k] = extract[posUno];
					asd->correctAnimeName[asd->numberOfAnime][k + 1] = '\0';
				}

				// Incremento di 1 il numero di anime trovati
				asd->numberOfAnime++;

				// Resetto le posizioni
				posUno = 0;
				posDue = 0;
			}
		}
	}

	// Faccio ritornare la struttura (link, nomi anime trovati, numero di anime trovati)
	free(extract);
	return asd;
}

void convertAnimeName(animeSearchData *baseData)
{
	// Dichiarazione puntatore
	char *toConvert = (char *)malloc(sizeof(char));
	if (toConvert == NULL) {
		perror("malloc");
		_exit(-2);
	}

	for (int count = 0; count < baseData->numberOfAnime; count++)
	{
		// Alloco la giusta quantita' con realloc()
		toConvert = (char *)realloc(toConvert, strlen(baseData->correctAnimeName[count]));
		strcpy(toConvert, "");

		/* &quote; = " */
		if (strstr(baseData->correctAnimeName[count], "&quot;") != NULL)
		{
			// Remove > &quote;
			char *token = strtok(baseData->correctAnimeName[count], "&;");

			while (token != NULL)
			{
				// Copio la stringa rimanente se non trova l'escape, altrimenti incollo il carattere corretto
				if (strstr(token, "quot") == NULL)
					strcat(toConvert, token);
				else
					strcat(toConvert, "\"");

				token = strtok(NULL, "&;");
			}
		}

		/* &amp; = & */
		if (strstr(baseData->correctAnimeName[count], "&amp;") != NULL)
		{
			// Remove &amp;
			char *token = strtok(baseData->correctAnimeName[count], "&;");

			while (token != NULL)
			{
				// Copio la stringa rimanente se non trova l'escape, altrimenti incollo il carattere corretto
				if (strstr(token, "amp") == NULL)
					strcat(toConvert, token);
				else
					strcat(toConvert, "&");

				token = strtok(NULL, "&;");
			}
		}

		/* &#x27; = ' */
		if (strstr(baseData->correctAnimeName[count], "&#x27;") != NULL)
		{
			// Remove &#x27;
			char *token = strtok(baseData->correctAnimeName[count], "&;");

			while (token != NULL)
			{
				// Copio la stringa rimanente se non trova l'escape, altrimenti incollo il carattere corretto
				if (strstr(token, "#x27") == NULL)
					strcat(toConvert, token);
				else
					strcat(toConvert, "'");

				token = strtok(NULL, "&;");
			}
		}

		// Copy if MODIFIED
		if (strlen(toConvert) > 0)
			strcpy(baseData->correctAnimeName[count], toConvert);
	}

	// Libero la memoria allocata
	free(toConvert);
}

bool printFindAnime(animeSearchData *baseData)
{
	// Stampo un errore se gli episodi sono 0
	if (baseData->numberOfAnime < 1)
	{
		printf(ANSI_COLOR_RED "Errore: nessun anime trovato..." ANSI_COLOR_RESET "\n\n");
		printf("Questo puo' accadere quando, all'inserimento del nome, MANCANO o SONO PRESENTI caratteri aggiuntivi\n");
		printf("al nome dell'anime, un semplice spazio o la mancanza di trattini, punti o virgole possono creare questo errore.\n");
		printf("Riprovare inserendo meno caratteri o il nome in giapponese, se nonostante cio' non dovesse risolvere\n");
		printf("significa che sul sito non esiste quello che cercava.\n");
		printf(ANSI_COLOR_YELLOW "Premere un tasto per riavviare il programma . . ." ANSI_COLOR_RESET);
		getch();

		return false;
	}
	else
	{
		// Stampo gli anime trovati se sono 1 o piu'
		printf(ANSI_COLOR_GREEN "Anime trovati: \n\n" ANSI_COLOR_RESET);
		for (int count = 0; count < baseData->numberOfAnime; count++)
		{
			// Inserisco uno spazio se il numero è minore di 10 cosi' da allineare le scritte
			if (baseData->numberOfAnime >= 10 && count < 10)
				printf(" ");

			printf(ANSI_COLOR_YELLOW "%d" ANSI_COLOR_RESET "] %s \n", count, baseData->correctAnimeName[count]);
		}

		// Se gli anime sono esattamente 40 (ricerca poco precisa)
		if (baseData->numberOfAnime == 40)
		{
			// Piu' di una pagina disponibile, MFU ?
			printf("\n");
			printf("E' possibile che alcuni anime non vengano mostrati.\n");
			printf("La prossima volta provare con una ricerca piu' precisa!\n");
		}
	}

	return true;
}

int selectAnime(animeSearchData *baseData)
{
	int selected;
	bool enter = false;

	// Richiedo di inserire nuovamente il valore se l'input e' impossibile (indexOutOfBounds)
	do
	{
		if (enter)
		{
			printf("Valore non ammesso, riprovare...\n");
			printFindAnime(baseData);
		}

		printf("\nQuale anime vuoi scaricare? Digita il numero a lato oppure inserisci -1 per riavviare il programma!\n");
		printf("Scelta: ");
		printf(ANSI_COLOR_YELLOW);
		scanf("%d", &selected);
		printf(ANSI_COLOR_RESET);

		// Ho inserito l'input almeno una volta, in caso di errore ristampo gli anime
		enter = true;

		// Faccio direttamente il return e gestisco l'eccezione dallo stesso main (ho accesso a piu' variabili)
		if (selected == -1)
			return -1;

	} while (selected < 0 || selected > baseData->numberOfAnime - 1);

	return selected;
}

char *downloadRedirectPage(animeSearchData *baseData, cookie biscottino, int selected)
{
	char *command = (char *)malloc(sizeof(char) * (strlen(baseData->findAnimeLink[selected]) + strlen(biscottino.code) + 200));
	if (command == NULL) {
		perror("malloc");
		_exit(-2);
	}

	// Controllo cookie
	if (!biscottino.enable)
		sprintf(command, "curl -s \"https://www.animeworld.tv%s\" > \"%s\"", baseData->findAnimeLink[selected], createPath("redirect.txt"));
	else
		sprintf(command, "curl -s \"https://www.animeworld.tv%s\" -b \"AWCookietest=%s\"> \"%s\"", baseData->findAnimeLink[selected], biscottino.code, createPath("redirect.txt"));

	system(command);
	free(command);

	return "redirect.txt";
}

char *getPageLink(char *redirectContent)
{
	char *momentCopy = (char *)malloc(sizeof(char) * (strlen(redirectContent) + 1));
	if (momentCopy == NULL) {
		perror("malloc");
		_exit(-2);
	}

	char *token = strtok(redirectContent, " ");
	while (token != NULL)
	{
		token = strtok(NULL, " ");
		
		if (token != NULL)
			strcpy(momentCopy, token);
	}

	return momentCopy;
}

char *downloadCorrectPage(cookie biscottino, char *pageDirectLink)
{
	char *command = (char *)malloc(sizeof(char) * (strlen(pageDirectLink) + 250));
	if (command == NULL) {
		perror("malloc");
		_exit(-2);
	}

	// Controllo cookie
	if (!biscottino.enable)
		sprintf(command, "curl -s \"https://www.animeworld.tv%s\" > \"%s\"", pageDirectLink, createPath("page.txt"));
	else
		sprintf(command, "curl -s \"https://www.animeworld.tv%s\" -b \"AWCookietest=%s\"> \"%s\"", pageDirectLink, biscottino.code, createPath("page.txt"));

	system(command);
	free(command);

	return "page.txt";
}

animeEpisodeData *getEpisodeExtension(char **pageDataResult, int line)
{
	bool noData = true;
	char *moment = (char *)malloc(sizeof(char));
	if (moment == NULL) {
		perror("malloc");
		_exit(-2);
	}

	// Dichiaro e alloco la struttura che conterra' i dati calcolati dalla funzione
	animeEpisodeData *episodeData = (animeEpisodeData *)malloc(sizeof(animeEpisodeData));
	if (episodeData == NULL)
	{
		perror("malloc");
		_exit(2);
	}

	episodeData->animeEpisodeExtension = (char **)malloc(sizeof(char *) * 600);
	if (episodeData->animeEpisodeExtension == NULL)
	{
		perror("malloc");
		_exit(2);
	}

	episodeData->numberOfEpisode = 0;

	// Inizio calcoli su ogni riga del file letto
	for (int i = 0; i < line; i++)
	{
		// Mancanza server di AnimeWorld > errore a prescindere
		if (noData)
		{
			char *server = strstr(pageDataResult[i], "data-name=\"9\">AnimeWorld Server");
			if (server != NULL)
				noData = false;
		}

		if (!noData)
		{
			// Server trovato
			char *serverID = strstr(pageDataResult[i], "data-name=\"9\" data-type=\"iframe\" data-id=\"9\"");
			if (serverID != NULL)
			{
				// Ciclo per evitare di dover fare controlli separati
				for (; i < line; i++)
				{
					char *dataID = strstr(pageDataResult[i], "<a data-episode-id=\"");
					if (dataID != NULL)
					{
						// != NULL > episode
						// Rialloco lo spazio necessario a moment per contenere i dati
						moment = (char *)realloc(moment, sizeof(char) * (strlen(pageDataResult[i]) + 1));
						strcpy(moment, pageDataResult[i]);

						// Forzo lo script a raggiungere la parte interessata dalle " (virgolette)
						char *token = strtok(moment, "\"");
						token = strtok(NULL, "\"");
						token = strtok(NULL, "\"");
						token = strtok(NULL, "\"");
						token = strtok(NULL, "\"");
						token = strtok(NULL, "\"");

						// Alloco l'array che conterra' l'estensione
						// Ottengo l'ID dal tag "data-id"
						episodeData->animeEpisodeExtension[episodeData->numberOfEpisode] = (char *)malloc(sizeof(char) * (strlen(token) + 1));
						if (episodeData->animeEpisodeExtension[episodeData->numberOfEpisode] == NULL) {
							perror("malloc");
							_exit(-2);
						}

						strcpy(episodeData->animeEpisodeExtension[episodeData->numberOfEpisode], token);
						episodeData->numberOfEpisode++;

						// Controllo se sono presenti ulteriori div successivi
						char *linePlus = strstr(pageDataResult[i + 1], "</ul>");
						if (linePlus != NULL)
						{
							// != NULL > more DIV?
							char *linePlusPlus = strstr(pageDataResult[i + 2], "<a data-episode-id=\"");
							if (linePlusPlus == NULL)
							{
								// == NULL > NO more DIV
								i = line;
								break;
							}
						}
					}
				}
			}
		}
	}

	if (noData)
	{
		// Error 404: Server not found!
		system(clearScreen);
		printf(ANSI_COLOR_YELLOW "Episodi disponibili solo su server esterni o su VVVVID!" ANSI_COLOR_RESET "\n");
		printf(ANSI_COLOR_RED "Errore: impossibile scaricare attraverso questo programma..." ANSI_COLOR_RESET "\n");
		printf("Premere un tasto qualsiasi per riavviare il programma...");
		getch();

		episodeData->numberOfEpisode = -1;

		return episodeData;
	}

	free(moment);
	return episodeData;
}

downloadOption *downloadMenu(char *name, int numberOfAnime)
{
	// Allocazione struttura per valore di return
	downloadOption *option = (downloadOption *)malloc(sizeof(downloadOption));
	if (option == NULL) {
		perror("malloc");
		_exit(-2);
	}
	option->nameEpisodeChange = false;

	system(clearScreen);
	printf("Sono disponibili " ANSI_COLOR_GREEN "%d" ANSI_COLOR_RESET " episodi per \"" ANSI_COLOR_GREEN "%s" ANSI_COLOR_RESET "\"\n", numberOfAnime, name);
	printf("Opzioni disponibili: \n");
	printf(ANSI_COLOR_YELLOW "INVIO" ANSI_COLOR_RESET " -- > Scarica tutti gli episodi\n");
	printf(ANSI_COLOR_YELLOW "1" ANSI_COLOR_RESET "     -- > Scarica un singolo episodio\n");
	printf(ANSI_COLOR_YELLOW "2" ANSI_COLOR_RESET "     -- > Scarica un range di episodi\n");
	printf(ANSI_COLOR_YELLOW "ESC" ANSI_COLOR_RESET "   -- > Riavvia il programma\n");
	printf(ANSI_COLOR_YELLOW "QUALSIASI ALTRO TASTO" ANSI_COLOR_RESET " -- > Esci dal programma\n");

	printf("Scelta: ");
	char choice = getch();

	printf("\n");
	switch (choice)
	{
	case '\n':
	case '\r':
		// Tutti gli episodi
		option->option = 0;
		break;

	case '1':
		// Singolo episodio
		option->option = 1;
		do
		{
			printf("Quale episodio vuoi scaricare? [" ANSI_COLOR_GREEN "1" ANSI_COLOR_RESET " - " ANSI_COLOR_GREEN "%d" ANSI_COLOR_RESET "]: ", numberOfAnime);

			printf(ANSI_COLOR_YELLOW);
			scanf("%d", &option->singleEpisode);
			printf(ANSI_COLOR_RESET);

			option->singleEpisode--;
		} while (option->singleEpisode < 0 || option->singleEpisode > numberOfAnime - 1);
		break;

	case '2':
		// Range X->Y di episodi
		option->option = 2;
		do
		{
			printf("Seleziona da dove iniziare scaricare [" ANSI_COLOR_GREEN "1" ANSI_COLOR_RESET " - " ANSI_COLOR_GREEN "%d" ANSI_COLOR_RESET "]: ", numberOfAnime);

			printf(ANSI_COLOR_YELLOW);
			scanf("%d", &option->firstEpisode);
			printf(ANSI_COLOR_RESET);

			option->firstEpisode--;
		} while (option->firstEpisode < 0 || option->firstEpisode > numberOfAnime - 1);

		// Controllo che il numero inserito non corrisponda al numero esatto di episodi
		if (option->firstEpisode == numberOfAnime - 1)
		{
			option->option = 1;
			option->singleEpisode = option->firstEpisode;
			break;
		}
		else
		{
			// Mi assicuro che il secondo numero sia compreso tra quello inserito e il numero massimo
			do
			{
				printf("Seleziona fin dove scaricare [" ANSI_COLOR_GREEN "%d" ANSI_COLOR_RESET " - " ANSI_COLOR_GREEN "%d" ANSI_COLOR_RESET "]: ", option->firstEpisode + 1, numberOfAnime);

				printf(ANSI_COLOR_YELLOW);
				scanf("%d", &option->secondEpisode);
				printf(ANSI_COLOR_RESET);

				option->secondEpisode--;
			} while (option->secondEpisode < option->firstEpisode || option->secondEpisode > numberOfAnime - 1);
		}

		// Se i due numeri sono uguali e' come dire di scaricare solo quello, gestisco la cosa
		if (option->firstEpisode == option->secondEpisode)
		{
			option->option = 1;
			option->singleEpisode = option->firstEpisode;
		}
		break;

	case 27:
		free(option);
		getchar();
		main();
		break;

	default:
		system(clearScreen);
		_exit(-4);
		break;
	}

	// Alloco il puntatore del nome
	option->nameEpisode = (char *)malloc(sizeof(char) * 101);
	if (option->nameEpisode == NULL) {
		perror("malloc");
		_exit(-2);
	}
	// Alloco il puntatore della directory
	option->downloadDirectory = (char *)malloc(sizeof(char) * 201);
	if (option->downloadDirectory == NULL) {
		perror("malloc");
		_exit(-2);
	}

	// Modifica nome
	do
	{
		system(clearScreen);
		printf("Vuoi cambiare il modo in cui gli episodi verranno salvati?\n");
		printf("Di default saranno \"" ANSI_COLOR_GREEN "Episodio XXX" ANSI_COLOR_RESET "\", tu puoi cambiare il nome di episodio.\n");
		printf("Scelta [" ANSI_COLOR_GREEN "Y" ANSI_COLOR_RESET "/" ANSI_COLOR_RED "N" ANSI_COLOR_RESET "]: ");
		choice = toupper(getch());
	} while (choice != 'Y' && choice != 'N');

	// Modifica nome confermata
	if (choice == 'Y')
	{
		printf("\nInserire il nuovo nome da utilizzare [massimo 100 caratteri]: ");
		// Leggo 100 caratteri (getchar() serve per annullare un '\n' residuo)
		getchar();
		printf(ANSI_COLOR_YELLOW);
		fgets(option->nameEpisode, 100, stdin);
		printf(ANSI_COLOR_RESET);

		// Utilizzo "Episodio" se non e' stato inserito un nome
		if (strlen(option->nameEpisode) == 1)
			strcpy(option->nameEpisode, "Episodio");
		else
		{
			// Per evitare un getchar() successivo che crea problemi e trancia una lettera
			option->nameEpisodeChange = true;

			// Correggo eventuali caratteri speciali non ammessi nei nomi ed elimino lo '\n' finale
			option->nameEpisode[strlen(option->nameEpisode) - 1] = '\0';
			for (int i = 0; i < strlen(option->nameEpisode); i++)
			{
				if (option->nameEpisode[i] == '\\' ||
					option->nameEpisode[i] == '/' ||
					option->nameEpisode[i] == ':' ||
					option->nameEpisode[i] == '*' ||
					option->nameEpisode[i] == '?' ||
					option->nameEpisode[i] == '\"' ||
					option->nameEpisode[i] == '<' ||
					option->nameEpisode[i] == '>' ||
					option->nameEpisode[i] == '|' ||
					option->nameEpisode[i] == '{' ||
					option->nameEpisode[i] == '}' ||
					option->nameEpisode[i] == '~')

					option->nameEpisode[i] = '_';
			}
		}
	}
	else
		option->nameEpisode[0] = '\0';

	// Modifica directory
	do
	{
		fflush(stdin);
		system(clearScreen);
		printf("Vuoi cambiare la directory in cui verranno scaricati gli anime?\n");
		printf("Di default saranno nella cartella \"" ANSI_COLOR_GREEN "Downloads" ANSI_COLOR_RESET "\".\n");
		printf("Scelta [" ANSI_COLOR_GREEN "Y" ANSI_COLOR_RESET "/" ANSI_COLOR_RED "N" ANSI_COLOR_RESET "]: ");

		choice = toupper(getch());
	} while (choice != 'Y' && choice != 'N');

	// Modifica directory confermata
	if (choice == 'Y')
	{
		printf("\nInserite il percorso completo della nuova directory [massimo 200 caratteri]: " ANSI_COLOR_YELLOW);

		// Leggo 200 caratteri (getchar() serve per annullare un '\n' residuo in caso si sia immesso il nome su Linux)
		#ifdef __unix__
				if (!option->nameEpisodeChange)
					getchar();
		#endif

		fgets(option->downloadDirectory, 200, stdin);
		printf(ANSI_COLOR_RESET);

		// Utilizzo la directory di default se non e' stata modificata
		// Passo "" in modo che la stringa di return contenga solo il path assoluto della cartella di download
		if (strlen(option->downloadDirectory) == 1)
			strcpy(option->downloadDirectory, createPath(""));

		// Controllo che termini con uno slash o un backslash
		if (option->downloadDirectory[strlen(option->downloadDirectory)] != '\\' || option->downloadDirectory[strlen(option->downloadDirectory)] != '/')
			option->downloadDirectory[strlen(option->downloadDirectory) - 1] = '/';
	}
	else
		option->downloadDirectory[0] = '\0';

	return option;
}

// Funzioni che servono a ottenere informazioni e dati necessari al solo download
void downloadPrepare(animeEpisodeData *lastData, downloadOption *settings, cookie biscottino, char *pageDirectLink, char *name)
{
	// Correggo il directLink eliminando l'estensione
	int cut;
	for (int i = 0; i < strlen(pageDirectLink); i++)
	{
		if (pageDirectLink[i] == '/')
			cut = i;
	}
	pageDirectLink[cut + 1] = '\0';

	// Controllo se la directory e' stata modificata, in caso la creo
	char *mkdir;
	if (strlen(settings->downloadDirectory) != 0)
	{
		mkdir = (char *)malloc(sizeof(char) * (strlen(settings->downloadDirectory) + 100));
		if (mkdir == NULL) {
			perror("malloc");
			_exit(-2);
		}

		strcpy(mkdir, "mkdir \"");

		#ifdef __unix__
				if (settings->downloadDirectory[0] != '/' || settings->downloadDirectory[0] != '\\')
					strcat(mkdir, "/");
		#endif

		strcat(mkdir, settings->downloadDirectory);
		strcat(mkdir, "\"");

		system(mkdir);
	}
	else
	{
		mkdir = (char *)malloc(sizeof(char) * 200);
		if (mkdir == NULL) {
			perror("malloc");
			_exit(-2);
		}

		sprintf(mkdir, "mkdir \"%s%s\"", createPath(""), fixDirectoryName(name));
		system(mkdir);
	}

	// Dealloco mkdir
	free(mkdir);

	// Inizio switch per processo download
	system(clearScreen);
	printf("Download in corso con le impostazioni fornite, questo processo potrebbe richiedere molto tempo.\n");
	printf("Non chiudere il programma o il download verra' interrotto e sara' irrecuperabile.\n");
	printf(ANSI_COLOR_GREEN "Avvio. . ." ANSI_COLOR_RESET "\n\n");

	// Puntatori per valori di return
	char *directDownloadLink;

	switch (settings->option)
	{
	case 0:
		// Scarico tutti gli episodi
		printf(ANSI_COLOR_GREEN "Download in corso di %d episodi di \"" ANSI_COLOR_CYAN "%s" ANSI_COLOR_GREEN "\"" ANSI_COLOR_RESET "\n", lastData->numberOfEpisode, name);
		for (int i = 0; i < lastData->numberOfEpisode; i++)
		{
			printf("Download episodio %d\n", i + 1);

			// Mi faccio ritornare il link al download diretto
			directDownloadLink = getDirectEpisodeDownloadLink(biscottino, pageDirectLink, lastData->animeEpisodeExtension[i]);

			// Chiamo la funzione che gestisce la creazione del comando finale da eseguire
			if (strcmp(directDownloadLink, "ERROR"))
				downloadFile(lastData, settings, biscottino, pageDirectLink, directDownloadLink, name, i);
			else
				printf(ANSI_COLOR_RED "Si e' verificato un errore per l'episodio %d" ANSI_COLOR_RESET "\n", i + 1);

			// free
			free(directDownloadLink);
		}

		break;

	case 1:
		// Scarico un singolo episodio
		printf(ANSI_COLOR_GREEN "Download in corso dell'episodio %d di \"" ANSI_COLOR_CYAN "%s" ANSI_COLOR_GREEN "\"" ANSI_COLOR_RESET "\n", settings->singleEpisode + 1, name);

		// Mi faccio ritornare il link al download diretto
		directDownloadLink = getDirectEpisodeDownloadLink(biscottino, pageDirectLink, lastData->animeEpisodeExtension[settings->singleEpisode]);

		// Chiamo la funzione che gestisce la creazione del comando finale da eseguire
		if (strcmp(directDownloadLink, "ERROR"))
			downloadFile(lastData, settings, biscottino, pageDirectLink, directDownloadLink, name, settings->singleEpisode);
		else
			printf(ANSI_COLOR_RED "Si e' verificato un errore per l'episodio %d" ANSI_COLOR_RESET "\n", settings->singleEpisode + 1);

		// free
		free(directDownloadLink);

		break;

	case 2:
		// Scarico fra i selezionati
		printf(ANSI_COLOR_GREEN "Download in corso di %d episodi [%d -> %d] di \"" ANSI_COLOR_CYAN "%s" ANSI_COLOR_GREEN "\"" ANSI_COLOR_RESET "\n", settings->secondEpisode - settings->firstEpisode + 1, settings->firstEpisode + 1, settings->secondEpisode + 1, name);
		for (int i = settings->firstEpisode; i <= settings->secondEpisode; i++)
		{
			printf("Download episodio %d\n", i + 1);

			// Mi faccio ritornare il link al download diretto
			directDownloadLink = getDirectEpisodeDownloadLink(biscottino, pageDirectLink, lastData->animeEpisodeExtension[i]);

			// Chiamo la funzione che gestisce la creazione del comando finale da eseguire
			if (strcmp(directDownloadLink, "ERROR"))
				downloadFile(lastData, settings, biscottino, pageDirectLink, directDownloadLink, name, i);
			else
				printf(ANSI_COLOR_RED "Si e' verificato un errore per l'episodio %d" ANSI_COLOR_RESET "\n", i + 1);

			// free
			free(directDownloadLink);
		}

		break;
	}
}

char *getDirectEpisodeDownloadLink(cookie biscottino, char *pageDirectLink, char *extension)
{
	// Creazione comando di download
	char *downloadCommand = (char *)malloc(sizeof(char) * (strlen(extension) + strlen(pageDirectLink) + strlen(biscottino.code) + 200));
	if (downloadCommand == NULL) {
		perror("malloc");
		_exit(-2);
	}

	// Controllo cookie
	if (!biscottino.enable)
		sprintf(downloadCommand, "curl -s \"https://www.animeworld.tv%s%s\" > \"%s", pageDirectLink, extension, createPath("ep.txt\""));
	else
		sprintf(downloadCommand, "curl -s \"https://www.animeworld.tv%s%s\" -b \"AWCookietest=%s\"> \"%s", pageDirectLink, extension, biscottino.code, createPath("ep.txt\""));

	// Scarico il file
	system(downloadCommand);
	free(downloadCommand);

	// Apro il file, lo leggo, estraggo il contenuto, lo trasformo in matrice ed elimino il file
	char *filePath = createPath("ep.txt");
	int line = 0;

	char **fileEpisodeData = createMatrixByEscapeCharacter(extractInMemoryFromFile(filePath), "\n", &line);
	free(filePath);

	// Ora inizio lo script di web-scraping
	// - Ricerca di "Download Alternativo"
	int posUno = 0, posDue = 0;
	for (int i = 0; i < line; i++)
	{
		// Le righe hanno un minimo di lunghezza, inutile confrontare quelle minori
		if (strlen(fileEpisodeData[i]) >= 50)
		{
			// Controllo che mi trovi nella riga giusta contenente il link al download ( strstr() )
			if (strstr(fileEpisodeData[i], "Download Alternativo") != NULL)
			{
				// Riga giusta
				for (int k = 0; k < strlen(fileEpisodeData[i]); k++)
				{
					// Calcolo quanto e' lungo il link di download (punto di inizio, punto di fine)
					if (posUno == 0 && fileEpisodeData[i][k] == '\"')
					{
						posUno = k + 1;
						k = posUno + 1;
					}

					if (posDue == 0 && fileEpisodeData[i][k] == '\"')
						posDue = k;

					if (posUno != 0 && posDue != 0)
						break;
				}

				// Alloco un pointer per il link, gli copio dentro il link corretto ed eseguo il return della funzione
				char *returnLink = (char *)malloc(sizeof(char) * (posDue - posUno + 1));
				if (returnLink == NULL) {
					perror("malloc");
					_exit(-2);
				}

				for (int k = 0; posUno < posDue; k++, posUno++)
					returnLink[k] = fileEpisodeData[i][posUno];

				free(fileEpisodeData);
				return returnLink;
			}
		}
	}
	
	free(fileEpisodeData);
	return "ERROR";
}

void downloadFile(animeEpisodeData *lastData, downloadOption *settings, cookie biscottino, char *pageDirectLink, char *directDownloadLink, char *name, int i)
{
	// Per sprintf() del numero dell'episodio che deve scaricare
	char ep[5];

	// Creo un comando curl che scarica l'episodio dal link ottenuto prima in caso il valore di return sia diverso da "ERROR"
	char *downloadCommandLink = (char *)malloc(sizeof(char) * (strlen(directDownloadLink) + strlen(name) + strlen(biscottino.code) + 500));
	if (downloadCommandLink == NULL) {
		perror("malloc");
		_exit(-2);
	}
	
	// Cookie
	if (!biscottino.enable)
		sprintf(downloadCommandLink, "curl -L -# \"%s\" > \"", directDownloadLink);
	else
		sprintf(downloadCommandLink, "curl -L -# \"%s\" -b \"AWCookietest=%s\"> \"", directDownloadLink, biscottino.code);

	// Controllo se la directory e' stata modificata
	if (strlen(settings->downloadDirectory) != 0)
	{
		#ifdef __unix__
				if (settings->downloadDirectory[0] != '/' || settings->downloadDirectory[0] != '\\')
					strcat(downloadCommandLink, "/");
		#endif

		strcat(downloadCommandLink, settings->downloadDirectory);
	}
	else
	{
		strcat(downloadCommandLink, createPath(""));
		strcat(downloadCommandLink, fixDirectoryName(name));
		strcat(downloadCommandLink, "/");
	}

	// Controllo se il nome e' stato modificato
	if (strlen(settings->nameEpisode) != 0)
	{
		strcat(downloadCommandLink, settings->nameEpisode);
		strcat(downloadCommandLink, " ");
	}
	else
		strcat(downloadCommandLink, "Episodio ");

	// Gestione zeri nome del file
	if (lastData->numberOfEpisode >= 99)
	{
		if (i < 9)
			strcat(downloadCommandLink, "0");

		if (i < 99)
			strcat(downloadCommandLink, "0");
	}
	else if (i < 9)
		strcat(downloadCommandLink, "0");

	// Inserimento numero dell'episodio
	sprintf(ep, "%d", i + 1);
	strcat(downloadCommandLink, ep);

	// Inserimento estensione del file
	strcat(downloadCommandLink, ".mp4\"");

	// Comando di download
	system(downloadCommandLink);
	free(downloadCommandLink);
}