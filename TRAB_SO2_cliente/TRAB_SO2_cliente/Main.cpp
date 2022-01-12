/* ===================================================== 
   Threads.c
   Cria dois threads (opção F1 do menu):
		Os dois threads executam em simultâneo dois
		ciclos de 0 a 99999 (um em ordem crescente e 
		outro em ordem decrescente).
	F2 - Para as threads
	F3 - Resume as threads paradas

	SEMAFORO: Objecto de sincronizacao que restringe acesso a recurso com unidades 
	limitadas que podem ser detidas por threads (extensao de mutex).

  NOTA: Enquanto os threads evoluem verificra que é possível
        abrir as outras opções do menu (DialogBox e Exit)

   ===================================================== */

#include <windows.h>
#include <tchar.h>
#include "resource.h"
#include "biblioteca.h"




#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include <aclapi.h>
#include <strsafe.h>



#define NUM 7
#define MAX 100000
#define PIPE_NAME TEXT("\\\\%s\\pipe\\teste")

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);
DWORD WINAPI ThreadDesistir();
DWORD WINAPI ThreadJogada(LPVOID jgd);
DWORD WINAPI ThreadAutenticar();
BOOL CALLBACK TentaPalavra(HWND hDlg, UINT messg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK TrataLogin(HWND hDlg, UINT messg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK fazok(HWND hDlg, UINT messg, WPARAM wParam, LPARAM lParam);
TCHAR CALLBACK coordenadaletra();
void CALLBACK verifica(HWND hWnd,int i);
void CALLBACK PintaLetras(HDC hdc,TCHAR *letrasusadas);
void CriaPen(HDC hdc);
BOOL CALLBACK configuraIp(HWND hDlg, UINT messg, WPARAM wParam, LPARAM lParam);
DWORD WINAPI lerinformacao();
BOOL CALLBACK adduser(HWND hDlg, UINT messg, WPARAM wParam, LPARAM lParam);
DWORD WINAPI Threadaddrem();
DWORD WINAPI ThreadList();
BOOL CALLBACK TrataLista(HWND hDlg, UINT messg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK TrataPI(HWND hDlg, UINT messg, WPARAM wParam, LPARAM lParam);

TCHAR szProgName[] = TEXT("Forca");

HWND hDlgGlobal;
BOOL ajogar;
BOOL clicaletra;
HINSTANCE hInstG;
User *list;

typedef struct{
		TCHAR palavra[TAM];
		TCHAR letra;
}jogada;



HANDLE threads[2],tj;
DWORD t1,t2;	
//static User user;
//static JOGO jogo;
//parametro param;	
static DATA data;
int xi, yi,nusers;
HINSTANCE hInstance, biblioteca;
HWND Button;
DWORD n;
HANDLE hPipe;
TCHAR ip[TAM];
HKEY chave;
jogada joga;
/* ----------------------------------------------------- */
/*  "WinMain" - Funcao utilizada no inicio da execucao.  */
/* ----------------------------------------------------- */
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst,  LPSTR lpCmdLine, int nCmdShow) {
	HWND hWnd;
	MSG lpMsg;
	WNDCLASSEX wcApp;
	HACCEL hAccel;
	

	/* Definicao da Window Class */
	wcApp.cbSize = sizeof(WNDCLASSEX);
	wcApp.hInstance = hInst;
	wcApp.lpszClassName = szProgName;
	wcApp.lpfnWndProc = WndProc;
	wcApp.style = CS_HREDRAW | CS_VREDRAW;
	wcApp.hIcon = LoadIcon(hInst, (LPCSTR)IDI_ICON1);
	wcApp.hIconSm = NULL;
	wcApp.hCursor = LoadCursor(hInst, (LPCSTR)IDC_CURSOR1);;
	wcApp.lpszMenuName = (LPCTSTR) IDR_MENU1;
	wcApp.cbClsExtra = 0;
	wcApp.cbWndExtra = 0;
	wcApp.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);

	/* Registo da Window Class */
	if (!RegisterClassEx(&wcApp))
		return(0);
	
	/* Criar janela */
	hWnd = CreateWindow(szProgName, TEXT("Jogo da Forca"), 
						WS_OVERLAPPEDWINDOW, 200, 
						50, 1024,768,
						(HWND) HWND_DESKTOP, (HMENU) NULL, 
						 hInst, (LPSTR) NULL);

	/* Guarda o valor da instancia actual */
	hInstance = hInst;

	/* Mostrar janela */
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	
	//declarar
	user.logado=FALSE;
	user.admin=FALSE;
	user.jogos=0;
	user.wins=0;
	user.nome;
	hDlgGlobal=NULL;
	ajogar=FALSE;
	clicaletra=FALSE;

	/* Ciclo de mensagens */
	while (GetMessage(&lpMsg,NULL,0,0)) {
			TranslateMessage(&lpMsg);
			DispatchMessage(&lpMsg);
	}

	/* Exit Status */
	return((int)lpMsg.wParam);
}



/* ----------------------------------------------------- */
/*  "WndProc" - Funcao chamada pelo NT quando precisa    */
/*              de passar mensagens (WinMain).           */
/* ----------------------------------------------------- */
LRESULT CALLBACK WndProc(HWND hWnd, UINT messg,  WPARAM wParam, LPARAM lParam) {
	int resposta;
	static HMENU menu;
	HDC hdc;
	PAINTSTRUCT PtStc;
	//RECT lpRect;
	TCHAR msgT[TAM]=TEXT("\0"),resp;
	HDC memdc, auxhdc,errodc;
	
	//HKEY chave;
	DWORD queAconteceu, tamanho,result;
	

	TCHAR str[256],strP[256];

	static HBITMAP figura_inicio, figura_jogo,figura_erro;

	/* Processa os diferentes tipos de mensagens */
	switch (messg) {
		case WM_CREATE:
			//Obter o handle do menu da janela principal
			menu=GetMenu(hWnd);
			
			

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case ID_JOGO_LOGIN:


					if(RegCreateKeyEx(HKEY_LOCAL_MACHINE,TEXT("Software\\Forca"),0, NULL, REG_OPTION_VOLATILE,
						KEY_ALL_ACCESS, NULL, &chave, &queAconteceu) != ERROR_SUCCESS)
						MessageBox(hWnd, TEXT("Erro ao criar/abrir chave"), TEXT("Registry"), MB_OK);
					else
						//Se a chave foi criada, inicializar os valores
						if(queAconteceu == REG_CREATED_NEW_KEY){
							//MessageBox(hWnd, TEXT("Chave: HKEY_LOCAL_MACHINE\Software\MinhaAplicação criada"), TEXT("Registry"), MB_OK);
							//Criar valor "Autor" = "Meu nome"

							MessageBox(hWnd, TEXT("Não tem IP guardado, por favor vá as opções e configure IP"), TEXT("Registry"), MB_OK);
							break;
						}else if (queAconteceu == REG_OPENED_EXISTING_KEY){
							//Se a chave foi aberta, ler os valores lá guardados
							tamanho = TAM;
							RegQueryValueEx(chave, TEXT("IP"), NULL, NULL, (LPBYTE)ip, &tamanho);
						}
						RegCloseKey(chave);

					_stprintf(strP,PIPE_NAME, ip);
					//_tprintf(TEXT("[CLIENTE] Esperar pelo pipe '%s'... (WaitNamedPipe)\n"), strP);
					if (!WaitNamedPipe(strP, 5000)) {
						MessageBox(hWnd,TEXT("[ERRO] Ligar ao pipe"), TEXT("Nao encontrou o pipe1"), MB_OK|MB_ICONQUESTION);
						//_tprintf(TEXT("[ERRO] Ligar ao pipe '%s'... (WaitNamedPipe)\n"), strP);
						break;
					}

					//_tprintf(TEXT("[CLIENTE] Ligação ao servidor... (CreateFile)\n"));
					hPipe = CreateFile(strP, GENERIC_WRITE|GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
					if (hPipe==NULL) {
						MessageBox(hWnd,TEXT("[ERRO] Ligar ao pipe"), TEXT("Nao encontrou o pipe"), MB_YESNO|MB_ICONQUESTION);
						//_tprintf(TEXT("[ERRO] Ligar ao pipe '%s'... (CreateFile)\n"), strP);
						exit(1);
					}

					DialogBox(hInstance, (LPCSTR) IDD_DIALOG1, hWnd, TrataLogin); //Moda
					
					if(user.logado){
						InvalidateRect(hWnd,NULL,1);
						//UpdateWindow(hWnd);
						EnableMenuItem(menu,ID_JOGO_LOGIN,MF_DISABLED);
						EnableMenuItem(menu,ID_JOGO_NOVOJOGO,MF_ENABLED);
						EnableMenuItem(menu,ID_JOGO_LIGA,MF_ENABLED);
						EnableMenuItem(menu,ID_JOGO_LOGOUT,MF_ENABLED);
						EnableMenuItem(menu,ID_JOGO_NOVOJOGO,MF_ENABLED);
						EnableMenuItem(menu,ID_VERSTATUS_MELHORES,MF_ENABLED);
						EnableMenuItem(menu,ID_VERSTATUS_PROPRIAS,MF_ENABLED);
						EnableMenuItem(menu,ID_JOGO_DESISTIR,MF_DISABLED);
						if(user.admin)
							EnableMenuItem(menu,ID_OPCOES_ADMINISTRADOR,MF_ENABLED);
						

					}
					
					
					break;
				case ID_JOGO_NOVOJOGO:
					
					jogo=InicioJogo(hPipe, user.nome);
					
					if(_tcsicmp(jogo.nomesJog1,TEXT(""))!=0&&!crasha){
							EnableMenuItem(menu,ID_JOGO_DESISTIR,MF_ENABLED);
							EnableMenuItem(menu,ID_JOGO_NOVOJOGO,MF_DISABLED);
							ajogar=TRUE;
							InvalidateRect(hWnd,NULL,0);
							
					}else{
						if(crasha)
							MessageBox(hWnd,TEXT("SERVER NAO ENCONTRADO!!"),TEXT("Nao é possivel comecar novo jogo!"),MB_OK|MB_ICONERROR);
						else
						MessageBox(hWnd,TEXT("Ja se encontra um jogo a decorrer!"),TEXT("Nao é possivel comecar novo jogo!"),MB_OK|MB_ICONERROR);
						//mensagem a dizer que ja existe jogo a decorrer
						
						break;
					}
					
					
					if(_tcsicmp(jogo.nomesJog2,user.nome)==0){
						UpdateWindow(hWnd);
						//lerinformacao();
						CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)lerinformacao,NULL,0, &t1);
						UpdateWindow(hWnd);
					}
					break;
				case ID_JOGO_LOGOUT:
					if(MessageBox(hWnd,TEXT("Tem a certeza que deseja fazer Logout?"), TEXT("LOGOUT"), MB_YESNO|MB_ICONQUESTION)==IDYES){
						EnableMenuItem(menu,ID_JOGO_LOGIN,MF_ENABLED);
						EnableMenuItem(menu,ID_JOGO_NOVOJOGO,MF_DISABLED);
						EnableMenuItem(menu,ID_JOGO_LIGA,MF_DISABLED);
						EnableMenuItem(menu,ID_JOGO_LOGOUT,MF_DISABLED);
						EnableMenuItem(menu,ID_VERSTATUS_MELHORES,MF_DISABLED);
						EnableMenuItem(menu,ID_VERSTATUS_PROPRIAS,MF_DISABLED);
						EnableMenuItem(menu,ID_OPCOES_ADMINISTRADOR,MF_DISABLED);
						EnableMenuItem(menu,ID_JOGO_DESISTIR,MF_DISABLED);
					}
					user.logado=FALSE;
					user.admin=FALSE;
					if(ajogar){
						CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) ThreadDesistir,NULL,0, &t2);
						
					}
					logout(hPipe,user.nome);
					ajogar=FALSE;
					InvalidateRect(hWnd,NULL,1);
					break;
				case ID_VERSTATUS_MELHORES:
					if(hDlgGlobal == NULL){
						hDlgGlobal = CreateDialog(hInstG, (LPCSTR) IDD_LIST, hWnd, TrataLista);//Nao modal
						ShowWindow(hDlgGlobal,SW_SHOW);
					}
					break;
				case ID_VERSTATUS_PROPRIAS:
					if(hDlgGlobal == NULL){
						hDlgGlobal = CreateDialog(hInstG, (LPCSTR) IDD_PINFO, hWnd, TrataPI);//Nao modal
						ShowWindow(hDlgGlobal,SW_SHOW);
					}
					break;
				case ID_JOGO_CONFIGURARSERVER:
					//Criar/abrir uma chave em HKEY_LOCAL_MACHINE\Software\MinhaAplicacao
					if(RegCreateKeyEx(HKEY_LOCAL_MACHINE,TEXT("Software\\Forca"),0, NULL, REG_OPTION_VOLATILE,
						KEY_ALL_ACCESS, NULL, &chave, &queAconteceu) != ERROR_SUCCESS)
						MessageBox(hWnd, TEXT("Erro ao criar/abrir chave"), TEXT("Registry"), MB_OK);
					else
						//Se a chave foi criada, inicializar os valores
						if(queAconteceu == REG_CREATED_NEW_KEY){
							//MessageBox(hWnd, TEXT("Chave: HKEY_LOCAL_MACHINE\Software\MinhaAplicação criada"), TEXT("Registry"), MB_OK);
							//Criar valor "Autor" = "Meu nome"


							DialogBox(hInstance, (LPCSTR) IDD_ConfigIP, hWnd, configuraIp); //Moda

							
						}
						//Se a chave foi aberta, ler os valores lá guardados
						else if (queAconteceu == REG_OPENED_EXISTING_KEY){
							//MessageBox(hWnd, TEXT("Chave: HKEY_LOCAL_MACHINE\Software\MinhaAplicacao aberta"), TEXT("Registry"), MB_OK);
							//tamanho = 20;

							tamanho = TAM;
							RegQueryValueEx(chave, TEXT("IP"), NULL, NULL, (LPBYTE)ip, &tamanho);
							//MessageBox(hWnd, ip, TEXT("IP guardado"), MB_OK );
							DialogBox(hInstance, (LPCSTR) IDD_ConfigIP, hWnd, configuraIp); //Moda

							/*RegSetValueEx(chave, TEXT("IP"), 0, REG_SZ, (LPBYTE) ip, _tcslen(ip) + 1);
								
							
							
							//RegQueryValueEx(chave, TEXT("Versao"), NULL, NULL, (LPBYTE)&versao, &tamanho);
							_stprintf(str, TEXT("IP alterado: %s"), ip);
							MessageBox(hWnd, str, TEXT("IP guardado"), MB_OK );*/


							
								//MessageBox(hWnd, TEXT("IP nao foi alterado"), TEXT("Registry"), MB_OK );
							
							
						}
						RegCloseKey(chave);
					break;

			

					break;
				case ID_OPCOES_ADMINISTRADOR:
					DialogBox(hInstance, (LPCSTR) IDD_ADDU, hWnd, adduser); //Moda
					break;
				case ID_HELP40010:
					if(hDlgGlobal == NULL){
							hDlgGlobal = CreateDialog(hInstance, (LPCSTR) IDD_DHELP, hWnd, fazok);//Nao modal
							ShowWindow(hDlgGlobal,SW_SHOW);
					}
					break;
				case ID_SOBRE:
					if(hDlgGlobal == NULL){
							hDlgGlobal = CreateDialog(hInstance, (LPCSTR) IDD_DSOBRE, hWnd, fazok);//Nao modal
							ShowWindow(hDlgGlobal,SW_SHOW);
					}
					break;
				case ID_JOGO_DESISTIR:
					CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) ThreadDesistir,NULL,0, &t2);
					InvalidateRect(data.hwnd,NULL,0);
					break;
				case ID_SAIR:
					resposta=MessageBox(hWnd,TEXT("Tem a certeza que deseja terminar o Programa?"), TEXT("Sair do jogo da forca?"), MB_YESNO|MB_ICONQUESTION);
					if (resposta==IDYES){
						PostQuitMessage(0);				// Se YES, terminar programa
						if(ajogar){
							CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) ThreadDesistir,NULL,0, &t2);
						
						}
						logout(hPipe,user.nome);
					}
						break;

			}
			break;
		case WM_KEYDOWN:
			if(ajogar&&(jogo.turno==1&&_tcsicmp(jogo.nomesJog1,user.nome)==0||jogo.turno==2&&_tcsicmp(jogo.nomesJog2,user.nome)==0)){
				if(wParam==VK_SPACE){
					DialogBox(hInstance, (LPCSTR) IDD_TENTAPAL, hWnd, TentaPalavra);
					InvalidateRect(hWnd,NULL,1);
				}
			}
			break;
		case WM_CHAR:
			if(wParam>=65 && wParam<=90){
				if(ajogar&&(jogo.turno==1&&_tcsicmp(jogo.nomesJog1,user.nome)==0||jogo.turno==2&&_tcsicmp(jogo.nomesJog2,user.nome)==0)){
					InvalidateRect(hWnd,NULL,0);
					joga.letra=wParam;
					joga.palavra[0]='0';
					//verfica(hWnd,EnviarPalavraOuLetra(NULL,wParam));
					result=WaitForSingleObject(tj,0);
					if(result==WAIT_OBJECT_0)
						tj=CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) ThreadJogada,&joga,0, &t2);
				}
			}else if(wParam>=97	&& wParam<=122){
				if(ajogar&&(jogo.turno==1&&_tcsicmp(jogo.nomesJog1,user.nome)==0||jogo.turno==2&&_tcsicmp(jogo.nomesJog2,user.nome)==0)){
					InvalidateRect(hWnd,NULL,0);
					//verfica(hWnd,EnviarPalavraOuLetra(NULL,wParam-32));
					joga.letra=wParam-32;
					joga.palavra[0]='0';
					result=WaitForSingleObject(tj,0);
					if(result==WAIT_OBJECT_0)
						tj=CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) ThreadJogada,&joga,0, &t2);
				}
			}
			InvalidateRect(hWnd,NULL,0);

			break;
		case WM_LBUTTONDOWN:
			xi=LOWORD(lParam);
			yi=HIWORD(lParam);
			//clicaletra=TRUE;
			if(ajogar&&(jogo.turno==1&&_tcsicmp(jogo.nomesJog1,user.nome)==0||jogo.turno==2&&_tcsicmp(jogo.nomesJog2,user.nome)==0)){
				data.car=coordenadaletra();
				if(data.car=='1'){
					DialogBox(hInstance, (LPCSTR) IDD_TENTAPAL, hWnd, TentaPalavra);
					
					InvalidateRect(hWnd,NULL,1);
				}
				else{
					if(data.car=='0'){
						InvalidateRect(hWnd,NULL,0);
						break;
					}
					data.hwnd=hWnd;
					//verfica(hWnd,EnviarPalavraOuLetra(NULL,resp));
					joga.letra=data.car;
					joga.palavra[0]='0';
					result=WaitForSingleObject(tj,0);
					if(result==WAIT_OBJECT_0)
						tj=CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) ThreadJogada,&joga,0, &t2);
				}
			}

			InvalidateRect(hWnd,NULL,0);
			break;

		case WM_PAINT:
			hdc=BeginPaint(hWnd, &PtStc);
			auxhdc=GetDC(hWnd);
			if(crasha && user.logado){
				user.logado=FALSE;
				crasha=FALSE;
				_stprintf(user.nome,TEXT(""));
				//server crashado;
				MessageBox(hWnd,TEXT("Servidor não encontrado!"), TEXT("INFO"),MB_ICONERROR);
				
			}
				
			
			switch (ajogar)
			{
				case 0:
					EnableMenuItem(menu,ID_JOGO_DESISTIR,MF_DISABLED);
					if(user.logado)
						EnableMenuItem(menu,ID_JOGO_NOVOJOGO,MF_ENABLED);
					figura_inicio=LoadBitmap(hInstance,(LPCTSTR) IDB_BITMAP1);
					memdc=CreateCompatibleDC(auxhdc);					
					SelectObject(memdc,figura_inicio);
					BitBlt(hdc,20,5,900,700,memdc,0,0,SRCCOPY);
					DeleteDC(memdc);
					DeleteObject(figura_inicio);

					if(!user.logado){
						TextOut(hdc, 650, 200, TEXT("Por favor faça login para começar a jogar!"), _tcslen(TEXT("Por favor faça login para começar a jogar!")));
					}else{
						_stprintf(msgT, TEXT("Bem vindo %s !"), user.nome);
						TextOut(hdc, 650, 200, msgT, _tcslen(msgT));
					}

					break;
				case 1:
					EnableMenuItem(menu,ID_JOGO_NOVOJOGO,MF_DISABLED);
					//lerinformacao();
					//lancar thread? e isto la dentro?
					figura_jogo=LoadBitmap(hInstance,(LPCTSTR) IDB_BITMAP2);
					memdc=CreateCompatibleDC(auxhdc);					
					SelectObject(memdc,figura_jogo);
					BitBlt(hdc,62,5,900,700,memdc,0,0,SRCCOPY);
					DeleteDC(memdc);
					DeleteObject(figura_inicio);
					//***********
					switch(jogo.jog1Erros){
							case 0:
								break;
							case 1:
								figura_erro=LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_BITMAP3));
								errodc=CreateCompatibleDC(auxhdc);	
								SelectObject(errodc,figura_erro);
								BitBlt(hdc,110,37,256,394,errodc,0,0,SRCCOPY);
								DeleteDC(errodc);
								DeleteObject(figura_erro);
								break;
							case 2:
								figura_erro=LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_BITMAP4));
								errodc=CreateCompatibleDC(auxhdc);	
								SelectObject(errodc,figura_erro);
								BitBlt(hdc,110,37,256,394,errodc,0,0,SRCCOPY);
								DeleteDC(errodc);
								DeleteObject(figura_erro);
								break;
							case 3:
								figura_erro=LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_BITMAP5));
								errodc=CreateCompatibleDC(auxhdc);	
								SelectObject(errodc,figura_erro);
								BitBlt(hdc,110,37,256,394,errodc,0,0,SRCCOPY);
								DeleteDC(errodc);
								DeleteObject(figura_erro);
								break;
							case 4:
								figura_erro=LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_BITMAP6));
								errodc=CreateCompatibleDC(auxhdc);	
								SelectObject(errodc,figura_erro);
								BitBlt(hdc,110,37,256,394,errodc,0,0,SRCCOPY);
								DeleteDC(errodc);
								DeleteObject(figura_erro);
								break;
							case 5:
								figura_erro=LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_BITMAP7));
								errodc=CreateCompatibleDC(auxhdc);	
								SelectObject(errodc,figura_erro);
								BitBlt(hdc,110,37,256,394,errodc,0,0,SRCCOPY);
								DeleteDC(errodc);
								DeleteObject(figura_erro);
								break;
							case 6:
								figura_erro=LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_BITMAP8));
								errodc=CreateCompatibleDC(auxhdc);	
								SelectObject(errodc,figura_erro);
								BitBlt(hdc,110,37,256,394,errodc,0,0,SRCCOPY);
								DeleteDC(errodc);
								DeleteObject(figura_erro);
								break;
					}

					switch(jogo.jog2Erros){
							case 0:
								break;
							case 1:
								figura_erro=LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_BITMAP14));
								errodc=CreateCompatibleDC(auxhdc);	
								SelectObject(errodc,figura_erro);
								BitBlt(hdc,657,37,256,394,errodc,0,0,SRCCOPY);
								DeleteDC(errodc);
								DeleteObject(figura_erro);
								break;
							case 2:
								figura_erro=LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_BITMAP13));
								errodc=CreateCompatibleDC(auxhdc);	
								SelectObject(errodc,figura_erro);
								BitBlt(hdc,657,37,256,394,errodc,0,0,SRCCOPY);
								DeleteDC(errodc);
								DeleteObject(figura_erro);
								break;
							case 3:
								figura_erro=LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_BITMAP12));
								errodc=CreateCompatibleDC(auxhdc);	
								SelectObject(errodc,figura_erro);
								BitBlt(hdc,657,37,256,394,errodc,0,0,SRCCOPY);
								DeleteDC(errodc);
								DeleteObject(figura_erro);
								break;
							case 4:
								figura_erro=LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_BITMAP11));
								errodc=CreateCompatibleDC(auxhdc);	
								SelectObject(errodc,figura_erro);
								BitBlt(hdc,657,37,256,394,errodc,0,0,SRCCOPY);
								DeleteDC(errodc);
								DeleteObject(figura_erro);
								break;
							case 5:
								figura_erro=LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_BITMAP10));
								errodc=CreateCompatibleDC(auxhdc);	
								SelectObject(errodc,figura_erro);
								BitBlt(hdc,657,37,256,394,errodc,0,0,SRCCOPY);
								DeleteDC(errodc);
								DeleteObject(figura_erro);
								break;
							case 6:
								figura_erro=LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_BITMAP9));
								errodc=CreateCompatibleDC(auxhdc);	
								SelectObject(errodc,figura_erro);
								BitBlt(hdc,657,37,256,394,errodc,0,0,SRCCOPY);
								DeleteDC(errodc);
								DeleteObject(figura_erro);
								break;
					}

					//*********
					if(_tcsicmp(jogo.nomesJog1,user.nome)==0){
						_stprintf(msgT, TEXT("%s - %d / %d"), user.nome, jogo.jog1Erros,ERRO);
						TextOut(hdc, 205, 600, msgT, _tcslen(msgT));
						

					}else{
						_stprintf(msgT, TEXT("%s - %d / %d"), user.nome, jogo.jog2Erros,ERRO);
						TextOut(hdc, 205, 600, msgT, _tcslen(msgT));
						
					}
					
					if((_tcsicmp(jogo.nomesJog1,user.nome)==0 && jogo.turno==1) || (_tcsicmp(jogo.nomesJog2,user.nome)==0 && jogo.turno==2)){
						_stprintf(msgT, TEXT("Minha vez de jogar!"));
						TextOut(hdc, 500, 100, msgT, _tcslen(msgT));
					}else{
						_stprintf(msgT, TEXT("Vez do adversario de jogar!"));
						TextOut(hdc, 480, 100, msgT, _tcslen(msgT));
					}

					
					_stprintf(msgT, TEXT("%s"), jogo.palavraoculta);
					TextOut(hdc, 490, 300, msgT, _tcslen(msgT));
					PintaLetras(hdc,jogo.letras);
					//clicaletra=FALSE;
					break;
			}
			ReleaseDC(hWnd, auxhdc);
			EndPaint(hWnd, &PtStc);
			break;

		case WM_CLOSE:
			resposta=MessageBox(hWnd,TEXT("Tem a certeza que deseja terminar o Programa?"), TEXT("Sair do jogo da forca?"), MB_YESNO|MB_ICONQUESTION);
			if (resposta==IDYES){
					PostQuitMessage(0);				// Se YES, terminar programa
					if(ajogar){
						CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) ThreadDesistir,NULL,0, &t2);
						
					}
					logout(hPipe,user.nome);
			}
			break;
		default:

			/* Executa o procedimento por defeito */
			return(DefWindowProc(hWnd,messg,wParam,lParam));
			break;
	} 
}


