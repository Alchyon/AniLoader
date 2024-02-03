#include "struct.h"

void starting();
void optionMenu();
void retrieveCookie(cookie *, char**);

char* insertName();
void searchAnimeByName(cookie, char *);
int searchAnimeDiv(char **, int);
animeSearchData *extractAnimeName(char **, int, int);
void convertAnimeName(animeSearchData *);
bool printFindAnime(animeSearchData *);
int selectAnime(animeSearchData *);
char *downloadRedirectPage(animeSearchData *, cookie, int);
char *getPageLink(char *);
char *downloadCorrectPage(cookie, char *);
animeEpisodeData *getEpisodeExtension(char **, int);
downloadOption *downloadMenu(char *, int, int, char **);

// Funzioni per il download
void downloadPrepare(animeEpisodeData *, downloadOption *, cookie, char *, char *, int, char **);
void createDirectory(downloadOption *, char *);
char *getDirectEpisodeDownloadLink(cookie, char *, char *);
void downloadFile(animeEpisodeData *, downloadOption *, cookie, char *, char *, int);

// Aggiunta a CFU, funzione a parte per via della complessita' della stessa
void addOnLoad(char *, char *, char *, char *, char *, int, char **);