#include "struct.h"

void starting();
void optionMenu();

void searchAnimeByName();
int searchAnimeDiv(char **, int);
animeSearchData *extractAnimeName(char **, int, int);
void convertAnimeName(animeSearchData *);
bool printFindAnime(animeSearchData *);
int selectAnime(animeSearchData *);
char *downloadRedirectPage(animeSearchData *, int);
char *getPageLink(char *);
char *downloadCorrectPage(char *);
animeEpisodeData *getEpisodeExtension(char **, int);
downloadOption *downloadMenu(char *, int);

// Funzioni per il download
void downloadPrepare(animeEpisodeData *, downloadOption *, char *, char *);
char *getDirectEpisodeDownloadLink(char *, char *);
void downloadFile(animeEpisodeData *, downloadOption *, char *, char *, char *, int);