/* ----------------------------------------------------- */
/*  "DlgProc" - Funcao chamada pelo NT quando precisa    */
/*              de passar mensagens (Dialog).            */
/* ----------------------------------------------------- */

BOOL CALLBACK DlgProc(HWND hWnd, UINT messg,WPARAM wParam, LPARAM lParam) 
{

	switch(messg) {
		case WM_COMMAND:
		switch(LOWORD(wParam)) 
		{
			
		}
	}
	return 0;
}
//************************TENTA PALAVRA**************************
BOOL CALLBACK TentaPalavra(HWND hDlg, UINT messg, WPARAM wParam, LPARAM lParam){
	TCHAR pal[TAM];
	int i, tamanho;
	switch (messg) 
	{
		case WM_CLOSE:
			EndDialog(hDlg,0);
			hDlgGlobal = NULL;
			return 1;

		case WM_COMMAND:
			if(LOWORD(wParam)==IDCANCEL){
				EndDialog(hDlg,0);
				hDlgGlobal = NULL;
			}
			if(LOWORD(wParam)==IDOK){
				GetDlgItemText(hDlg,IDC_PAL, pal,TAM);
				tamanho=_tcslen(pal);
				for(i=0;i<tamanho;i++){
						pal[i]=_totupper(pal[i]);
				}
				
				joga.letra='0';
				_stprintf(joga.palavra,pal);
				CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) ThreadJogada,&joga,0, &t2);
				
				EndDialog(hDlg,0);
				hDlgGlobal = NULL;
			}
			
		return 1;

	}
		return 0;
}

