#include "libraries.h"
#include "function.h"
#include "utilities.h"
#include "cfu.h"

// exit() values:
// 0 -> terminato con successo, qualunque sia la causa
// 1 -> errore apertura file
// 2 -> errore allocazione memoria
// 3 -> OS non supportato
// 4 -> errore di cURL durante la generazione del cookie

// Variabile globale utilizzata per i COOKIE
char *COOKIECMD = NULL;

bool generateCookieFile () {
	int maximumRetry = 3;

	char *path = (char *) calloc(strlen(APPDATA) + 100, sizeof(char));
	if (path == NULL) {
		perror("calloc");
		exit(2);
	}

	do {
		// Creazione percorso del file
		sprintf(path, "%s\\AniLoader\\%s", APPDATA, COOKIEFILE);

		// Viene effettuato un controllo sull'esistenza del file
		FILE *f = fopen(path, "r");
		if (f != NULL) {
			fclose(f);
			free(path);

			// Istanzia il cookie
			cookieInitialization();
			return true;
		}

		// Se il file non esiste, si forza una richiesta esclusivamente per generarlo
		sprintf(path, "curl --cookie-jar \"%s\\AniLoader\\%s\" -s %s -o NUL", APPDATA, COOKIEFILE, URL);
		system(path);
	}
	while (maximumRetry-- != 0);

	return false;
}

// Questa funzione e' necessaria per inizializzare la variabile COOKIECMD a livello globale, visibile su tutti i file,
// utilizzata per calcolare una volta soltanto tutto il papiro da passare al comando
void cookieInitialization() {
	// Allocazione dinamica della memoria per il comando
	COOKIECMD = (char *) calloc(1024, sizeof(char));
	if (COOKIECMD == NULL) {
		fprintf(stderr, "Errore nell'allocazione della memoria.\n");
		exit(2);
	}

	// Inizializzazione del comando
	INIT_COOKIESCRIPT(COOKIECMD, APPDATA);
}

void starting () {
	system(clearScreen);
	printf(
		"########################################################\n"
		"#                                                      #\n"
		"#                      " ANSI_COLOR_GREEN "Ani-Loader" ANSI_COLOR_RESET "                      #\n"
		"#                       v. 1.8.3                       #\n"
		"#                      15/05/2025                      #\n"
		"#                                                      #\n"
		"#            </>     - " ANSI_COLOR_GREEN "By Alchyon" ANSI_COLOR_RESET " -     </>            #\n"
		"#     " ANSI_COLOR_CYAN "GitHub" ANSI_COLOR_RESET ": " ANSI_COLOR_CYAN "https://github.com/Alchyon/AniLoader" ANSI_COLOR_RESET "     #\n"
		"#                                                      #\n"
		"########################################################\n"
	);

	printf("\n");
}

void optionMenu () {
	char option;

	// Il ciclo infinito e' preferibile alla ricorsione per evitare dei problemi di overflow
	while (1) {
		printf(
			"########################################################\n"
			"#" ANSI_COLOR_GREEN "                Impostazioni generali                 " ANSI_COLOR_RESET "#\n"
			"########################################################\n"
			"#                                                      #\n"
			"#" ANSI_COLOR_YELLOW "  INVIO" ANSI_COLOR_RESET " -- > Cerca un anime                           #\n"
			"#" ANSI_COLOR_YELLOW "  1" ANSI_COLOR_RESET " -- > Controlla l'ultimo changelog                 #\n"
			"#" ANSI_COLOR_YELLOW "  2" ANSI_COLOR_RESET " -- > Visualizza o elimina gli anime preferiti     #\n"
			"#" ANSI_COLOR_YELLOW "  3" ANSI_COLOR_RESET " -- > Controlla gli aggiornamenti per i preferiti  #\n"
			"#" ANSI_COLOR_YELLOW "  ESC" ANSI_COLOR_RESET " -- > Esci dal programma                         #\n"
			"#                                                      #\n"
			"########################################################\n"
			"Scegliere opzione: "
		);

		do option = getch();
		while (option != 13 && option != 27 && option != '1' && option != '2' && option != '3' && option != EOF);

		system(clearScreen);
		switch (option) {
			case '1':	changelog();
						break;

			case '2':	libraryOption();
						break;

			case '3':	printf(ANSI_COLOR_YELLOW "Controllo aggiornamenti in corso...\n" ANSI_COLOR_RESET);
						CheckForUpdatesRoutine();
						printf(ANSI_COLOR_GREEN "Aggiornamento completato.\n" ANSI_COLOR_RESET);
						break;

			case 27:	exit(0);
			default:	break;
		}

		// Eseguito solo se l'utente ha scelto di effettuare una qualsiasi operazione
		if (option == '1' || option == '2' || option == '3') {
			printf("Premere INVIO per continuare...");
			while ((option = getch()) != 13 && option != EOF);
			
			starting();
		}
		else
			// Eseguito solo se l'utente ha premuto INVIO '\n' oppure '\r'
			break;
	}
}

