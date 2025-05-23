// Funzione utilizzata per ottenere i cookie
bool generateCookieFile();
void cookieInitialization();

// Inizio codice reale
int main();
void starting();
void optionMenu();

char *insertName();
void searchAnimeByName(char *);
int searchAnimeDiv(char **, int);
animeSearchData *extractAnimeName(char **, int, int);
void convertAnimeName(animeSearchData *);
bool printFindAnime(animeSearchData *);
int selectAnime(animeSearchData *);
char *downloadRedirectPage(animeSearchData *, int);
char *getPageLink(char *);
char *downloadCorrectPage(char *);
animeEpisodeData *getEpisodeExtension(char **, int);

// Funzione usata per ottenere informazioni sullo stato dell'anime:
// episodi previsti, stato, righe gia' lette
char **getAnimeStatus(char**, int, int);

// Funzioni per il download
downloadOption *downloadMenu(char *, int, char **);
void downloadPrepare(animeEpisodeData *, downloadOption *, char *, char *, cfuFile *);
void createDirectory(downloadOption *, char *);
char *getDirectEpisodeDownloadLink(char *, char *);
void downloadFile(animeEpisodeData *, downloadOption *, char *, char *, int);