//*************************trata login***************************
BOOL CALLBACK TrataLogin(HWND hDlg, UINT messg, WPARAM wParam, LPARAM lParam)
{
	TCHAR login[TAM], pass[TAM], msg[70];
	int i;
	
	data.hwnd=hDlg;
	switch (messg) 
	{
		case WM_CLOSE:
			EndDialog(hDlg,0);
			hDlgGlobal = NULL;
			return 1;

		case WM_COMMAND:
			if(LOWORD(wParam)==IDCANCEL){
				EndDialog(hDlg,0);
				hDlgGlobal = NULL;
			}
			if(LOWORD(wParam)==IDOK){
				GetDlgItemText(hDlg,IDC_LOGIN, login,TAM);
				GetDlgItemText(hDlg,IDC_PASS, pass,TAM);
				_stprintf(data.str1,login);
				_stprintf(data.str2,pass);
				
				tj=CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) ThreadAutenticar, NULL,0, &t1);
				
			}
			
		return 1;

	}
		return 0;
}

BOOL CALLBACK configuraIp(HWND hDlg, UINT messg, WPARAM wParam, LPARAM lParam)
{
	
	int i, tamanho;
	switch (messg) 
	{
		case WM_INITDIALOG:
			SetDlgItemText(hDlg,IDC_IPSAVE,ip);
			break;
			//SetDlgItemText(data.hwnd,IDC_LOGIN,TEXT(""));
		case WM_CLOSE:
			EndDialog(hDlg,0);
			hDlgGlobal = NULL;
			return 1;

		case WM_COMMAND:
			if(LOWORD(wParam)==IDCANCEL){
				MessageBox(hDlg, TEXT("Não foi guardado nenhum IP"), TEXT("Registry"), MB_OK );
				EndDialog(hDlg,0);
				hDlgGlobal = NULL;
			}
			if(LOWORD(wParam)==IDOK){
				GetDlgItemText(hDlg,IDC_InserirIP, ip,TAM);
				
				RegSetValueEx(chave, TEXT("IP"), 0, REG_SZ, (LPBYTE) ip, _tcslen(ip) + 1);
				MessageBox(hDlg, TEXT("IP guardado"), TEXT("Registry"), MB_OK );
							
				
				
				
				EndDialog(hDlg,0);
				hDlgGlobal = NULL;
				
			}
			
		return 1;

	}
		return 0;
}

