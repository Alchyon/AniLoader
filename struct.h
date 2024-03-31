typedef struct {
	char *name;
	char *token;
} cookie;

typedef struct {
	char **findAnimeLink;
	char **correctAnimeName;
	int numberOfAnime;
} animeSearchData;

typedef struct {
	char **animeEpisodeExtension;
	int numberOfEpisode;

	// Questo valore serve ESCLUSIVAMENTE per evitare di rileggere le righe
	// passate, serve SOLO per la funzione getAnimeStatus(...);
	int rLine;
} animeEpisodeData;

typedef struct {
	int firstEpisode;
	int secondEpisode;

	// Questa variabile determina il tipo di operazione che devo svolgere durante il download
	// 0 -> ALL	|| 1 -> SINGLE || 2 -> RANGE || 3 -> CFU
	int option;

	char *nameEpisode;
	bool nameEpisodeChange;

	char *downloadDirectory;
} downloadOption;

typedef struct {
	// Percorso del file contenente l'elenco dei preferiti
	char *summaryPath;

	// Elenco dei dati del file localizzato in summaryPath
	// e relativa dimensione
	char **libData;
	int libLine;
} library;