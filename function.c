#include "libraries.h"
#include "function.h"
#include "utilities.h"
#include "cfu.h"

// _exit() values:
//	0 -> terminato con successo, qualunque sia la causa
// -1 -> errore apertura file
// -2 -> errore allocazione memoria
// -3 -> OS non supportato

int main () {
	// TO-DO
	// - Avvio con parametri ??:
	//	-	--curl		directory custom per l'eseguibile di curl, bypassa il problema di limitazione dell'OS
	//	-	--cfu		avvio diretto per controllare i nuovi episodi usciti, tralasciando tutto il resto del codice

	#if !_WIN32_WINNT_WIN10
		printf("Programma non compatibile con il sistema operativo.\n");
		printf("Supportati: Windows 10, Windows 11.\n");
		system("pause");
		_exit(-3);
	#endif

	// Funzione con il solo scopo grafico
	starting();
	// Menu' iniziale
	optionMenu();

	// Inserisco il nome da cercare
	char *name = insertName();

	// Variabili usate nella parte sottostante per il funzionamento base del programma, dichiarate qui per questioni di scope
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

		free(searchDataResult);
		main();
	}

	// Estraggp i nomi degli anime dall'HTML
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
		// Gestisco un '\n' che permane dopo l'inserimento di '-1', non posso inserirlo nella funzione
		// oppure mi crea errori con l'output dell'ultimo anime trovato
		getchar();
		free(baseData);
		main();
	}
	
	// Scarico la pagina di redirect dell'anime selezionato, e' un path che identifica l'anime ma non gli episodi di esso
	// URL ... /search?keyword?= ... / nome_Anime /
	// Il return e' il nome del file creato che viene usato per creare il path del file da leggere
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
	free(pageDataResult);
	if (lastData->numberOfEpisode == -1) {
		// Errore, nessun episodio trovato, probabilmente server ID differente da quello usato nella ricerca
		// Catturo il carattere nell'stdin e riavvio il main()
		getchar();
		main();
	}

	// Creo una struct con tutte le opzioni necessarie per il download
	downloadOption *settings = downloadMenu(baseData->correctAnimeName[selected], lastData->numberOfEpisode);

	// Chiamo l'ultima funzione del main che si occuperà di richiamare a sua volta funzioni annidate
	// in un ciclo per gestire in maniera completa il download
	downloadPrepare(lastData, settings, pageDirectLink, baseData->correctAnimeName[selected]);
	
	// Free finali
	free(pageDirectLink);
	free(baseData);
	free(lastData);
	free(settings);

	return 0;
}

void starting () {
	system(clearScreen);
	printf("########################################################\n");
	printf("#                                                      #\n");
	printf("#                      " ANSI_COLOR_GREEN "Ani-Loader" ANSI_COLOR_RESET "                      #\n");
	printf("#                       v. 1.7.1                       #\n");
	printf("#                                                      #\n");
	printf("#            </>     - " ANSI_COLOR_GREEN "By Alchyon" ANSI_COLOR_RESET " -     </>            #\n");
	printf("#             " ANSI_COLOR_CYAN "Telegram" ANSI_COLOR_RESET ": " ANSI_COLOR_CYAN "@AniLoaderUpdates" ANSI_COLOR_RESET "              #\n");
	printf("#                                                      #\n");
	printf("########################################################\n");
	printf("\n");
}

void optionMenu () {
	printf("########################################################\n");
	printf("#" ANSI_COLOR_GREEN "                Impostazioni generali                 " ANSI_COLOR_RESET "#\n");
	printf("########################################################\n");
	printf("#                                                      #\n");
	printf("#" ANSI_COLOR_YELLOW "  INVIO" ANSI_COLOR_RESET " -- > Cerca un anime                           #\n");
	printf("#" ANSI_COLOR_YELLOW "  1" ANSI_COLOR_RESET " -- > Controlla l'ultimo changelog riassuntivo     #\n");
	printf("#" ANSI_COLOR_YELLOW "  2" ANSI_COLOR_RESET " -- > Visualizza o elimina gli anime preferiti     #\n");
	printf("#" ANSI_COLOR_YELLOW "  3" ANSI_COLOR_RESET " -- > Ricerca gli aggiornamenti per i preferiti    #\n");
	printf("#" ANSI_COLOR_YELLOW "  ESC" ANSI_COLOR_RESET " -- > Esci dal programma                         #\n");
	printf("#                                                      #\n");
	printf("########################################################\n");
	printf("Scegliere opzione: ");

	char option;
	do	option = getch();
	while (option != 13 && option != 27 && option != '1' && option != '2' && option != '3' && option != EOF);

	system(clearScreen);
	switch (option) {
		case '1':	changelog();
					break;
		
		case '2':	delLibrary();
					break;
		
		case '3':	printf(ANSI_COLOR_YELLOW "Controllo aggiornamenti in corso...\n" ANSI_COLOR_RESET);
					CheckForUpdatesRoutine();
					printf(ANSI_COLOR_GREEN "Aggiornamento completato.\n" ANSI_COLOR_RESET);
					break;
		
		case 27:	exit(-4);
	}

	switch (option) {
		case '1':
		case '2':
		case '3':	printf("Premere INVIO per continuare...");
					while (option = getch() != 13 && option != EOF);
					system(clearScreen);

					starting();
					optionMenu();
					break;
		
		case '\n':
		case '\r':	break;
	}
}

