#include "biblioteca.h"
BOOL ret;
BOOL crasha=FALSE;
BOOL WINAPI DllMain(HANDLE hInstance, DWORD razao, LPVOID dados){
    switch(razao){
        case DLL_PROCESS_ATTACH:
           break;
        case DLL_PROCESS_DETACH:
          break;
    }
    return 1;
}


int Autenticar(HANDLE hPipe, TCHAR *utilizador,TCHAR *pass){
	DATA d;
	d.id=1;
	DWORD n;
	int i;
	_stprintf(d.str1, utilizador);
	_stprintf(d.str2, pass);
	if (!WriteFile(hPipe, &d, sizeof(DATA), &n, NULL)) {
					
					//_tperror(TEXT("[ERRO] Escrever no pipe... (WriteFile)\n"));
					i=0;
					}
	ret=ReadFile(hPipe, &i, sizeof(int), &n, NULL);
	if (!ret || !n){
		crasha=TRUE;
		return 0;
	}
	return i;
}

JOGO InicioJogo(HANDLE hPipe, TCHAR *utilizador){
	DATA d;
	JOGO j;
	d.id=2;
	DWORD n;
	int i;
	_stprintf(d.str1, utilizador);
	
	if (!WriteFile(hPipe, &d, sizeof(DATA), &n, NULL)) {
		//_tperror(TEXT("[ERRO] Escrever no pipe... (WriteFile)\n"));			
	}
	ret=ReadFile(hPipe, &j, sizeof(JOGO), &n, NULL);
	if (!ret || !n){
		crasha=TRUE;
	}
	return j;
}

JOGO LerInformacao(HANDLE hPipe){
	DATA d;
	JOGO j;
	d.id=3;
	DWORD n;
	if (!WriteFile(hPipe, &d, sizeof(DATA), &n, NULL)) {
		//_tperror(TEXT("[ERRO] Escrever no pipe... (WriteFile)\n"));
	}
	ret=ReadFile(hPipe, &j, sizeof(JOGO), &n, NULL);
	if (!ret || !n){
		crasha=TRUE;
	}
	return j;
}

int EnviarPalavraOuLetra(HANDLE hPipe, TCHAR *palavra, TCHAR letra, TCHAR *utilizador){
	DATA d;
	int i;
	d.id=4;
	DWORD n;
	_stprintf(d.str1, palavra);
	_stprintf(d.str2, utilizador);
	d.car=letra;
	if (!WriteFile(hPipe, &d, sizeof(DATA), &n, NULL)) {
		//_tperror(TEXT("[ERRO] Escrever no pipe... (WriteFile)\n"));
	}
	ret=ReadFile(hPipe, &i, sizeof(int), &n, NULL);
	if (!ret || !n){
		crasha=TRUE;
		return 0;
	}
	return i;
}

int Desistir(HANDLE hPipe, TCHAR *utilizador){
	DATA d;
	int i;
	d.id=6;
	DWORD n;
	_stprintf(d.str1, utilizador);
	
	
	if (!WriteFile(hPipe, &d, sizeof(DATA), &n, NULL)) {
					
					//_tperror(TEXT("[ERRO] Escrever no pipe... (WriteFile)\n"));
					
					}
	ret=ReadFile(hPipe, &i, sizeof(int), &n, NULL);
	if (!ret || !n){
		crasha=TRUE;
		return 0;
	}
	return i;
}


User* LerListaJogadores(HANDLE hPipe){

	DATA d;
	int i;
	d.id=5;
	DWORD n;
	User jogadores[TAM];
	
	if (!WriteFile(hPipe, &d, sizeof(DATA), &n, NULL)) {
					
					//_tperror(TEXT("[ERRO] Escrever no pipe... (WriteFile)\n"));
					
					}
	ret=ReadFile(hPipe, &i,sizeof(int), &n, NULL);
	if (!ret || !n){
		crasha=TRUE;
		return 0;
	}
	ret=ReadFile(hPipe, &jogadores, TAM*sizeof(User), &n, NULL);
	if (!ret || !n){
		crasha=TRUE;
		return 0;
	}
	
	return jogadores;
}

int adduser(HANDLE hPipe, TCHAR *utilizador,TCHAR *pass){
	DATA d;
	int i;
	d.id=7;
	DWORD n;
	_stprintf(d.str1, utilizador);
	_stprintf(d.str2, pass);
	
	if (!WriteFile(hPipe, &d, sizeof(DATA), &n, NULL)) {
					
					//_tperror(TEXT("[ERRO] Escrever no pipe... (WriteFile)\n"));
					
					}
	ret=ReadFile(hPipe, &i, sizeof(int), &n, NULL);
	if (!ret || !n){
		crasha=TRUE;
		return 0;
	}
	
	return i;
}
 int logout(HANDLE hPipe, TCHAR *utilizador){
	DATA d;
	int i;
	d.id=8;
	DWORD n;
	_stprintf(d.str1, utilizador);

	if (!WriteFile(hPipe, &d, sizeof(DATA), &n, NULL)) {
					
					//_tperror(TEXT("[ERRO] Escrever no pipe... (WriteFile)\n"));
					
					}
	ret=ReadFile(hPipe, &i, sizeof(int), &n, NULL);
	if (!ret || !n){
		crasha=TRUE;
		return 0;
	}
	return i;
}