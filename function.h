#include "struct.h"

int main();
void starting();
void optionMenu();

char* insertName();
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
downloadOption *downloadMenu(char *, int);

// Funzioni per il download
void downloadPrepare(animeEpisodeData *, downloadOption *, char *, char *);
void createDirectory(downloadOption *, char *);
char *getDirectEpisodeDownloadLink(char *, char *);
void downloadFile(animeEpisodeData *, downloadOption *, char *, char *, int);