//***************************faz ok******************************
BOOL CALLBACK fazok(HWND hDlg, UINT messg, WPARAM wParam, LPARAM lParam){
	switch (messg) 
	{
		case WM_CLOSE:
			EndDialog(hDlg,0);
			hDlgGlobal = NULL;
			return 1;
			break;
		case WM_COMMAND:
			if(LOWORD(wParam)==IDOK){
				EndDialog(hDlg,0);
				hDlgGlobal = NULL;
				return 1;
			}
			break;
	}
	return 0;
}
void CriaPen(HDC hdc){
	HPEN pen=CreatePen(PS_SOLID,5,RGB(255,0,0));
	SelectObject(hdc,pen);
}
void CALLBACK PintaLetras(HDC hdc,TCHAR *letrasusadas){
	int i=0,tam;
	tam=_tcslen(letrasusadas);
	CriaPen(hdc);
	for(i=0;i<tam;i++){
		switch(letrasusadas[i]){
			case 'A':	MoveToEx(hdc,176,464,NULL);
						LineTo(hdc,225,513);
						break;
			case 'B':	MoveToEx(hdc,227,464,NULL);
						LineTo(hdc,276,513);
						break;
			case 'C':	MoveToEx(hdc,278,464,NULL);
						LineTo(hdc,327,513);
						break;
			case 'D':	MoveToEx(hdc,329,464,NULL);
						LineTo(hdc,378,513);
						break;
			case 'E':	MoveToEx(hdc,380,464,NULL);
						LineTo(hdc,429,513);
						break;
			case 'F':	MoveToEx(hdc,431,464,NULL);
						LineTo(hdc,480,513);
						break;
			case 'G':	MoveToEx(hdc,482,464,NULL);
						LineTo(hdc,531,513);
						break;
			case 'H':	MoveToEx(hdc,533,464,NULL);
						LineTo(hdc,582,513);
						break;
			case 'I':	MoveToEx(hdc,584,464,NULL);
						LineTo(hdc,633,513);
						break;
			case 'J':	MoveToEx(hdc,635,464,NULL);
						LineTo(hdc,684,513);
						break;
			case 'K':	MoveToEx(hdc,686,464,NULL);
						LineTo(hdc,735,513);
						break;
			case 'L':	MoveToEx(hdc,737,464,NULL);
						LineTo(hdc,786,513);
						break;
			case 'M':	MoveToEx(hdc,788,464,NULL);
						LineTo(hdc,837,513);
						break;
			case 'N':	MoveToEx(hdc,176,515,NULL);
						LineTo(hdc,225,564);
						break;
			case 'O':	MoveToEx(hdc,227,515,NULL);
						LineTo(hdc,276,564);
						break;
			case 'P':	MoveToEx(hdc,278,515,NULL);
						LineTo(hdc,327,564);
						break;
			case 'Q':	MoveToEx(hdc,329,515,NULL);
						LineTo(hdc,378,564);
						break;
			case 'R':	MoveToEx(hdc,380,515,NULL);
						LineTo(hdc,429,564);
						break;
			case 'S':	MoveToEx(hdc,431,515,NULL);
						LineTo(hdc,480,564);
						break;
			case 'T':	MoveToEx(hdc,482,515,NULL);
						LineTo(hdc,531,564);
						break;
			case 'U':	MoveToEx(hdc,533,515,NULL);
						LineTo(hdc,582,564);
						break;
			case 'V':	MoveToEx(hdc,584,515,NULL);
						LineTo(hdc,633,564);
						break;
			case 'W':	MoveToEx(hdc,635,515,NULL);
						LineTo(hdc,684,564);
						break;
			case 'X':	MoveToEx(hdc,686,515,NULL);
						LineTo(hdc,735,564);
						break;
			case 'Y':	MoveToEx(hdc,737,515,NULL);
						LineTo(hdc,786,564);
						break;
			case 'Z':	MoveToEx(hdc,788,515,NULL);
						LineTo(hdc,837,564);
						break;
		}
	}
	
}

