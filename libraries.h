#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <unistd.h>

// Windows specific
// conio.h for Windows getch();
#include <conio.h>

// struct(s)
#include "struct.h"

#define clearScreen "cls"
#define BPATH "C:/Users/"
#define URL "https://www.animeworld.ac"
#define serverID "data-name=\"9\">AnimeWorld Server"
#define serverNumber "9"

// Windows 10 - versione generica
#define _WIN32_WINNT_WIN10 0x0A00

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

// COOKIE SECTION!
#define APPDATA getenv("APPDATA")
#define USERAGENT "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/42.0.2311.135 Safari/537.36 Edge/12.246"
#define COOKIEFILE "__cookies.cookie"
#define COOKIESCRIPT_FORMAT "-A \" USERAGENT \" --cookie-jar \"%s\\AniLoader\\" COOKIEFILE "\" --cookie \"%s\\AniLoader\\" COOKIEFILE "\""
#define INIT_COOKIESCRIPT(buffer, appdata) snprintf(buffer, 1024, COOKIESCRIPT_FORMAT, appdata, appdata)

// Variabile globale che contiene tutto lo script di COOKIESCRIPT, calcolato una volta!
extern char *COOKIECMD;