char *insertName () {
	// Allocazione memoria per il nome
	char *name = (char *) calloc(101, sizeof(char));
	if (name == NULL) {
		perror("calloc");
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

	// Essendo una richiesta HTTP, il nome non puo' contenere spazi che verranno quindi rimpiazzati dal segno '+'
	for (unsigned int i = 0; i < strlen(name); i++) {
		if (name[i] == ' ')
			name[i] = '+';
	}

	return name;
}

void searchAnimeByName (char* name) {
	// Creazione comando
	char *command = (char *) calloc(strlen(name) + 100, sizeof(char));
	if (command == NULL) {
		perror("calloc");
		_exit(-2);
	}

	sprintf(command, "curl -s \"%s/search?keyword=%s\" > \"%s\"", URL, name, createPath("search.txt"));

	system(command);
	free(command);
}

int searchAnimeDiv (char **searchDataResult, int line) {
	// Ricerco la linea della matrice in cui si trova l'inizio dell'HTML contenente i nomi degli anime
	for (int i = 0; i < line; i++) {
		char *div = strstr(searchDataResult[i], "<div class=\"film-list\">");

		if (div != NULL)
			return i + 3;
	}

	return -1;
}

animeSearchData *extractAnimeName (char **searchDataResult, int line, int divFound) {
	int posUno = 0, posDue = 0;
	char *extract = NULL;

	// Allocazione struttura e puntatori interni, controllo errori
	animeSearchData *asd = (animeSearchData *) malloc(sizeof(animeSearchData));
	if (asd == NULL) {
		perror("malloc");
		_exit(-2);
	}

	asd->findAnimeLink = (char **) malloc(sizeof(char *) * 40);
	if (asd->findAnimeLink == NULL) {
		perror("malloc");
		_exit(-2);
	}

	asd->correctAnimeName = (char **) malloc(sizeof(char *) * 40);
	if (asd->correctAnimeName == NULL) {
		perror("malloc");
		_exit(-2);
	}

	// Inizializzo a 0 il numero di anime trovati
	asd->numberOfAnime = 0;

	// Parto a controllare la matrice dalla posizione divFound
	for (int i = divFound; i < line; i++) {
		// Controllo se nella riga in cui mi trovo inizia il div corretto
		char *poster = strstr(searchDataResult[i], "\" class=\"poster\"");
		if (poster != NULL) {
			// Creo una variabile di appoggio dinamica
			extract = (char *) calloc(strlen(searchDataResult[i]) + 1, sizeof(char));
			if (extract == NULL) {
				perror("calloc");
				_exit(-2);
			}

			strcpy(extract, searchDataResult[i]);

			for (unsigned int k = 0; k < strlen(extract); k++) {
				// Ottengo il punto di inizio e il punto di fine del link
				if (posUno == 0 && extract[k] == '\"') {
					posUno = k + 1;
					k = posUno + 1;
				}

				if (posDue == 0 && extract[k] == '\"')
					posDue = k;
			}

			// Allocazione variabile, di base e' un +1, se da errori in qualche modo, aumentare a +10
			asd->findAnimeLink[asd->numberOfAnime] = (char *) calloc(posDue - posUno + 1, sizeof(char));
			if (asd->findAnimeLink[asd->numberOfAnime] == NULL) {
				perror("calloc");
				_exit(-2);
			}

			// Estrazione link
			for (int k = 0; posUno < posDue; k++, posUno++) {
				asd->findAnimeLink[asd->numberOfAnime][k] = extract[posUno];
				asd->findAnimeLink[asd->numberOfAnime][k + 1] = '\0';
			}

			// Resetto le posizioni
			posUno = 0;
			posDue = 0;

			// Controllo che nella riga successiva ci sia l'effettivo nome dell'anime
			char *alter = strstr(searchDataResult[i + 1], "alt=\"");
			if (alter != NULL) {
				// Uso sempre la variabile di appoggio
				extract = (char *) realloc(extract, sizeof(char) * (strlen(searchDataResult[i + 1]) + 10));
				strcpy(extract, alter);

				for (unsigned int k = 0; k < strlen(extract); k++) {
					// Ottengo il punto di inizio e la fine del nome dell'anime
					if (posUno == 0 && extract[k] == '\"')
					{
						posUno = k + 1;
						k = posUno + 1;
					}

					if (posDue == 0 && extract[k] == '\"')
						posDue = k;
				}

				// Allocazione variabile, stesso discorso di sopra, +1 per correttezza, +10 per sicurezza
				asd->correctAnimeName[asd->numberOfAnime] = (char *) calloc(posDue - posUno + 1, sizeof(char));
				if (asd->correctAnimeName[asd->numberOfAnime] == NULL) {
					perror("calloc");
					_exit(-2);
				}

				// Copia del nome
				for (int k = 0; posUno < posDue; k++, posUno++) {
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

void convertAnimeName (animeSearchData *baseData) {
	// Dichiarazione puntatore
	char *toConvert = NULL;

	for (int count = 0; count < baseData->numberOfAnime; count++) {
		// Alloco la giusta quantita' con realloc()
		toConvert = (char *) realloc(toConvert, sizeof(char) * (strlen(baseData->correctAnimeName[count]) + 10));
		strcpy(toConvert, "");

		/* &quote; = " */
		if (strstr(baseData->correctAnimeName[count], "&quot;") != NULL) {
			// Remove > &quote;
			char *token = strtok(baseData->correctAnimeName[count], "&;");

			while (token != NULL) {
				// Copio la stringa rimanente se non trova l'escape, altrimenti incollo il carattere corretto
				if (strstr(token, "quot") == NULL)
					strcat(toConvert, token);
				else
					strcat(toConvert, "\"");

				token = strtok(NULL, "&;");
			}
		}

		/* &amp; = & */
		if (strstr(baseData->correctAnimeName[count], "&amp;") != NULL) {
			// Remove &amp;
			char *token = strtok(baseData->correctAnimeName[count], "&;");

			while (token != NULL) {
				// Copio la stringa rimanente se non trova l'escape, altrimenti incollo il carattere corretto
				if (strstr(token, "amp") == NULL)
					strcat(toConvert, token);
				else
					strcat(toConvert, "&");

				token = strtok(NULL, "&;");
			}
		}

		/* &#x27; = ' */
		if (strstr(baseData->correctAnimeName[count], "&#x27;") != NULL) {
			// Remove &#x27;
			char *token = strtok(baseData->correctAnimeName[count], "&;");

			while (token != NULL) {
				// Copio la stringa rimanente se non trova l'escape, altrimenti incollo il carattere corretto
				if (strstr(token, "#x27") == NULL)
					strcat(toConvert, token);
				else
					strcat(toConvert, "'");

				token = strtok(NULL, "&;");
			}
		}

		// TO-DO, Unicode into ASCII
		/* ÔÇÖ = ' */
		/* ÔÇ£ = “ */
		/* ÔÇØ = ” */
		/* ÔÇô = - */
		/* ├╣  = ù */

		// Copy if MODIFIED
		if (strlen(toConvert) > 0)
			strcpy(baseData->correctAnimeName[count], toConvert);
	}

	// Libero la memoria allocata
	free(toConvert);
}

bool printFindAnime (animeSearchData *baseData) {
	// Stampo un errore se gli episodi sono 0
	if (baseData->numberOfAnime < 1) {
		printf(ANSI_COLOR_RED "Errore: nessun anime trovato..." ANSI_COLOR_RESET "\n\n");
		printf("Questo puo' accadere quando, all'inserimento del nome, MANCANO o SONO PRESENTI caratteri aggiuntivi\n");
		printf("come spazi, trattini, punti o virgole. Si consiglia di riprovare inserendo meno caratteri o eventualmente\n");
		printf("usando il nome in giapponese, se nonostante cio' non verranno trovati risultati, allora significa che l'anime\n");
		printf("non e' presente o e' codificato in un formato sconosciuto.\n");
		printf(ANSI_COLOR_YELLOW "Premere un tasto per riavviare il programma . . ." ANSI_COLOR_RESET);
		getch();

		return false;
	}
	else {
		// Stampo gli anime trovati se sono 1 o piu'
		printf(ANSI_COLOR_GREEN "Anime trovati: \n\n" ANSI_COLOR_RESET);
		for (int count = 0; count < baseData->numberOfAnime; count++) {
			// Inserisco uno spazio se il numero è minore di 10 cosi' da allineare le scritte
			if (baseData->numberOfAnime >= 10 && count < 10)
				printf(" ");

			printf(ANSI_COLOR_YELLOW "%d" ANSI_COLOR_RESET "] %s \n", count, baseData->correctAnimeName[count]);
		}

		// Se gli anime sono esattamente 40 (ricerca poco precisa)
		if (baseData->numberOfAnime == 40) {
			// TO-DO Piu' di una pagina disponibile, update futuro ?
			printf("\nE' possibile che alcuni anime non vengano mostrati.\n");
			printf("La prossima volta si consiglia di provare con una ricerca piu' precisa!\n");
		}
	}

	return true;
}

int selectAnime (animeSearchData *baseData) {
	int selected;
	bool enter = false;

	// Richiedo di inserire nuovamente il valore se l'input e' impossibile (indexOutOfBounds)
	do {
		if (enter) {
			printf("Valore non ammesso, riprovare...\n");
			printFindAnime(baseData);
		}

		// Nota: il %c serve a grabbare il "\n" derivante dall'invio
		printf("\nQuale anime vuoi scaricare? Digita il numero a lato oppure inserisci -1 per riavviare il programma!\nScelta: ");
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

char *downloadRedirectPage (animeSearchData *baseData, int selected) {
	char *command = (char *) calloc(strlen(baseData->findAnimeLink[selected]) + 300, sizeof(char));
	if (command == NULL) {
		perror("calloc");
		_exit(-2);
	}

	// Concateno in una stringa il comando da eseguire
	sprintf(command, "curl -s \"%s%s\" > \"%s\"", URL, baseData->findAnimeLink[selected], createPath("redirect.txt"));

	system(command);
	free(command);

	return "redirect.txt";
}

char *getPageLink (char *redirectContent) {
	char *momentCopy = (char *) calloc(strlen(redirectContent) + 1, sizeof(char));
	if (momentCopy == NULL) {
		perror("calloc");
		_exit(-2);
	}

	char *token = strtok(redirectContent, " ");
	while (token != NULL) {
		token = strtok(NULL, " ");
		
		if (token != NULL)
			strcpy(momentCopy, token);
	}

	return momentCopy;
}

char *downloadCorrectPage (char *pageDirectLink) {
	char *command = (char *) calloc(strlen(pageDirectLink) + 300, sizeof(char));
	if (command == NULL) {
		perror("calloc");
		_exit(-2);
	}

	sprintf(command, "curl -s \"%s%s\" > \"%s\"", URL, pageDirectLink, createPath("page.txt"));

	system(command);
	free(command);

	return "page.txt";
}

animeEpisodeData *getEpisodeExtension (char **pageDataResult, int line) {
	bool noData = true;
	char *moment;

	// Dichiaro e alloco la struttura che conterra' i dati calcolati dalla funzione
	animeEpisodeData *episodeData = (animeEpisodeData *) malloc(sizeof(animeEpisodeData));
	if (episodeData == NULL) {
		perror("malloc");
		_exit(2);
	}

	// One Piece e Detective Conan hanno 1000+ episodi e sono gli anime con il numero maggiore. Il 1500 sottostante e' dovuto alla loro esistenza.
	// 21/02/2024, record: 1137 in Detective Conan
	episodeData->animeEpisodeExtension = (char **) malloc(sizeof(char *) * 1500);
	if (episodeData->animeEpisodeExtension == NULL) {
		perror("malloc");
		_exit(2);
	}

	episodeData->numberOfEpisode = 0;

	// Inizio calcoli su ogni riga del file letto
	for (int i = 0; i < line; i++) {
		// Mancanza server di AnimeWorld > errore a prescindere
		if (noData) {
			char *server = strstr(pageDataResult[i], serverID);
			if (server != NULL)
				noData = false;
		}

		if (!noData) {
			// Server trovato
			char *serverCheck = strstr(pageDataResult[i], "data-name=\"" serverNumber "\" data-type=\"iframe\" data-id=\"9\"");
			if (serverCheck != NULL) {
				// Ciclo per evitare di dover fare controlli separati
				for (; i < line; i++) {
					char *dataID = strstr(pageDataResult[i], "<a data-episode-id=\"");
					if (dataID != NULL) {
						// != NULL > episodio trovato
						// Rialloco lo spazio necessario a moment per contenere i dati
						moment = (char *) realloc(moment, sizeof(char) * (strlen(pageDataResult[i]) + 1));
						strcpy(moment, pageDataResult[i]);

						// Forzo lo script a raggiungere la parte interessata dalle " (virgolette)
						char *token = strtok(moment, "\"");
						token = strtok(NULL, "\"");
						token = strtok(NULL, "\"");
						token = strtok(NULL, "\"");
						token = strtok(NULL, "\"");
						token = strtok(NULL, "\"");

						// Alloco l'array che conterra' l'estensione, il "+10" e' dovuto al fatto che alcuni ID hanno una dimensione > 5
						// Ottengo l'ID dal tag "data-id"
						episodeData->animeEpisodeExtension[episodeData->numberOfEpisode] = (char *) calloc(strlen(token) + 10, sizeof(char));
						if (episodeData->animeEpisodeExtension[episodeData->numberOfEpisode] == NULL) {
							perror("calloc");
							_exit(-2);
						}

						strcpy(episodeData->animeEpisodeExtension[episodeData->numberOfEpisode], token);
						episodeData->numberOfEpisode++;

						// Controllo se sono presenti ulteriori div successivi
						if (strstr(pageDataResult[i + 1], "</ul>") != NULL && strstr(pageDataResult[i + 2], "<a data-episode-id=\"") == NULL) {
							i = line;
							break;
						}
					}
				}
			}
		}
	}

	if (noData) {
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

downloadOption *downloadMenu (char *name, int numberOfEpisode) {
	// Allocazione struttura per valore di return
	downloadOption *option = (downloadOption *) malloc(sizeof(downloadOption));
	if (option == NULL) {
		perror("malloc");
		_exit(-2);
	}

	// Setto a false dei parametri che necessitano di inizializzazione
	option->nameEpisodeChange = false;
	option->firstEpisode = -1;
	option->secondEpisode = -1;

	system(clearScreen);
	printf("Sono disponibili " ANSI_COLOR_GREEN "%d" ANSI_COLOR_RESET " episodi per \"" ANSI_COLOR_GREEN "%s" ANSI_COLOR_RESET "\"\n", numberOfEpisode, name);
	printf("Opzioni disponibili: \n");
	printf(ANSI_COLOR_YELLOW "INVIO" ANSI_COLOR_RESET " -- > Scarica tutti gli episodi\n");
	printf(ANSI_COLOR_YELLOW "1" ANSI_COLOR_RESET "     -- > Scarica un singolo episodio\n");
	printf(ANSI_COLOR_YELLOW "2" ANSI_COLOR_RESET "     -- > Scarica un range di episodi\n");
	printf(ANSI_COLOR_YELLOW "3" ANSI_COLOR_RESET "     -- > Controlla all'avvio nuove uscite\n");
	printf(ANSI_COLOR_YELLOW "ESC" ANSI_COLOR_RESET "   -- > Riavvia il programma\n");
	printf(ANSI_COLOR_YELLOW "QUALSIASI ALTRO TASTO" ANSI_COLOR_RESET " -- > Esci dal programma\n");

	printf("Scelta: ");
	char choice = getch();
	getchar();

	printf("\n");
	switch (choice) {
		case '\n':
		case '\r':	// Tutti gli episodi
					option->option = 0;
					option->firstEpisode = 0;
					option->secondEpisode = numberOfEpisode;
					break;

		case '1':	// Singolo episodio
					option->option = 1;
					do {
						printf("Quale episodio vuoi scaricare? [" ANSI_COLOR_GREEN "1" ANSI_COLOR_RESET " - " ANSI_COLOR_GREEN "%d" ANSI_COLOR_RESET "]: ", numberOfEpisode);

						printf(ANSI_COLOR_YELLOW);
						scanf("%d", &option->firstEpisode);
						printf(ANSI_COLOR_RESET);

						option->firstEpisode--;
					} while (option->firstEpisode < 0 || option->firstEpisode > numberOfEpisode - 1);

					option->secondEpisode = option->firstEpisode + 1;
					break;

		case '2':	// Range X -> Y di episodi
					option->option = 2;
					do {
						printf("Seleziona da dove iniziare scaricare [" ANSI_COLOR_GREEN "1" ANSI_COLOR_RESET " - " ANSI_COLOR_GREEN "%d" ANSI_COLOR_RESET "]: ", numberOfEpisode);

						printf(ANSI_COLOR_YELLOW);
						scanf("%d", &option->firstEpisode);
						printf(ANSI_COLOR_RESET);

						option->firstEpisode--;
					} while (option->firstEpisode < 0 || option->firstEpisode > numberOfEpisode - 1);

					// Controllo che il numero inserito non corrisponda al numero esatto di episodi, in quel caso scarico solo l'ultimo
					if (option->firstEpisode == numberOfEpisode - 1) {
						option->option = 1;
						option->secondEpisode = option->firstEpisode + 1;
						break;
					}
					else {
						// Mi assicuro che il secondo numero sia compreso tra quello inserito e il numero massimo
						do {
							printf("Seleziona fin dove scaricare [" ANSI_COLOR_GREEN "%d" ANSI_COLOR_RESET " - " ANSI_COLOR_GREEN "%d" ANSI_COLOR_RESET "]: ", option->firstEpisode + 1, numberOfEpisode);

							printf(ANSI_COLOR_YELLOW);
							scanf("%d", &option->secondEpisode);
							printf(ANSI_COLOR_RESET);
						} while (option->secondEpisode < option->firstEpisode || option->secondEpisode > numberOfEpisode);
					}

					// Se i due numeri sono uguali e' come dire di scaricare solo quello, gestisco la cosa
					if (option->firstEpisode == option->secondEpisode - 1) {
						option->option = 1;
						option->secondEpisode = option->firstEpisode + 1;
					}
					break;

		case '3':	// Essendo il parametro "option" il valore di controllo, modifico questo e, senza cambiare il codice, aggiungo
					// alla funzione successiva le istruzioni per questa impostazione, sfruttando il codice precedentemente esistente
					// per evitare duplicati, faccio il "download" delle informazioni ma camuffandolo all'interno di codice gia' presente.
					option->option = 3;
					break;

		case 27:	// ESC -> riavvio
					free(option);
					getchar();
					main();
					break;

		default:	system(clearScreen);
					_exit(0);
					break;
	}

	// Alloco il puntatore del nome
	option->nameEpisode = (char *) calloc(101, sizeof(char));
	if (option->nameEpisode == NULL) {
		perror("calloc");
		_exit(-2);
	}
	// Alloco il puntatore della directory
	option->downloadDirectory = (char *) calloc(501, sizeof(char));
	if (option->downloadDirectory == NULL) {
		perror("calloc");
		_exit(-2);
	}

	// Modifica nome dei singoli episodi
	do {
		system(clearScreen);
		printf("Vuoi cambiare il modo in cui gli episodi verranno salvati?\n");
		printf("Di default saranno \"" ANSI_COLOR_GREEN "Episodio XXX" ANSI_COLOR_RESET "\", tu puoi cambiare il nome di episodio.\n");
		printf("Scelta [" ANSI_COLOR_GREEN "Y" ANSI_COLOR_RESET "/" ANSI_COLOR_RED "N" ANSI_COLOR_RESET "]: ");
		choice = toupper(getch());
	} while (choice != 'Y' && choice != 'N');

	// Modifica nome confermata
	if (choice == 'Y') {
		printf("\nInserire il nuovo nome da utilizzare [massimo 100 caratteri]: ");
		// Leggo 100 caratteri (getchar() serve per annullare un '\n' residuo)
		getchar();
		printf(ANSI_COLOR_YELLOW);
		fgets(option->nameEpisode, 100, stdin);
		printf(ANSI_COLOR_RESET);

		// Utilizzo "Episodio" se non e' stato inserito un nome
		if (strlen(option->nameEpisode) == 1)
			strcpy(option->nameEpisode, "Episodio");
		else {
			option->nameEpisodeChange = true;

			// Correggo eventuali caratteri speciali non ammessi nei nomi ed elimino lo '\n' finale
			option->nameEpisode[strlen(option->nameEpisode) - 1] = '\0';
			for (unsigned int i = 0; i < strlen(option->nameEpisode); i++) {
				// TO-DO optimize into function FixDirectoryName() into FixDataName() or FixFileName()
				if (option->nameEpisode[i] == '\\' ||	option->nameEpisode[i] == '/' ||	option->nameEpisode[i] == ':' ||
					option->nameEpisode[i] == '*' ||	option->nameEpisode[i] == '?' ||	option->nameEpisode[i] == '\"' ||
					option->nameEpisode[i] == '<' ||	option->nameEpisode[i] == '>' ||	option->nameEpisode[i] == '|' ||
					option->nameEpisode[i] == '{' ||	option->nameEpisode[i] == '}' ||	option->nameEpisode[i] == '~')

					option->nameEpisode[i] = '_';
			}
		}
	}
	else
		option->nameEpisode[0] = '\0';

	// Modifica directory
	do {
		system(clearScreen);
		printf("Vuoi cambiare la directory in cui verranno scaricati gli anime?\n");
		printf("Di default saranno nella cartella \"" ANSI_COLOR_GREEN "Downloads" ANSI_COLOR_RESET "\".\n");
		printf("Scelta [" ANSI_COLOR_GREEN "Y" ANSI_COLOR_RESET "/" ANSI_COLOR_RED "N" ANSI_COLOR_RESET "]: ");

		choice = toupper(getch());
	} while (choice != 'Y' && choice != 'N');

	// Modifica directory confermata
	if (choice == 'Y') {
		printf("\nInserite il percorso completo della nuova directory [massimo 400 caratteri]: " ANSI_COLOR_YELLOW);

		fgets(option->downloadDirectory, 400, stdin);
		printf(ANSI_COLOR_RESET);

		// Utilizzo la directory di default se non e' stata modificata, premuto '\n'
		// Passo "" in modo che la stringa di return contenga solo il path assoluto della cartella di download
		if (strlen(option->downloadDirectory) == 1)
			option->downloadDirectory[0] = '\0';

		// Controllo che termini con uno slash o un backslash e controllo la presenza di caratteri non ammessi, in questo caso ripristino quella di default, piu' pratico
		// del chiedere un nuovo inserimento, verra' implementato in un futuro
		else if (option->downloadDirectory[strlen(option->downloadDirectory)] != '\\' && option->downloadDirectory[strlen(option->downloadDirectory)] != '/') {
			option->downloadDirectory[strlen(option->downloadDirectory) - 1] = '/';

			for (unsigned int i = 0; i < strlen(option->downloadDirectory) && choice == 'Y'; i++)
				switch (option->downloadDirectory[i]) {
					case '*':	case '?':	case '\"':
					case '<':	case '>':	case '|':
					case '{':	case '}':	case '~':
					
						printf(ANSI_COLOR_RED "\nErrore!\n" ANSI_COLOR_RESET);
						printf("Il nome della directory contiene dei caratteri non ammessi, ripristino la cartella di default!\n");
						system("pause");

						option->downloadDirectory[0] = '\0';
						choice = 'E';
						break;
				}
		}
	}
	else
		option->downloadDirectory[0] = '\0';

	return option;
}

// Funzioni che servono a ottenere informazioni e dati necessari al solo download
void downloadPrepare (animeEpisodeData *lastData, downloadOption *settings, char *pageDirectLink, char *name) {
	// Correggo il directLink eliminando l'estensione
	int i;
	for (i = strlen(pageDirectLink); pageDirectLink[i] != '/'; i--);
	pageDirectLink[i + 1] = '\0';

	// Inizio switch per processo download
	system(clearScreen);
	printf("Download in corso con le impostazioni fornite, questo processo potrebbe richiedere molto tempo.\n");
	printf("Non chiudere il programma o il download verra' interrotto e sara' irrecuperabile.\n");
	printf(ANSI_COLOR_GREEN "Avvio. . ." ANSI_COLOR_RESET "\n\n");

	// Puntatori per valori di return
	char *directDownloadLink;

	// Creo la directory, riduco le righe nello switch sottostante
	if (settings->option == 0 || settings->option == 1 || settings->option == 2)
		createDirectory(settings, name);

	// Si ottimizza il tutto in meno righe ma il funzionamento rimane lo stesso del commit precedente
	switch (settings->option) {
		case 0:	printf(ANSI_COLOR_GREEN "Download in corso di " ANSI_COLOR_CYAN "%d" ANSI_COLOR_GREEN " episodi di \"" ANSI_COLOR_CYAN "%s" ANSI_COLOR_GREEN "\"" ANSI_COLOR_RESET "\n", lastData->numberOfEpisode, name);
				break;
		
		case 1:	printf(ANSI_COLOR_GREEN "Download in corso dell'episodio " ANSI_COLOR_CYAN "%d" ANSI_COLOR_GREEN " di \"" ANSI_COLOR_CYAN "%s" ANSI_COLOR_GREEN "\"" ANSI_COLOR_RESET "\n", settings->firstEpisode + 1, name);
				break;
		
		case 2:	printf(ANSI_COLOR_GREEN "Download in corso di " ANSI_COLOR_CYAN "%d" ANSI_COLOR_GREEN " episodi [" ANSI_COLOR_CYAN "%d" ANSI_COLOR_GREEN " -> " ANSI_COLOR_CYAN "%d" ANSI_COLOR_GREEN "] di \"" ANSI_COLOR_CYAN "%s" ANSI_COLOR_GREEN "\"" ANSI_COLOR_RESET "\n", settings->secondEpisode - settings->firstEpisode, settings->firstEpisode + 1, settings->secondEpisode, name);
				break;
		
		case 3:	system(clearScreen);

				// Sistemo nome e directory finche' ho a disposizione l'accesso ai dati
				if (!settings->nameEpisodeChange)
					strcpy(settings->nameEpisode, "Episodio");

				if (strlen(settings->downloadDirectory) == 0)
					sprintf(settings->downloadDirectory + strlen(settings->downloadDirectory), "%s%s/", createPath(""), fixDirectoryName(name));

				addOnLoad(name, pageDirectLink, lastData->animeEpisodeExtension[0], settings->downloadDirectory, settings->nameEpisode);
				main();
				break;
	}

	// Download effettivo, parti comuni unite, printf() separati tramite switch sovrastante
	for (int i = settings->firstEpisode; i < settings->secondEpisode; i++) {
		printf("Download episodio %d\n", i + 1);

		// Mi faccio ritornare il link al download diretto
		directDownloadLink = getDirectEpisodeDownloadLink(pageDirectLink, lastData->animeEpisodeExtension[i]);

		// Chiamo la funzione che gestisce la creazione del comando finale da eseguire
		if (strcmp(directDownloadLink, "ERROR"))
			downloadFile(lastData, settings, directDownloadLink, name, i);
		else
			printf(ANSI_COLOR_RED "Si e' verificato un errore per l'episodio %d" ANSI_COLOR_RESET "\n", i + 1);

		// free
		free(directDownloadLink);
	}
}

void createDirectory (downloadOption *settings, char *name) {
	// Controllo se la directory e' stata modificata, in caso la creo
	char *mkdir = (char *) calloc(strlen(settings->downloadDirectory) + strlen(createPath("")) + strlen(name) + 100, sizeof(char));
	if (mkdir == NULL) {
			perror("calloc");
			_exit(-2);
	}
	
	if (strlen(settings->downloadDirectory) != 0)
		sprintf(mkdir, "mkdir \"%s\"", settings->downloadDirectory);
	else
		sprintf(mkdir, "mkdir \"%s%s\"", createPath(""), fixDirectoryName(name));
	
	// Eseguo e dealloco mkdir
	system(mkdir);
	free(mkdir);
}

char *getDirectEpisodeDownloadLink (char *pageDirectLink, char *extension) {
	// Creazione comando di download
	char *downloadCommand = (char *) calloc(strlen(extension) + strlen(pageDirectLink) + 200, sizeof(char));
	if (downloadCommand == NULL) {
		perror("calloc");
		_exit(-2);
	}

	sprintf(downloadCommand, "curl -s \"%s%s%s\" > \"%s", URL, pageDirectLink, extension, createPath("ep.txt\""));

	// Scarico il file
	system(downloadCommand);
	free(downloadCommand);

	// Apro il file, lo leggo, estraggo il contenuto, lo trasformo in matrice ed elimino il file
	char *filePath = createPath("ep.txt");
	int line = 0;

	char **fileEpisodeData = createMatrixByEscapeCharacter(extractInMemoryFromFile(filePath, true), "\n", &line);
	free(filePath);

	// Ora inizio lo script di web-scraping
	// - Ricerca di "Download Alternativo"
	int posUno = 0, posDue = 0;
	for (int i = 0; i < line; i++) {
		// Le righe hanno un minimo di lunghezza, inutile confrontare quelle minori
		if (strlen(fileEpisodeData[i]) >= 50) {
			// Controllo che mi trovi nella riga giusta contenente il link al download ( strstr() )
			if (strstr(fileEpisodeData[i], "Download Alternativo") != NULL) {
				// Riga giusta
				for (unsigned int k = 0; k < strlen(fileEpisodeData[i]); k++) {
					// Calcolo quanto e' lungo il link di download (punto di inizio, punto di fine)
					if (posUno == 0 && fileEpisodeData[i][k] == '\"') {
						posUno = k + 1;
						k = posUno + 1;
					}

					if (posDue == 0 && fileEpisodeData[i][k] == '\"')
						posDue = k;

					if (posUno != 0 && posDue != 0)
						break;
				}

				// Alloco un pointer per il link, gli copio dentro il link corretto ed eseguo il return della funzione
				char *returnLink = (char *) calloc(posDue - posUno + 1, sizeof(char));
				if (returnLink == NULL) {
					perror("calloc");
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

void downloadFile (animeEpisodeData *lastData, downloadOption *settings, char *directDownloadLink, char *name, int i) {
	// Creo un comando curl che scarica l'episodio dal link ottenuto prima in caso il valore di return sia diverso da "ERROR"
	char *downloadCommandLink = (char *) calloc(strlen(directDownloadLink) + strlen(name) + 500, sizeof(char));
	if (downloadCommandLink == NULL) {
		perror("calloc");
		_exit(-2);
	}

	// Controllo se la directory e' stata modificata
	if (strlen(settings->downloadDirectory) != 0)
		sprintf(downloadCommandLink, "curl -L -# \"%s\" > \"%s\0", directDownloadLink, settings->downloadDirectory);
	else
		sprintf(downloadCommandLink, "curl -L -# \"%s\" > \"%s%s/\0", directDownloadLink, createPath(""), fixDirectoryName(name));

	// Controllo se il nome e' stato modificato
	if (strlen(settings->nameEpisode) != 0)
		sprintf(downloadCommandLink + strlen(downloadCommandLink), "%s \0", settings->nameEpisode);
	else
		sprintf(downloadCommandLink + strlen(downloadCommandLink), "Episodio \0");

	// Gestione zeri nome del file
	if (lastData->numberOfEpisode >= 99) {
		if (i < 9)
			strcat(downloadCommandLink, "0");

		if (i < 99)
			strcat(downloadCommandLink, "0");
	}
	else if (i < 9)
		strcat(downloadCommandLink, "0");

	// Inserimento numero dell'episodio ed estensione del file
	sprintf(downloadCommandLink + strlen(downloadCommandLink), "%d.mp4", i + 1);

	// Comando di download
	system(downloadCommandLink);
	free(downloadCommandLink);
}

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