TCHAR CALLBACK coordenadaletra(){

	if((xi>=176 && xi<=225) && (yi>=464 && yi<=513 ))
		return 'A';
	if((xi>=227 && xi<=276) && (yi>=464 && yi<=513 ))
		return 'B';
	if((xi>=278 && xi<=327) && (yi>=464 && yi<=513 ))
		return 'C';
	if((xi>=329 && xi<=378) && (yi>=464 && yi<=513 ))
		return 'D';
	if((xi>=380 && xi<=429) && (yi>=464 && yi<=513 ))
		return 'E';
	if((xi>=431 && xi<=480) && (yi>=464 && yi<=513 ))
		return 'F';
	if((xi>=482 && xi<=531) && (yi>=464 && yi<=513 ))
		return 'G';
	if((xi>=533 && xi<=582) && (yi>=464 && yi<=513 ))
		return 'H';
	if((xi>=584 && xi<=633) && (yi>=464 && yi<=513 ))
		return 'I';
	if((xi>=635 && xi<=684) && (yi>=464 && yi<=513 ))
		return 'J';
	if((xi>=686 && xi<=735) && (yi>=464 && yi<=513 ))
		return 'K';
	if((xi>=737 && xi<=786) && (yi>=464 && yi<=513 ))
		return 'L';
	if((xi>=788 && xi<=837) && (yi>=464 && yi<=513 ))
		return 'M';

	if((xi>=176 && xi<=225) && (yi>=515 && yi<=564 ))
		return 'N';
	if((xi>=227 && xi<=276) && (yi>=515 && yi<=564 ))
		return 'O';
	if((xi>=278 && xi<=327) && (yi>=515 && yi<=564 ))
		return 'P';
	if((xi>=329 && xi<=378) && (yi>=515 && yi<=564 ))
		return 'Q';
	if((xi>=380 && xi<=429) && (yi>=515 && yi<=564 ))
		return 'R';
	if((xi>=431 && xi<=480) && (yi>=515 && yi<=564 ))
		return 'S';
	if((xi>=482 && xi<=531) && (yi>=515 && yi<=564 ))
		return 'T';
	if((xi>=533 && xi<=582) && (yi>=515 && yi<=564 ))
		return 'U';
	if((xi>=584 && xi<=633) && (yi>=515 && yi<=564 ))
		return 'V';
	if((xi>=635 && xi<=684) && (yi>=515 && yi<=564 ))
		return 'W';
	if((xi>=686 && xi<=735) && (yi>=515 && yi<=564 ))
		return 'X';
	if((xi>=737 && xi<=786) && (yi>=515 && yi<=564 ))
		return 'Y';
	if((xi>=788 && xi<=837) && (yi>=515 && yi<=564 ))
		return 'Z';
	if((xi>=380 && xi<=633) && (yi>=580 && yi<=632 ))
		return '1';
	
	return '0';
}
DWORD WINAPI lerinformacao(){
	jogo=LerInformacao(hPipe);
	InvalidateRect(data.hwnd,NULL,0);
	return 0;
}

