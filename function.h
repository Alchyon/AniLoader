int main();
void starting();
void optionMenu();

char *insertName();
void searchAnimeByName(cookie *, char *);
int searchAnimeDiv(char **, int);
animeSearchData *extractAnimeName(char **, int, int);
void convertAnimeName(animeSearchData *);
bool printFindAnime(animeSearchData *);
int selectAnime(animeSearchData *);
char *downloadRedirectPage(animeSearchData *, cookie *, int);
char *getPageLink(char *);
char *downloadCorrectPage(cookie *, char *);
animeEpisodeData *getEpisodeExtension(char **, int);

// Funzione utilizzata per ottenere i cookie
cookie *getCookie(char *);

// Funzione usata per ottenere informazioni sullo stato dell'anime:
// episodi previsti, stato, righe gia' lette
char **getAnimeStatus(char**, int, int);

// Funzioni per il download
downloadOption *downloadMenu(char *, int, char **);
void downloadPrepare(animeEpisodeData *, downloadOption *, cookie *, char *, char *);
void createDirectory(downloadOption *, char *);
char *getDirectEpisodeDownloadLink(cookie *, char *, char *);
void downloadFile(animeEpisodeData *, downloadOption *, cookie *, char *, char *, int);