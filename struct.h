typedef struct {
	int enable;
	char code[33];
} cookie;

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
	int singleEpisode;

	int firstEpisode;
	int secondEpisode;

	// 0 -> ALL	|| 1 -> SINGLE || 2 -> RANGE
	int option;

	char *nameEpisode;
	bool nameEpisodeChange;

	char *downloadDirectory;
} downloadOption;