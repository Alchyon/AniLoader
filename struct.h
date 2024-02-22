typedef struct {
	char **findAnimeLink;
	char **correctAnimeName;
	int numberOfAnime;
} animeSearchData;

typedef struct {
	char **animeEpisodeExtension;
	int numberOfEpisode;
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