DWORD WINAPI ThreadJogada(LPVOID jgd){
	jogada temp = *((jogada *)jgd);
	int i;
	i=EnviarPalavraOuLetra(hPipe, temp.palavra, temp.letra, user.nome);
	if(i!=2){

		verifica(data.hwnd,i);
		if(ajogar==FALSE){
			//desligar do servidor etc
			return 0;
		}else{
	//ReadFile(hPipe, &jogo, sizeof(JOGO), &n, NULL);
	//ReadFile(hPipe, &jogo, sizeof(JOGO), &n, NULL);
			lerinformacao();
			if(jogo.vencedor==1 && _tcsicmp(jogo.nomesJog1,user.nome)==0 ){
				verifica(data.hwnd,1);
				return 0;
			}else if(jogo.vencedor!=0){
					verifica(data.hwnd,-1);
					return 0;
			}
			if(jogo.vencedor==2 && _tcsicmp(jogo.nomesJog2,user.nome)==0){
				verifica(data.hwnd,1);
				return 0;
			}else if(jogo.vencedor!=0){
				verifica(data.hwnd,-1);
				return 0;
			}



			
			lerinformacao();
			if(jogo.vencedor==1 && _tcsicmp(jogo.nomesJog1,user.nome)==0 ){
				verifica(data.hwnd,1);
				
			}else if(jogo.vencedor!=0){
					verifica(data.hwnd,-1);
					
			}
			if(jogo.vencedor==2 && _tcsicmp(jogo.nomesJog2,user.nome)==0){
				verifica(data.hwnd,1);
				
			}else if(jogo.vencedor!=0){
				verifica(data.hwnd,-1);
			}
		}
	}
	//InvalidateRect(data.hwnd,NULL,0);
	//Sleep(5000);
	return 0;
}
void CALLBACK verifica(HWND hWnd,int i){
	if(i==1){
		InvalidateRect(hWnd,NULL,0);
		MessageBox(hWnd,TEXT("Parabéns Ganhou!!!! :D"),TEXT("Fim do jogo"),MB_OK);

		InvalidateRect(hWnd,NULL,0);
		UpdateWindow(hWnd);
		crasha=FALSE;
		ajogar=FALSE;
	}else if (i==-1){
		InvalidateRect(hWnd,NULL,0);
		MessageBox(hWnd,TEXT("Foi enforcado!!!! =X"),TEXT("Fim do jogo"),MB_OK|MB_ICONERROR);
		InvalidateRect(hWnd,NULL,0);
		UpdateWindow(hWnd);
		crasha=FALSE;
		ajogar=FALSE;
	}else return;
}