char *insertName () {
	// Allocazione memoria per il nome
	char *name = (char *) calloc(101, sizeof(char));
	if (name == NULL) {
		perror("calloc");
		exit(2);
	}

	// Pulizia dello schermo per l'inserimento del nome
	system(clearScreen);

	// Inserimento del nome dell'anime [max 100 char]
	printf("Inserire il nome dell'anime da ricercare: " ANSI_COLOR_YELLOW);
	fgets(name, 100, stdin);
	printf(ANSI_COLOR_RESET);

	// Rimuove il carattere '\n' finale, se presente
	size_t len = strlen(name);
	if (len > 0 && name[len - 1] == '\n')
		name[len - 1] = '\0';

	// Essendo una richiesta HTTP, il nome non puo' contenere spazi che verranno quindi rimpiazzati dal segno '+'
	for (unsigned int i = 0; i < len; i++) {
		if (name[i] == ' ')
			name[i] = '+';
	}

	return name;
}

void searchAnimeByName (char* name) {
	// Creazione comando
	char *command = (char *) calloc(strlen(COOKIECMD) + strlen(name) + 300, sizeof(char));
	if (command == NULL) {
		perror("calloc");
		exit(2);
	}

	sprintf(command, "curl %s -s \"%s/search?keyword=%s\" > \"%s\"", COOKIECMD, URL, name, createPath("search.txt"));

	system(command);
	free(command);
}

int searchAnimeDiv (char **searchDataResult, int line) {
	// Ricerca la linea della matrice in cui si trova l'inizio dell'HTML contenente i nomi degli anime
	for (int i = 0; i < line; i++)
		if (strstr(searchDataResult[i], "<div class=\"film-list\">") != NULL)
			return i + 3;

	return -1;
}

animeSearchData *extractAnimeName (char **searchDataResult, int line, int divFound) {
	// Allocazione struttura e puntatori interni, controllo errori
	animeSearchData *asd = (animeSearchData *) malloc(sizeof(animeSearchData));
	if (asd == NULL) {
		perror("malloc");
		exit(2);
	}

	asd->findAnimeLink = (char **) malloc(sizeof(char *) * 40);
	asd->correctAnimeName = (char **) malloc(sizeof(char *) * 40);

	if (asd->findAnimeLink == NULL || asd->correctAnimeName == NULL) {
		perror("malloc");
		exit(2);
	}

	// Inizializza a 0 il numero di anime trovati
	asd->numberOfAnime = 0;

	// La ricerca comincia da divFound in modo da skippare una buona parte di righe gia' scansionate
	for (int i = divFound; i < line; i++) {
		// Controlla la posizione del div corretto
		char *poster = strstr(searchDataResult[i], "\" class=\"poster\"");
		if (poster != NULL) {
			// La stringa interessata si trova tra due "<stringa>", quindi si puo' usare strchr() per ricavarne
			// direttamente le posizioni
			char *linkStart = strchr(searchDataResult[i], '\"') + 1;
			char *linkEnd = strchr(linkStart, '\"');

			if (linkStart && linkEnd) {
				// Termina la stringa qui
				*linkEnd = '\0';
				// Clona la parte interessata all'interno della struttura
				asd->findAnimeLink[asd->numberOfAnime] = strdup(linkStart);
				// Ripristina il carattere originale
				*linkEnd = '\"';
			} else
				// Passa alla riga successiva in caso di errore
				continue;

			// Controlla la riga successiva per il nome dell'anime
			char *alter = strstr(searchDataResult[i + 1], "alt=\"");
			if (alter != NULL) {
				// Stesso algoritmo utilizzato per la parte soprastante
				char *nameStart = strchr(alter, '\"') + 1;
				char *nameEnd = strchr(nameStart, '\"');

				if (nameStart && nameEnd) {
					*nameEnd = '\0';
					asd->correctAnimeName[asd->numberOfAnime] = strdup(nameStart);
					*nameEnd = '\"';
				}
			}

			// Incrementa il numero di anime trovati
			asd->numberOfAnime++;
		}
	}	

	// Restituisce la struttura (link, nomi anime trovati, numero di anime trovati)
	return asd;
}

