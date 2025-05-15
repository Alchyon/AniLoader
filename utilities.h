void changelog();
long int findSize(char *);
char *createPath(char *);
char *extractInMemoryFromFile(char *, bool);
char **createMatrixByEscapeCharacter(char *, char *, int *);
char *fixDirectoryName(char *);

// Funzioni di libreria inserite a causa di problemi con il compilatore.
// Verranno rimosse in futuro
char *strndup(const char*, size_t);