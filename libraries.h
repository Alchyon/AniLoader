#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <unistd.h>
#include <time.h>

#ifdef __unix__
	// Libreria necessaria per creare un getch() inesistente su Linux
	#include <termios.h>

	// Variabili d'ambiente in base all'OS
	#define clearScreen "clear"
	#define BPATH "/home/"
#endif

#ifdef _WIN32
	// Definisco windows.h e Lmcons.h per trasformare getlogin() in una Windows API
	#include <windows.h>
	#include <Lmcons.h>
	// Define conio.h for Windows getch();
	#include <conio.h>

	// Variabili d'ambiente in base all'OS
	#define clearScreen "cls"
	#define BPATH "C:/Users/"

	// Windows 10 - versione generica
	#define _WIN32_WINNT_WIN10 0x0A00
#endif

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"