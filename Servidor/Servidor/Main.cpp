
#include "Funcoes.h"
#define TAM 50

#define PIPE_NAME TEXT("\\\\.\\pipe\\teste")
DWORD WINAPI Atende(LPVOID param);
JOGO InicioJogo(DATA data);
DWORD WINAPI servidorjogo();
DWORD WINAPI sair();
void gestao();
static User users[TAM];
static int nusers;
static JOGO jogo;
static HANDLE evento,threadjogo;
BOOL running=FALSE;
BOOL jogodecorrer=FALSE;
BOOL ret = FALSE;

	DWORD tamanho;
	HKEY chave;
	DWORD queAconteceu;
void ErrorExit(LPTSTR lpszFunction) 
{ 
    // Retrieve the system error message for the last-error code
    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError(); 


    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

    // Display the error message and exit the process

    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, 
        (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR)); 
    StringCchPrintf((LPTSTR)lpDisplayBuf, 
        LocalSize(lpDisplayBuf) / sizeof(TCHAR),
        TEXT("%s failed with error %d: %s"), 
        lpszFunction, dw, lpMsgBuf); 
    MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK); 

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
    ExitProcess(dw); 
}

// Buffer clean up routine
void Cleanup(PSID pEveryoneSID, PSID pAdminSID, PACL pACL, PSECURITY_DESCRIPTOR pSD)
{
	if(pEveryoneSID)
		FreeSid(pEveryoneSID);
	if(pAdminSID)
		FreeSid(pAdminSID);
	if(pACL)
		LocalFree(pACL);
	if(pSD)
		LocalFree(pSD);
}
 

void MB(TCHAR *str)
{
	MessageBox(NULL,str,TEXT("Msg"),MB_OK);
}

void Seguranca(SECURITY_ATTRIBUTES *sa){
	PSECURITY_DESCRIPTOR pSD;
	PACL pAcl;
	EXPLICIT_ACCESS ea;
	PSID pEveryoneSID = NULL, pAdminSID = NULL;
	SID_IDENTIFIER_AUTHORITY SIDAuthWorld = SECURITY_WORLD_SID_AUTHORITY;
	TCHAR str[256];


	pSD = (PSECURITY_DESCRIPTOR) LocalAlloc(LPTR,
		SECURITY_DESCRIPTOR_MIN_LENGTH);
	if (pSD == NULL) {
		ErrorExit(TEXT("Erro LocalAlloc!!!"));
		return;
	}
	if (!InitializeSecurityDescriptor(pSD,SECURITY_DESCRIPTOR_REVISION)) {
		ErrorExit(TEXT("Erro IniSec!!!"));
		return;
	}

	// Create a well-known SID for the Everyone group.
	if(!AllocateAndInitializeSid(&SIDAuthWorld, 1, SECURITY_WORLD_RID,
	0, 0, 0, 0, 0, 0, 0, &pEveryoneSID))
	{
		_stprintf(str,TEXT("AllocateAndInitializeSid() error %u"),GetLastError());
		ErrorExit(str);
		Cleanup(pEveryoneSID, pAdminSID, NULL, pSD);
	}
	ZeroMemory(&ea, sizeof(EXPLICIT_ACCESS));

	ea.grfAccessPermissions = GENERIC_WRITE|GENERIC_READ;
	ea.grfAccessMode = SET_ACCESS;
	ea.grfInheritance= SUB_CONTAINERS_AND_OBJECTS_INHERIT;
	ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
	ea.Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
	ea.Trustee.ptstrName = (LPTSTR) pEveryoneSID;

	if (SetEntriesInAcl(1,&ea,NULL,&pAcl)!=ERROR_SUCCESS) {
		ErrorExit(TEXT("Erro SetAcl!!!"));
		return;
	}

	if (!SetSecurityDescriptorDacl(pSD,TRUE,pAcl,FALSE)) {
		ErrorExit(TEXT("Erro IniSec!!!"));
		return;
	}

	sa->nLength = sizeof(sa);
	sa->lpSecurityDescriptor = pSD;
	sa->bInheritHandle = TRUE;
}

