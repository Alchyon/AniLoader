#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <unistd.h>
#include <time.h>

// Windows specific
// conio.h for Windows getch();
#include <windows.h>
#include <conio.h>

#define clearScreen "cls"
#define BPATH "C:/Users/"
#define URL "https://www.animeworld.so"
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