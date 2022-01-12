// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the DLL_IMP_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// DLL_IMP_API functions as being imported from a DLL, whereas this DLL sees symbols

#include <windows.h>
#include <tchar.h>
//#include "Header.h"


#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include <aclapi.h>
#include <strsafe.h>

//Definir uma constante para facilitar a leitura do protótipo da função
//Este .h deve ser incluído no projecto que o vai usar (modo implícito)
#define TAM 50
#define ERRO 6
#define DllExport extern "C"


//Esta macro é definida pelo sistema caso estejamos na DLL (DLL_IMP_EXPORTS definida)
//ou na app (DLL_IMP_EXPORTS não definida)
#ifdef DLL_IMP_EXPORTS
#define DLL_IMP_API DllExport __declspec(dllexport)
#else
#define DLL_IMP_API __declspec(dllimport)
#endif







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


User user;
JOGO jogo;
//Variável global da DLL
extern DLL_IMP_API BOOL crasha;

//Funções a serem exportadas/importadas
DLL_IMP_API int Autenticar(HANDLE hPipe, TCHAR *utilizador,TCHAR *pass);
DLL_IMP_API JOGO InicioJogo(HANDLE hPipe, TCHAR *utilizador);
DLL_IMP_API JOGO LerInformacao(HANDLE hPipe);
DLL_IMP_API int EnviarPalavraOuLetra(HANDLE hPipe, TCHAR *palavra, TCHAR letra, TCHAR *utilizador);
DLL_IMP_API int Desistir(HANDLE hPipe, TCHAR *utilizador);
DLL_IMP_API User* LerListaJogadores(HANDLE hPipe);
DLL_IMP_API int adduser(HANDLE hPipe, TCHAR *utilizador,TCHAR *pass);
DLL_IMP_API int logout(HANDLE hPipe, TCHAR *utilizador);
