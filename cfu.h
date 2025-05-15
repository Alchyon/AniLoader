// Aggiunta ai preferiti
void addOnLoad(char *, char *, char *, char *, char *);

// Funzione di base per il controllo dei preferiti
void CheckForUpdatesRoutine();
void updateCfuFile (cfuFile *, int);

// Altre funzioni che hanno, in qualche modo, un collegamento con CFU/preferiti
library *getLibrary();
void printLibrary();
void libraryOption();
void delLibrary(library *, int);