DWORD WINAPI ThreadAutenticar(){
	int i;
	TCHAR msg[TAM];

	i=Autenticar(hPipe,data.str1,data.str2);
				if(i==1){
					_stprintf(user.nome,data.str1,NULL,NULL);
					user.logado=TRUE;
					_stprintf(msg,TEXT("Bem vindo %s"),data.str1);
					MessageBox(data.hwnd,msg,TEXT("Login feito com sucesso"),MB_OK|MB_ICONASTERISK);
					EndDialog(data.hwnd,0);
					hDlgGlobal = NULL;
				}else{
					if(i==2){
					_stprintf(user.nome,data.str1,NULL,NULL);
					user.logado=TRUE;
					user.admin=TRUE;
					_stprintf(msg,TEXT("Administrador logado"));
					MessageBox(data.hwnd,msg,TEXT("Login feito com sucesso"),MB_OK|MB_ICONEXCLAMATION);
					EndDialog(data.hwnd,0);
					hDlgGlobal = NULL;
				}else{
					MessageBox(data.hwnd,TEXT("Não foi possivel efectuar o login\nLogin ou Password errados"),TEXT("Dados incorrectos!"),MB_OK|MB_ICONERROR);
					SetDlgItemText(data.hwnd,IDC_LOGIN,TEXT(""));
					SetDlgItemText(data.hwnd,IDC_PASS,TEXT(""));
					}
				}
				
				return 0;

}