void main(void) {
    TCHAR buf[256];
	int i=0;
    HANDLE hPipe,t;
	SECURITY_ATTRIBUTES sa;
	User tmp;
	JOGO jogo;
	Seguranca(&sa);
	
	
    
	//********************************temporario**************
	/*tmp.admin=TRUE;
	tmp.logado=0;
	_stprintf(tmp.nome,"admin");
	_stprintf(tmp.pass,"admin");
	tmp.wins=0;
	tmp.jogos=0;
	
	nusers=0;
	users[nusers]=tmp;
	nusers++;
	tmp.admin=FALSE;
	tmp.logado=0;
	_stprintf(tmp.nome,"user");
	_stprintf(tmp.pass,"user");
	tmp.wins=0;
	tmp.jogos=0;
	
	users[nusers]=tmp;
	nusers++;*/
	evento=CreateEvent(NULL,TRUE,FALSE,NULL);
	int x;
	BOOL result;
	//UNICODE: By default, windows console does not process wide characters. 
	//Probably the simplest way to enable that functionality is to call _setmode:
#ifdef UNICODE 
	_setmode(_fileno(stdin), _O_WTEXT); 
	_setmode(_fileno(stdout), _O_WTEXT); 
#endif

	//*************************
	
	//Criar/abrir uma chave em HKEY_LOCAL_MACHINE\Software\MinhaAplicacao
					if(RegCreateKeyEx(HKEY_LOCAL_MACHINE,TEXT("Software\\Forca"),0, NULL, REG_OPTION_VOLATILE,
						KEY_ALL_ACCESS, NULL, &chave, &queAconteceu) != ERROR_SUCCESS)
						//MessageBox(hWnd, TEXT("Erro ao criar/abrir chave"), TEXT("Registry"), MB_OK);
						_tprintf(TEXT("[SERVIDOR-] Erro ao criar/abrir chave\n"));
					else
						//Se a chave foi criada, inicializar os valores
						if(queAconteceu == REG_CREATED_NEW_KEY){
							//MessageBox(hWnd, TEXT("Chave: HKEY_LOCAL_MACHINE\Software\MinhaAplicação criada"), TEXT("Registry"), MB_OK);
							//Criar valor "Autor" = "Meu nome"
							
							RegSetValueEx(chave, TEXT("Nusers"), 0, REG_DWORD, (LPBYTE) &nusers, sizeof(DWORD));
							RegSetValueEx(chave, TEXT("Lista"), 0, REG_BINARY, (LPBYTE) users, sizeof(users));
							_tprintf(TEXT("[SERVIDOR-]Registou valores\n"));

							//RegSetValueEx(chave, TEXT("Versao"), 0, REG_DWORD, (LPBYTE)&versao, sizeof(DWORD));
							//RegSetValueEx(chave, TEXT("ListaInteiros"), 0, REG_BINARY, (LPBYTE) valores, sizeof(valores));
						}
						//Se a chave foi aberta, ler os valores lá guardados
						else if (queAconteceu == REG_OPENED_EXISTING_KEY){
							

							_tprintf(TEXT("[SERVIDOR-]leu valores\n"));
							tamanho = sizeof(nusers);
							RegQueryValueEx(chave, TEXT("Nusers"), NULL, NULL, (LPBYTE)&nusers,&tamanho);
							tamanho = sizeof(users);
							RegQueryValueEx(chave, TEXT("Lista"), NULL, NULL, (LPBYTE)&users, &tamanho);
							
							//RegQueryValueEx(chave, TEXT("Versao"), NULL, NULL, (LPBYTE)&versao, &tamanho);
							//RegQueryValueEx(chave, TEXT("ListaInteiros"), NULL, NULL, (LPBYTE)&valores, &tamanho);
							
						}
						RegCloseKey(chave);
					

	//users[nusers]=tmp;
	for(x=0;x<nusers;x++){
		_tprintf(TEXT("[SERVIDOR] user %d nome:%s)\n"), x, users[x].nome);
		users[x].logado=FALSE;
	}
	//*************************
	t=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)sair,NULL,0,NULL);
		
	while(1){
		_tprintf(TEXT("[SERVIDOR-%d] Vou criar uma cópia do pipe '%s'... (CreateNamedPipe)\n"), GetCurrentThreadId(),PIPE_NAME);
		hPipe = CreateNamedPipe(PIPE_NAME, PIPE_ACCESS_DUPLEX, PIPE_WAIT | PIPE_TYPE_MESSAGE
							| PIPE_READMODE_MESSAGE, PIPE_UNLIMITED_INSTANCES, sizeof(buf), sizeof(buf), 1000, &sa);
		if(hPipe == INVALID_HANDLE_VALUE){
			_tperror(TEXT("Erro na criação do pipe!"));
			exit(-1);
		}
	


		_tprintf(TEXT("[SERVIDOR-%d] Esperar ligação de um cliente... (ConnectNamedPipe)\n"), GetCurrentThreadId());
		if(!ConnectNamedPipe(hPipe, NULL)){
			_tperror(TEXT("Erro na ligação ao cliente!"));
			exit(-1);
		}

		_tprintf(TEXT("[SERVIDOR-%d] Um cliente ligou-se...\n"), GetCurrentThreadId());
		//Lançar a thread para atender cliente novo
		CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Atende,(LPVOID)hPipe,0,NULL);
		Sleep(200);
	}

	CloseHandle(hPipe);
	Sleep(2000);
}
DWORD WINAPI Atende(LPVOID param){
	int n, resp;
	DATA data;
	HANDLE hPipe = (HANDLE) param;
	JOGO j;
	while (1) {
		ret = ReadFile(hPipe,&data , sizeof(DATA), (LPDWORD)&n, NULL);
		if (!ret || !n)
			break;

		_tprintf(TEXT("[SERVIDOR-%d] Recebi %d bytes: '%c','%s' e '%s'... (ReadFile)\n"),GetCurrentThreadId(),n,data.car, data.str1, data.str2);
		//apagar
		resp=data.id;

		//switch para codigo da funçao
		switch(data.id){
			case 1:Autentica(hPipe,data,users,nusers);break;
			case 2:if(!running){
						running=TRUE;
						
						jogo=InicioJogo(data); 
						jogo.p1pipe=hPipe;
						WriteFile(hPipe, &jogo, sizeof(JOGO),(DWORD *)&n, NULL);
						threadjogo=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)servidorjogo,NULL,0,NULL);
						
						WaitForSingleObject(threadjogo,INFINITE);
				   }else{

					   if(_tcsicmp(jogo.nomesJog2,TEXT(""))==0){
						   
						   
						   if(!jogodecorrer){
							  
							   _stprintf(jogo.nomesJog2,data.str1);
							  
							   jogo.p2pipe=hPipe;
							   WriteFile(hPipe, &jogo, sizeof(JOGO),(DWORD *)&n, NULL);
							    SetEvent(evento);
							   WaitForSingleObject(threadjogo,INFINITE);
						   }else{
							   _stprintf(j.nomesJog1,TEXT(""));
							    WriteFile(hPipe, &j, sizeof(JOGO),(DWORD *)&n, NULL);
						   }
					   }else{
						    _stprintf(j.nomesJog1,TEXT(""));
							WriteFile(hPipe, &j, sizeof(JOGO),(DWORD *)&n, NULL);
					   }
				   }
						break;
			case 5:
				WriteFile(hPipe, &nusers, sizeof(int),(DWORD *)&n, NULL);WriteFile(hPipe, &users, TAM*sizeof(User),(DWORD *)&n, NULL);break;
			case 7:nusers=adduser(hPipe,data,users,nusers);break;
			case 8:logoff(hPipe,data,users,nusers);break;
			default:break;
		}
	}
	return 0;
}