void convertAnimeName(animeSearchData *baseData) {
	char *toConvert = NULL;
	char *temp = NULL;
	
	for (int count = 0; count < baseData->numberOfAnime; count++) {
		// Allocazione memoria per il buffer temporaneo
		toConvert = (char *) realloc(toConvert, sizeof(char) * (strlen(baseData->correctAnimeName[count]) + 10));
		if (toConvert == NULL) {
			perror("realloc");
			exit(2);
		}

		// Inizializzazione del buffer temporaneo
		toConvert[0] = '\0';

		// Crea una copia temporanea della stringa originale
		temp = strdup(baseData->correctAnimeName[count]);
		if (temp == NULL) {
			perror("strdup");
			free(toConvert);
			exit(2);
		}

		// Analizza la stringa e sostituisce le sequenze HTML
		char *token = strtok(temp, "&;");
		while (token != NULL) {
			if (strcmp(token, "quot") == 0)
				strcat(toConvert, "\"");
			else if (strcmp(token, "amp") == 0)
				strcat(toConvert, "&");
			else if (strcmp(token, "#x27") == 0)
				strcat(toConvert, "'");
			else
				strcat(toConvert, token);

			token = strtok(NULL, "&;");
		}

		// Copia il risultato finale nella stringa originale
		strcpy(baseData->correctAnimeName[count], toConvert);

		// Libera la memoria temporanea
		free(temp);
	}

	// Libera il buffer temporaneo
	free(toConvert);

	// TO-DO, Unicode into ASCII, HTML parser needed
	/* ÔÇÖ = ' */
	/* ÔÇ£ = “ */
	/* ÔÇØ = ” */
	/* ÔÇô = - */
	/* ├╣ = ù */
}


bool printFindAnime (animeSearchData *baseData) {
	// Stampo un errore se gli episodi sono 0
	if (baseData->numberOfAnime < 1) {
		printf(
			ANSI_COLOR_RED "Errore: nessun anime trovato..." ANSI_COLOR_RESET "\n\n"
			"Questo puo' accadere quando, all'inserimento del nome, MANCANO o SONO PRESENTI caratteri aggiuntivi\n"
			"come spazi, trattini, punti o virgole. Si consiglia di riprovare inserendo meno caratteri o eventualmente\n"
			"usando il nome in giapponese, se nonostante cio' non verranno trovati risultati, allora significa che l'anime\n"
			"non e' presente o e' codificato in un formato sconosciuto.\n"
			ANSI_COLOR_YELLOW "Premere un tasto per riavviare il programma . . ." ANSI_COLOR_RESET
		);
		getch();

		return false;
	}
	else {
		// Stampa gli anime trovati se sono 1 o piu'
		printf(ANSI_COLOR_GREEN "Anime trovati: \n\n" ANSI_COLOR_RESET);
		for (int count = 0; count < baseData->numberOfAnime; count++) {
			// Inserisce uno spazio se il numero è minore di 10 ma il totale e' maggiore cosi' da allineare le scritte
			// Gestito direttamente da %2d
			printf(ANSI_COLOR_YELLOW "%2d" ANSI_COLOR_RESET "] %s \n", count, baseData->correctAnimeName[count]);
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
			system(clearScreen);
			printf("Valore non ammesso, riprovare...\n");
			printFindAnime(baseData);
		}

		printf("\nQuale anime vuoi scaricare? Digita il numero a lato oppure inserisci -1 per riavviare il programma!\nScelta: ");
		printf(ANSI_COLOR_YELLOW);
		scanf("%d", &selected);
		printf(ANSI_COLOR_RESET);
		fflush(stdin);

		// Input inserito almeno una volta, in caso di errore ristampa gli anime
		enter = true;

		// Si effettua direttamente il return e si gestisce l'eccezione dal main, si possiede accesso a piu' variabili
		if (selected == -1)
			return -1;

	} while (selected < 0 || selected > baseData->numberOfAnime - 1);

	return selected;
}

