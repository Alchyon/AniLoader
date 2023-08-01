#ifdef _WIN32
	char* getlogin();
#endif

void changelog();
long int findSize(char *);
char* createPath(char *);
char* extractInMemoryFromFile(char *);
char** createMatrixByEscapeCharacter(char *, char *, int *);
char* fixDirectoryName(char *);