DWORD WINAPI sair(){
	TCHAR del[TAM];
	while (_tcsicmp(del,TEXT("sair"))) {
		scanf("%s",del);
	}

	if(RegCreateKeyEx(HKEY_LOCAL_MACHINE,TEXT("Software\\Forca"),0, NULL, REG_OPTION_VOLATILE,
						KEY_ALL_ACCESS, NULL, &chave, &queAconteceu) != ERROR_SUCCESS)
						_tprintf(TEXT("[SERVIDOR-] Erro ao criar/abrir chave\n"));
	else
						//Se a chave foi criada, inicializar os valores
		if(queAconteceu == REG_CREATED_NEW_KEY){
							

							
		}else if (queAconteceu == REG_OPENED_EXISTING_KEY){
							//Se a chave foi aberta, ler os valores lá guardados
			RegSetValueEx(chave, TEXT("Nusers"), 0, REG_DWORD, (LPBYTE) &nusers, sizeof(DWORD));
			RegSetValueEx(chave, TEXT("Lista"), 0, REG_BINARY, (LPBYTE) users, sizeof(users));
			_tprintf(TEXT("[SERVIDOR-]Registou valores\n"));
							
		}
	RegCloseKey(chave);
	exit(-1);
	return 0;
}



DWORD WINAPI servidorjogo(){
	
	int n, resp;
	DATA data;
	HANDLE hPipe;
	WaitForSingleObject(evento,20000);
	SetEvent(evento);
	jogodecorrer=TRUE;
	
	while (jogo.vencedor==0) {
		
		do{
			_tprintf(TEXT("[SERVIDOR-%d] espera de receber letras...jog1 (ReadFile)\n"),GetCurrentThreadId());
			ret = ReadFile(jogo.p1pipe,&data , sizeof(DATA), (LPDWORD)&n, NULL);
			if (!ret || !n){
				gestao();
				jogo.vencedor=2;
				WriteFile(jogo.p2pipe, &jogo, sizeof(JOGO),(DWORD *)&n, NULL);
				return 0;
			}
			_tprintf(TEXT("[SERVIDOR-%d] Recebi %d bytes: '%c','%s' e '%s' do %s... (ReadFile)\n"),GetCurrentThreadId(),n,data.car, data.str1, data.str2,jogo.nomesJog1);
			//apagar
			resp=data.id;
			//switch para codigo da funçao
			switch(data.id){
				//case 3:WriteFile(hPipe, &jogo, sizeof(JOGO),(DWORD *)&n, NULL);break;
				case 4:EnviarPalavraOuLetra(jogo.p1pipe,data,jogo,users,nusers);break;
				case 5:WriteFile(jogo.p1pipe, &nusers, sizeof(int),(DWORD *)&n, NULL);WriteFile(jogo.p1pipe, &users, TAM*sizeof(User),(DWORD *)&n, NULL);break;
				case 6:Desistir(jogo.p1pipe,data,jogo,users, nusers);break;
				case 8:logoff(jogo.p1pipe,data,users,nusers);break;
				default:break;
			}
		}while(data.id!=4);
		if(jogo.vencedor!=0){
			if(_tcsicmp(jogo.nomesJog2,TEXT(""))!=0){
				ReadFile(jogo.p2pipe,&data , sizeof(DATA), (LPDWORD)&n, NULL);
				if (!ret || !n){
					gestao();
					jogo.vencedor=2;
					WriteFile(jogo.p2pipe, &jogo, sizeof(JOGO),(DWORD *)&n, NULL);
					return 0;
				}
				WriteFile(jogo.p2pipe, &jogo, sizeof(JOGO),(DWORD *)&n, NULL);
				break;
			}
			break;

		}

		ret=ReadFile(jogo.p1pipe,&data , sizeof(DATA), (LPDWORD)&n, NULL);
		if (!ret || !n){
			gestao();
			jogo.vencedor=2;
			WriteFile(jogo.p2pipe, &jogo, sizeof(JOGO),(DWORD *)&n, NULL);
			return 0;
		}
		WriteFile(jogo.p1pipe, &jogo, sizeof(JOGO),(DWORD *)&n, NULL);
		_tprintf(TEXT("[SERVIDOR-%d] enviei:  jogo %s... (ReadFile)\n"),GetCurrentThreadId(),jogo.nomesJog1);
		
		//********se 2 jogadores
		if(_tcsicmp(jogo.nomesJog2,TEXT(""))!=0){
			ReadFile(jogo.p2pipe,&data , sizeof(DATA), (LPDWORD)&n, NULL);
			if (!ret || !n){
				gestao();
				jogo.vencedor=1;
				WriteFile(jogo.p1pipe, &jogo, sizeof(JOGO),(DWORD *)&n, NULL);
				return 0;
			}
			WriteFile(jogo.p2pipe, &jogo, sizeof(JOGO),(DWORD *)&n, NULL);
			_tprintf(TEXT("[SERVIDOR-%d] enviei:  jogo %s... (ReadFile)\n"),GetCurrentThreadId(),jogo.nomesJog2);
			do{
				_tprintf(TEXT("[SERVIDOR-%d] espera de receber letras...jog2 (ReadFile)\n"),GetCurrentThreadId());
				ret = ReadFile(jogo.p2pipe,&data , sizeof(DATA), (LPDWORD)&n, NULL);
				if (!ret || !n){
					gestao();
					jogo.vencedor=1;
					WriteFile(jogo.p1pipe, &jogo, sizeof(JOGO),(DWORD *)&n, NULL);
					return 0;
				}
				_tprintf(TEXT("[SERVIDOR-%d] Recebi %d bytes: '%c','%s' e '%s' do %s... (ReadFile)\n"),GetCurrentThreadId(),n,data.car, data.str1, data.str2,jogo.nomesJog2);
				//apagar
				resp=data.id;
				//switch para codigo da funçao
				switch(data.id){
					//case 3:WriteFile(hPipe, &jogo, sizeof(JOGO),(DWORD *)&n, NULL);break;
					case 4:EnviarPalavraOuLetra(jogo.p2pipe,data,jogo,users,nusers);break;
					case 5:WriteFile(jogo.p2pipe, &nusers, sizeof(int),(DWORD *)&n, NULL);WriteFile(jogo.p2pipe, &users, TAM*sizeof(User),(DWORD *)&n, NULL);break;
					case 6:Desistir(jogo.p2pipe,data,jogo,users, nusers);break;
					case 8:logoff(jogo.p2pipe,data,users,nusers);break;
					default:break;
				}
			}while(data.id!=4);

			if(jogo.vencedor!=0){
				
				ReadFile(jogo.p1pipe,&data , sizeof(DATA), (LPDWORD)&n, NULL);
				if (!ret || !n){
					gestao();
					jogo.vencedor=1;
					WriteFile(jogo.p1pipe, &jogo, sizeof(JOGO),(DWORD *)&n, NULL);
					return 0;
				}
				WriteFile(jogo.p1pipe, &jogo, sizeof(JOGO),(DWORD *)&n, NULL);
				break;
				
			}
			ReadFile(jogo.p2pipe,&data , sizeof(DATA), (LPDWORD)&n, NULL);
			if (!ret || !n){
				gestao();
				jogo.vencedor=1;
				WriteFile(jogo.p1pipe, &jogo, sizeof(JOGO),(DWORD *)&n, NULL);
				return 0;
			}
			WriteFile(jogo.p2pipe, &jogo, sizeof(JOGO),(DWORD *)&n, NULL);
			_tprintf(TEXT("[SERVIDOR-%d] enviei:  jogo %s... (ReadFile)\n"),GetCurrentThreadId(),jogo.nomesJog2);
			
		}

		ReadFile(jogo.p1pipe,&data , sizeof(DATA), (LPDWORD)&n, NULL);
		if (!ret || !n){
			gestao();
			jogo.vencedor=2;
			WriteFile(jogo.p2pipe, &jogo, sizeof(JOGO),(DWORD *)&n, NULL);
			return 0;
		}
		WriteFile(jogo.p1pipe, &jogo, sizeof(JOGO),(DWORD *)&n, NULL);
		_tprintf(TEXT("[SERVIDOR-%d] enviei:  jogo %s... (ReadFile)\n"),GetCurrentThreadId(),jogo.nomesJog1);
		//***********
	}

	gestao();
	return 0;
}
void gestao(){
	//desligar pipes meter reset do jogo
	
	jogodecorrer=FALSE;
	running=FALSE;
	//****************gere************
	for(int i=0;i<nusers;++i){
		if(_tcsicmp(jogo.nomesJog1,users[i].nome)==0){
			users[i].jogos++;
			if(jogo.vencedor==1)
					users[i].wins++;
		}
	}
	if(_tcsicmp(jogo.nomesJog2,TEXT(""))!=0){
		for(int i=0;i<nusers;++i){
			if(_tcsicmp(jogo.nomesJog2,users[i].nome)==0){
				users[i].jogos++;
				if(jogo.vencedor==2)
					users[i].wins++;
			}
		}
		
	}
	_stprintf(jogo.nomesJog1,TEXT(""));
	_stprintf(jogo.nomesJog2,TEXT(""));
}