char *downloadRedirectPage (animeSearchData *baseData, int selected) {
	char *command = (char *) calloc(strlen(COOKIECMD) + strlen(baseData->findAnimeLink[selected]) + 300, sizeof(char));
	if (command == NULL) {
		perror("calloc");
		exit(2);
	}

	// Concateno in una stringa il comando da eseguire
	sprintf(command, "curl %s -s \"%s%s\" > \"%s\"", COOKIECMD ,URL, baseData->findAnimeLink[selected], createPath("redirect.txt"));

	system(command);
	free(command);

	return "redirect.txt";
}

char *getPageLink (char *redirectContent) {
	char *momentCopy = (char *) calloc(strlen(redirectContent) + 1, sizeof(char));
	if (momentCopy == NULL) {
		perror("calloc");
		exit(2);
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
	char *command = (char *) calloc(strlen(COOKIECMD) + strlen(pageDirectLink) + 300, sizeof(char));
	if (command == NULL) {
		perror("calloc");
		exit(2);
	}

	sprintf(command, "curl %s -s \"%s%s\" > \"%s\"", COOKIECMD, URL, pageDirectLink, createPath("page.txt"));

	system(command);
	free(command);

	return "page.txt";
}

animeEpisodeData *getEpisodeExtension (char **pageDataResult, int line) {
	int lineCounter = 0;
	char *moment;

	// Dichiara e alloca la struttura che conterra' i dati calcolati dalla funzione
	animeEpisodeData *episodeData = (animeEpisodeData *) malloc(sizeof(animeEpisodeData));
	if (episodeData == NULL) {
		perror("malloc");
		exit(2);
	}

	// TO-DO Si potrebbe ottimizzare con un doppio giro, lettura del numero totale > allocazione > salvataggio o con numerose realloc()
	// One Piece e Detective Conan hanno 1000+ episodi e sono gli anime con il numero maggiore. Il 1500 sottostante e' dovuto alla loro esistenza.
	// 20/04/2025, record: 1155 in Detective Conan
	episodeData->animeEpisodeExtension = (char **) malloc(sizeof(char *) * 1500);
	if (episodeData->animeEpisodeExtension == NULL) {
		perror("malloc");
		exit(2);
	}

	episodeData->numberOfEpisode = 0;

	// Inizio calcoli su ogni riga del file letto
	// Skippo le prime 200 dato che sono header inutili, computazione in meno da fare
	for (lineCounter = 200; lineCounter < line; lineCounter++) {
		// Mancanza server di AnimeWorld > errore a prescindere
		char *server = strstr(pageDataResult[lineCounter], serverID);
		if (server != NULL)
			break;
	}

	// Server non trovato
	if (lineCounter == line) {
		// Si controlla lo stato, se non e' ancora stato rilasciato, allora non si tratta
		// di un errore ma solo di una mancanza di episodi.
		// Per saperlo bisogna fare un ulteriore ciclo su tutta la matrice, non efficiente ma funzionante
		char **animeStatus = getAnimeStatus(pageDataResult, line, 200);
		system(clearScreen);

		if (strcmp(animeStatus[1], "non rilasciato") == 0) {
			printf(ANSI_COLOR_YELLOW "Anime non ancora rilasciato!" ANSI_COLOR_RESET "\n");
			printf("La data di rilascio prevista e': " ANSI_COLOR_CYAN "%s" ANSI_COLOR_RESET ".\n", animeStatus[2]);
			printf("Sono previsti " ANSI_COLOR_CYAN "%s" ANSI_COLOR_RESET " episodi.\n", animeStatus[0]);
		}
		else
		{
			printf(ANSI_COLOR_YELLOW "Stato dell'anime sconosciuto, impossibile recuperare informazioni in merito!" ANSI_COLOR_RESET "\n");
			printf(ANSI_COLOR_RED "Errore: impossibile scaricare attraverso questo programma..." ANSI_COLOR_RESET "\n");
		}

		printf("Premere un tasto qualsiasi per riavviare il programma...");
		getch();

		episodeData->numberOfEpisode = -1;

		return episodeData;
	}

	// Server trovato, ricerca div
	for (; lineCounter < line; lineCounter++) {
		char *serverCheck = strstr(pageDataResult[lineCounter], "data-name=\"" serverNumber "\" data-type=\"iframe\" data-id=\"9\"");
		if (serverCheck != NULL)
			break;
	}

	// div trovato, estrazione degli ID
	for (; lineCounter < line; lineCounter++) {
		char *dataID = strstr(pageDataResult[lineCounter], "<a data-episode-id=\"");
		if (dataID != NULL) {
			// != NULL > episodio trovato
			// Rialloco lo spazio necessario a moment per contenere i dati
			moment = (char *) realloc(moment, sizeof(char) * (strlen(pageDataResult[lineCounter]) + 1));
			strcpy(moment, pageDataResult[lineCounter]);

			// Forzo lo script a raggiungere la parte interessata dalle " (virgolette)
			// Sono necessari cinque cicli
			char *token = strtok(moment, "\"");
			for (int i = 0; i < 5; i++, token = strtok(NULL, "\""));

			// Copia l'ID dal tag "data-id"
			episodeData->animeEpisodeExtension[episodeData->numberOfEpisode] = strdup(token);
			if (episodeData->animeEpisodeExtension[episodeData->numberOfEpisode] == NULL) {
				perror("strdup");
				exit(2);
			}

			// Incrementa il numero di episodi
			episodeData->numberOfEpisode++;

			// Controllo se sono presenti ulteriori div successivi
			if (strstr(pageDataResult[lineCounter + 1], "</ul>") != NULL && strstr(pageDataResult[lineCounter + 2], "<a data-episode-id=\"") == NULL) {
				// In caso negativo, termino il ciclo, mi salvo il valore di "lineCounter" per getAnimeStatus(...);
				episodeData->rLine = lineCounter;
				lineCounter = line;

				free(moment);
				break;
			}
		}
	}

	return episodeData;
}

char **getAnimeStatus(char **pageDataResult, int line, int startingPoint) {
	char **data = (char **) calloc(3, sizeof(char *));
	if (data == NULL) {
		perror("calloc");
		exit(2);
	}

	// Episodi, 6 caratteri = 10^6 - 1, +1 per '\0'
	data[0] = calloc(6, sizeof(char));
	// Stato, lunghezza massima = 15, +1 per '\0'
	data[1] = calloc(15, sizeof(char));
	// Data di uscita, arrotondato a 20
	data[2] = calloc(20, sizeof(char));
	if (data[0] == NULL || data[1] == NULL || data[2] == NULL) {
		perror("calloc");
		exit(2);
	}

	// Inizializzazione, 0 significa errore o dati incompleti
	// data[0] -> numero di episodi previsti
	// data[1] -> stato dell'anime, "In corso", "Completato", "Non rilasciato", "Droppato" ... "0" (sconosciuto)
	// data[2] -> data di uscita in parole
	strcpy(data[0], "??");
	strcpy(data[2], "0");

	// Inizio calcoli su ogni riga del file
	// Skippo le prime "startingPoint" dato che sono gia' state lette, computazione in meno da fare
	// Mi fermo quando trovo lo stato dell'anime a cui sono interessato
	for (int i = startingPoint; i < line; i++) {
		if (strstr(pageDataResult[i], "Data di Uscita:") != NULL) {
			char *token = strtok(pageDataResult[i + 1], "<>");
			token = strtok(NULL, "<>");
			token = strtok(NULL, "<>");

			// Previene errori di buffer overflow
			strncpy(data[2], token, 19);
			break;
		}
	}

	for (int i = startingPoint; i < line; i++) {
		if (strstr(pageDataResult[i], "Episodi:") != NULL) {
			// Se sono qui, +1, si estrae il numero di episodi
			for (int k = 0, startingPoint = 0; startingPoint < strlen(pageDataResult[i + 1]); startingPoint++)
				if (isdigit(pageDataResult[i + 1][startingPoint])) {
					data[0][k++] = pageDataResult[i + 1][startingPoint];
					data[0][k] = '\0';
				}

			// Se sono qui, +4 e si estrae lo stato
			// Se non corrisponde, probabilmente esiste uno stato nuovo oppure c'e' stato un errore
			// Necessario per avere i nomi in lower-case e avere un print pulito
			const char *status[] = {"In corso", "Finito", "Non rilasciato", "Droppato"};
			const char *result[] = {"in corso", "finito", "non rilasciato", "droppato"};
			bool find = false;

			for (int j = 0; j < 4; j++) {
				if (strstr(pageDataResult[i + 4], status[j]) != NULL) {
					strcpy(data[1], result[j]);
					find = true;
					break;
				}
			}

			// Stato sconosciuto, non dovrebbe mai apparire
			if (!find)
				strcpy(data[1], "sconosciuto");

			break;
		}
	}

	return data;
}

downloadOption *downloadMenu (char *name, int numberOfEpisode, char **animeStatus) {
	// Allocazione struttura per valore di return
	downloadOption *option = (downloadOption *) malloc(sizeof(downloadOption));
	if (option == NULL) {
		perror("malloc");
		exit(2);
	}

	// Setto i parametri che necessitano di inizializzazione
	option->nameEpisodeChange = false;
	option->firstEpisode = -1;
	option->secondEpisode = -1;

	system(clearScreen);
	printf(
		"Anime selezionato: " ANSI_COLOR_GREEN "%s\n" ANSI_COLOR_RESET
		"Episodi:" ANSI_COLOR_CYAN " %d " ANSI_COLOR_RESET "usciti su" ANSI_COLOR_GREEN " %s " ANSI_COLOR_RESET "previsti\n"
		"Stato dell'anime: " ANSI_COLOR_GREEN "%s\n" ANSI_COLOR_RESET
		"Data di uscita: " ANSI_COLOR_GREEN "%s\n\n" ANSI_COLOR_RESET,
		name, numberOfEpisode, animeStatus[0], animeStatus[1], animeStatus[2]
	);

	printf(
		"Come si desidera procedere?\n"
		ANSI_COLOR_YELLOW "INVIO" ANSI_COLOR_RESET " -- > Scarica tutti gli episodi\n"
		ANSI_COLOR_YELLOW "1" ANSI_COLOR_RESET "     -- > Scarica un singolo episodio\n"
		ANSI_COLOR_YELLOW "2" ANSI_COLOR_RESET "     -- > Scarica un range di episodi\n"
		ANSI_COLOR_YELLOW "3" ANSI_COLOR_RESET "     -- > Aggiungi l'anime ai preferiti\n"
		ANSI_COLOR_YELLOW "ESC" ANSI_COLOR_RESET "   -- > Riavvia il programma\n"
		ANSI_COLOR_YELLOW "QUALSIASI ALTRO TASTO" ANSI_COLOR_RESET " -- > Esci dal programma\n"
	);

	printf("Scelta: ");
	char choice = getch();

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

					// Se i due numeri sono uguali e' come dire di scaricare solo quello
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
					main();
					break;

		default:	system(clearScreen);
					exit(0);
	}

	// Allocazione pointer nome e directory
	option->nameEpisode = (char *) calloc(101, sizeof(char));
	option->downloadDirectory = (char *) calloc(501, sizeof(char));
	if (option->nameEpisode == NULL || option->downloadDirectory == NULL) {
		perror("calloc");
		exit(2);
	}

	// Modifica nome dei singoli episodi
	do {
		system(clearScreen);
		printf("Vuoi cambiare il modo in cui gli episodi verranno salvati?\n");
		printf("Di default saranno \"" ANSI_COLOR_GREEN "Episodio XXX" ANSI_COLOR_RESET "\", tu puoi cambiare il nome di episodio.\n");
		printf("Scelta [" ANSI_COLOR_GREEN "Y" ANSI_COLOR_RESET "/" ANSI_COLOR_RED "N" ANSI_COLOR_RESET "]: ");
		choice = toupper(getch());
	} while (choice != 'Y' && choice != 'N');

	// Pulisce il flusso di ingresso per prevenire problemi di input
	fflush(stdin);

	// Modifica nome confermata
	if (choice == 'Y') {
		printf("\nInserire il nuovo nome da utilizzare [massimo 100 caratteri]: ");
		printf(ANSI_COLOR_YELLOW);
		fgets(option->nameEpisode, 100, stdin);
		printf(ANSI_COLOR_RESET);

		// Utilizzo "Episodio" se non e' stato inserito un nome
		if (strlen(option->nameEpisode) == 1)
			strcpy(option->nameEpisode, "Episodio");
		else {
			option->nameEpisodeChange = true;

			// Correggo eventuali caratteri speciali non ammessi nei nomi ed elimino lo '\n' finale
			for (unsigned int i = 0; i < strlen(option->nameEpisode); i++) {
				if (option->nameEpisode[i] == '\n')
					option->nameEpisode[i] = '\0';
				else if (strchr("\\/:*?\"<>|{}~", option->nameEpisode[i]))
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

	// Pulisce il flusso di ingresso per prevenire problemi di input
	fflush(stdin);

	// Modifica directory confermata
	if (choice == 'Y') {
		printf("\nInserite il percorso completo della nuova directory [massimo 400 caratteri]: " ANSI_COLOR_YELLOW);
		fgets(option->downloadDirectory, 400, stdin);
		printf(ANSI_COLOR_RESET);

		// Utilizzo la directory di default se non e' stata modificata, premuto '\n'
		// Passo "" in modo che la stringa di return contenga solo il path assoluto della cartella di download
		size_t len = strlen(option->downloadDirectory);
		if (len == 1)
			option->downloadDirectory[0] = '\0';
		else {
			// Controllo terminazione con slash o un backslash e controllo presenza di caratteri non ammessi, in questo caso si ripristina
			// quella di default, piu' pratico del chiedere un nuovo inserimento, verra' implementato in un futuro
			if (option->downloadDirectory[len - 1] != '\\' && option->downloadDirectory[len - 1] != '/')
				option->downloadDirectory[len - 1] = '/';

			for (unsigned int i = 0; i < len; i++) {
				// Controllo sui caratteri non ammessi
				if (strchr("*?\"<>|{}~", option->downloadDirectory[i])) {
					printf(ANSI_COLOR_RED "\nErrore!\n" ANSI_COLOR_RESET);
					printf("Il nome della directory contiene dei caratteri non ammessi, ripristino directory di default!\n");
					system("pause");

					option->downloadDirectory[0] = '\0';
					choice = 'E';
					break;
				}
			}
		}
	}
	else
		option->downloadDirectory[0] = '\0';

	return option;
}

// Funzioni che servono a ottenere informazioni e dati necessari al solo download
void downloadPrepare (animeEpisodeData *lastData, downloadOption *settings, char *pageDirectLink, char *name, cfuFile *cfu) {
	// Correggo il directLink eliminando l'estensione
	int i = strlen(pageDirectLink);
	for (; pageDirectLink[i] != '/'; i--);
	pageDirectLink[i + 1] = '\0';

	// Inizio switch per processo download
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

	// Eseguito una volta a monte e riciclato
	char *dirPassing = fixDirectoryName(name);

	// Download effettivo, parti comuni unite, printf() separati tramite switch sovrastante
	for (int i = settings->firstEpisode; i < settings->secondEpisode; i++) {
		printf("Download episodio %d\n", i + 1);

		// Mi faccio ritornare il link al download diretto
		directDownloadLink = getDirectEpisodeDownloadLink(pageDirectLink, lastData->animeEpisodeExtension[i]);

		// Chiamo la funzione che gestisce la creazione del comando finale da eseguire
		if (strcmp(directDownloadLink, "ERROR") != 0) {
			downloadFile(lastData, settings, directDownloadLink, dirPassing, i);

			// Se si sta effettuando un aggiornamento dei preferiti, allora il file contentente
			// i dati di quell'anime va aggiornato al termine del download di ogni episodio.
			if (cfu != NULL)
				updateCfuFile(cfu, i + 1);
		}
		else
			printf(ANSI_COLOR_RED "Si e' verificato un errore per l'episodio %d" ANSI_COLOR_RESET "\n", i + 1);

		// free
		free(directDownloadLink);
	}
}

void createDirectory (downloadOption *settings, char *name) {
	// Controllo se la directory e' stata modificata, se non esistente, la creo
	char *dirPath = (char *) calloc(strlen(settings->downloadDirectory) + strlen(createPath("")) + strlen(name) + 100, sizeof(char));
	if (dirPath == NULL) {
			perror("calloc");
			exit(2);
	}

	// Impostazione della directory, funzione mkdir()
	if (strlen(settings->downloadDirectory) != 0)
		sprintf(dirPath, "%s", settings->downloadDirectory);
	else
		sprintf(dirPath, "%s%s", createPath(""), fixDirectoryName(name));

	// Esecuzione e free()
	mkdir(dirPath);
	free(dirPath);
}

char *getDirectEpisodeDownloadLink (char *pageDirectLink, char *extension) {
	// Creazione comando di download
	char *downloadCommand = (char *) calloc(strlen(COOKIECMD) + strlen(pageDirectLink) + strlen(extension) + 300, sizeof(char));
	if (downloadCommand == NULL) {
		perror("calloc");
		exit(2);
	}

	sprintf(downloadCommand, "curl %s -s \"%s%s%s\" -o \"%s\"", COOKIECMD, URL, pageDirectLink, extension, createPath("ep.txt"));

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
	for (int i = 0; i < line; i++) {
		// Le righe hanno un minimo di lunghezza, inutile confrontare quelle minori
		// Controllo riga corretta contenente il link al download ( strstr() )
		if (strlen(fileEpisodeData[i]) >= 50 && strstr(fileEpisodeData[i], "Download Alternativo") != NULL) {
			// Estrae il link che si trova tra le virgolette
			char *start = strchr(fileEpisodeData[i], '\"') + 1;
			char *end = strchr(start, '\"');

			if (start != NULL && end != NULL) {
				// Alloca e duplica il link ( safe - strndup() )
				size_t len = end - start;
				char *returnLink = strndup(start, len);
				if (returnLink == NULL) {
					perror("strndup");
					exit(2);
				}

				// Libera la memoria e restituisce il link
				for (int j = 0; j < line; free(fileEpisodeData[j++]));
				free(fileEpisodeData);

				return returnLink;
			}
		}
	}

	free(fileEpisodeData);
	return "ERROR";
}

void downloadFile (animeEpisodeData *lastData, downloadOption *settings, char *directDownloadLink, char *dirPassing, int i) {
	// Crea un comando curl che scarica l'episodio dal link ottenuto prima in caso il valore di return sia diverso da "ERROR"
	char *downloadCommandLink = (char *) calloc(strlen(COOKIECMD) + strlen(directDownloadLink) + strlen(settings->downloadDirectory) + strlen(settings->nameEpisode) + strlen(dirPassing) + 200, sizeof(char));
	if (downloadCommandLink == NULL) {
		perror("calloc");
		exit(2);
	}

	// Controllo se la directory e' stata modificata
	if (strlen(settings->downloadDirectory) != 0)
		sprintf(downloadCommandLink, "curl %s -L -# \"%s\" -o \"%s\0", COOKIECMD, directDownloadLink, settings->downloadDirectory);
	else
		sprintf(downloadCommandLink, "curl %s -L -# \"%s\" -o \"%s%s/\0", COOKIECMD, directDownloadLink, createPath(""), dirPassing);

	// Controllo se il nome e' stato modificato
	if (settings->nameEpisodeChange)
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
	sprintf(downloadCommandLink + strlen(downloadCommandLink), "%d.mp4\0", i + 1);

	// Comando di download
	system(downloadCommandLink);
	free(downloadCommandLink);
}