#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include <aclapi.h>
#include <tchar.h>
#include <strsafe.h>
#include <ctime>

#define TAM 50
#define ERRO 6

typedef struct {
	TCHAR nome[TAM];
	TCHAR pass[TAM];
	int jogos,wins;
	BOOL logado,admin;
}User;

typedef struct{
		TCHAR letras[27];
		TCHAR palavra[TAM];
		TCHAR nomesJog1[TAM];
		TCHAR nomesJog2[TAM];
		TCHAR palavraoculta[TAM];
		int jog1Erros;
		int jog2Erros;
		int turno;
		int vencedor;	
		HANDLE p1pipe,p2pipe;
}JOGO;

typedef struct {
	int id;
	TCHAR str1[TAM];
	TCHAR str2[TAM];
	CHAR car;
	HWND hwnd;
}DATA;

void Autentica(HANDLE hPipe,DATA data,User *users,int nusers);

void EnviarPalavraOuLetra(HANDLE hPipe,DATA data,JOGO &jogo,User *users,int nusers);
void Desistir(HANDLE hPipe,DATA data,JOGO &jogo,User *users,int nusers);
int adduser(HANDLE hPipe,DATA data,User *users,int nusers);
void logoff(HANDLE hPipe,DATA data,User *users,int nusers);