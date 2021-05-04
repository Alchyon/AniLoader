#include "struct.h"

void starting();
void optionMenu();
void retrieveCookie(cookie *, char**, int);

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
downloadOption *downloadMenu(char *, int);

// Funzioni per il download
void downloadPrepare(animeEpisodeData *, downloadOption *, cookie, char *, char *);
char *getDirectEpisodeDownloadLink(cookie, char *, char *);
void downloadFile(animeEpisodeData *, downloadOption *, cookie, char *, char *, char *, int);