JOGO InicioJogo(DATA data){
	JOGO jogo;
	int n,i;
	_stprintf(jogo.nomesJog1,data.str1);
	_stprintf(jogo.nomesJog2,TEXT(""));
	srand((unsigned)time(NULL));
	n=(rand()%8+1);
	switch(n){
		case 1:_stprintf(jogo.palavra,TEXT("SINCRONIZACAO\0"));break;
		case 2:_stprintf(jogo.palavra,TEXT("COMUNICACAO\0"));break;
		case 3:_stprintf(jogo.palavra,TEXT("SEMAFORO\0"));break;
		case 4:_stprintf(jogo.palavra,TEXT("CONCORRENCIA\0"));break;
		case 5:_stprintf(jogo.palavra,TEXT("FUNCIONALIDADES\0"));break;
		case 6:_stprintf(jogo.palavra,TEXT("DERROTADO\0"));break;
		case 7:_stprintf(jogo.palavra,TEXT("VENCEDOR\0"));break;
	}
	for(i=0;i<_tcslen(jogo.palavra);++i){
		jogo.palavraoculta[i]='*';
	}
	jogo.palavraoculta[i]='\0';
	jogo.jog1Erros=0;
	jogo.jog2Erros=0;
	jogo.vencedor=0;
	jogo.turno=1;
	
	_stprintf(jogo.letras,TEXT(""));

	return jogo;//pedido aceite
}