DWORD WINAPI ThreadDesistir(){

	Desistir(hPipe, user.nome);
	ajogar=FALSE;
	MessageBox(hDlgGlobal,TEXT("Desistiu =X"),TEXT("Fim do jogo"),MB_OK|MB_ICONERROR);

	InvalidateRect(data.hwnd,NULL,0);
	return 0;
}

BOOL CALLBACK adduser(HWND hDlg, UINT messg, WPARAM wParam, LPARAM lParam)
{
	TCHAR login[TAM], pass[TAM];
	int i;

	data.hwnd=hDlg;
	switch (messg) 
	{
		case WM_CLOSE:
		EndDialog(hDlg,0);
		hDlgGlobal = NULL;
		return 1;
		case WM_COMMAND:
		if(LOWORD(wParam)==IDCANCEL){
			EndDialog(hDlg,0);
			hDlgGlobal = NULL;
		}
		if(LOWORD(wParam)==IDC_ADD){
			GetDlgItemText(hDlg,IDC_Nome, login,TAM);
			GetDlgItemText(hDlg,IDC_PASS, pass,TAM);
			_stprintf(data.str1,login);
			_stprintf(data.str2,pass);
			data.id=7;
			threads[0]=CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) Threadaddrem, NULL,0, &t1);
			
		}
		if(LOWORD(wParam)==IDC_REM){
			GetDlgItemText(hDlg,IDC_Nome, login,TAM);
			_stprintf(data.str1,"REMOVE");
			_stprintf(data.str2,login);
			data.id=7;
		    threads[0]=CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) Threadaddrem, NULL,0, &t1);
			
		}

		return 1;
	}
	return 0;
}

DWORD WINAPI Threadaddrem(){
	int i;
	TCHAR msg[TAM];

	

	i=adduser(hPipe, data.str1,data.str2);
				if(i==0){
					
					
					MessageBox(data.hwnd,TEXT("Removido com sucesso"),TEXT("Sucesso"),MB_OK|MB_ICONASTERISK);
					EndDialog(data.hwnd,0);
					hDlgGlobal = NULL;
				}else if(i==-1){
					
					MessageBox(data.hwnd,TEXT("Nao foi possivel remover utilizador"),TEXT("ERRO"),MB_OK|MB_ICONEXCLAMATION);
					EndDialog(data.hwnd,0);
					hDlgGlobal = NULL;
				}else if(i==1){
					MessageBox(data.hwnd,TEXT("Adicionado com sucesso"),TEXT("Sucesso"),MB_OK|MB_ICONERROR);
					EndDialog(data.hwnd,0);
					hDlgGlobal = NULL;
				}else {
					MessageBox(data.hwnd,TEXT("Nao adicionado"),TEXT("Remover"),MB_OK|MB_ICONERROR);
					EndDialog(data.hwnd,0);
					hDlgGlobal = NULL;
				}
				
				return 0;

}

BOOL CALLBACK TrataLista(HWND hDlg, UINT messg, WPARAM wParam, LPARAM lParam){
	DATA d;
	int i;
	DWORD n;
	switch (messg) 
	{
		case WM_CLOSE:
			EndDialog(hDlg,0);
			hDlgGlobal = NULL;
			return 1;

		case WM_COMMAND:
			if(LOWORD(wParam)==IDCANCEL){
				EndDialog(hDlg,0);
				hDlgGlobal = NULL;
			}
			
			
			break;

		case WM_INITDIALOG:
			//threads[0]=CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) ThreadList, NULL,0, &t1);
			//WaitForSingleObject(threads[0],4000);
			list=LerListaJogadores(hPipe);
			TCHAR li[TAM];
			i=0;
			while(_tcsicmp(list[i].nome,TEXT(""))!=0){
				SendDlgItemMessage(hDlg, IDC_LISTN,LB_ADDSTRING,0, (LPARAM)list[i].nome);
				_stprintf(li,"%d",list[i].jogos);
				SendDlgItemMessage(hDlg, IDC_LISTJ,LB_ADDSTRING,0, (LPARAM)li);
				_stprintf(li,"%d",list[i].wins);
				SendDlgItemMessage(hDlg, IDC_LISTV,LB_ADDSTRING,0, (LPARAM)li);
				i++;
			}
		break;
			
		return 1;

	}
		return 0;
}

BOOL CALLBACK TrataPI(HWND hDlg, UINT messg, WPARAM wParam, LPARAM lParam){
	DATA d;
	int i;
	DWORD n;
	switch (messg) 
	{
		case WM_CLOSE:
			EndDialog(hDlg,0);
			hDlgGlobal = NULL;
			return 1;

		case WM_COMMAND:
			if(LOWORD(wParam)==IDCANCEL){
				EndDialog(hDlg,0);
				hDlgGlobal = NULL;
			}
			
			
			break;

		case WM_INITDIALOG:
			
			list=LerListaJogadores(hPipe);
			TCHAR li[TAM];
			i=0;
			while(_tcsicmp(list[i].nome,user.nome)!=0){
				i++;
			}


			SetDlgItemText(hDlg,IDC_EDITN,list[i].nome);
			_stprintf(li,"%d",list[i].wins);
			SetDlgItemText(hDlg,IDC_EDITV,li);
			_stprintf(li,"%d",list[i].jogos);
			SetDlgItemText(hDlg,IDC_EDITJ,li);
			_stprintf(li,"%d",list[i].jogos-list[i].wins);
			SetDlgItemText(hDlg,IDC_EDITD,li);

		break;
			
		return 1;

	}
		return 0;
}

DWORD WINAPI ThreadList(){
	int i;
	list=LerListaJogadores(hPipe);

	return 0;
}