#include "Funcoes.h"



void Autentica(HANDLE hPipe,DATA data,User *users,int nusers){
	BOOL ret = FALSE;
	int n, resp,i;
	resp=0;
	for(int i=0;i<nusers;i++){
		if(_tcsicmp(data.str1,users[i].nome)==0 && _tcsicmp(data.str2,users[i].pass)==0){
			if(users[i].logado==FALSE){
				if(users[i].admin)
					resp=2;
				else
					resp=1;
				
				users[i].logado=TRUE;
				i=nusers;
			}else
				resp=-1;
		}
	}
	WriteFile(hPipe, &resp, sizeof(int),(DWORD *)&n, NULL);
	return;
}
void EnviarPalavraOuLetra(HANDLE hPipe,DATA data,JOGO &jogo,User *users,int nusers){
	int tamanho,i=0,e=0,n,resp;
	BOOL flag=FALSE;
	if(data.car!='0'){
		tamanho=_tcslen(jogo.letras);
		if(tamanho>27)
			tamanho=27;
		for(i=0;i<tamanho;i++){
			if(jogo.letras[i]==data.car){
				resp=2;//caracter existe
				WriteFile(hPipe, &resp, sizeof(int),(DWORD *)&n, NULL);
				return;
			}
		}
	}
	tamanho=_tcslen(jogo.palavra);
	if(data.str1[0]!='0'){
		if(_tcsicmp(data.str1,jogo.palavra)==0){
			resp=1;//palavra correcta
			jogo.vencedor=jogo.turno;
			WriteFile(hPipe, &resp, sizeof(int),(DWORD *)&n, NULL);
				
			for(int i=0;i<nusers;++i){
				if(_tcsicmp(data.str2,users[i].nome)==0){
					users[i].jogos++;
					users[i].wins++;	
					return;
				}
			}
			return;
		}else{
			for(i=0;i<tamanho;i++){
				if(jogo.palavra[i]==data.str1[i]){
					jogo.palavraoculta[i]=data.str1[i];
				}
				else{
					e++;
				}
			}
			if(jogo.turno==1){
				jogo.jog1Erros+=e;
				if(_tcsicmp(jogo.nomesJog2,TEXT(""))!=0)
					jogo.turno=2;
				if(jogo.jog1Erros>=ERRO){
					resp=-1;
					jogo.vencedor=2;
					WriteFile(hPipe, &resp, sizeof(int),(DWORD *)&n, NULL);
				
					return;
				}
			}else{
				jogo.jog2Erros+=e;
				jogo.turno=1;
				if(jogo.jog2Erros>=ERRO){
					resp=-1;
					jogo.vencedor=1;
					WriteFile(hPipe, &resp, sizeof(int),(DWORD *)&n, NULL);
				
					return;
				}
			}	
		}
		resp=0;
		WriteFile(hPipe, &resp, sizeof(int),(DWORD *)&n, NULL);
	}

	if(data.car!='0'){
		for(i=0;i<tamanho;i++){
			if(jogo.palavra[i]==data.car){
				jogo.palavraoculta[i]=data.car;
				flag=TRUE;
			}
		}
		tamanho=_tcslen(jogo.letras);
		jogo.letras[tamanho]=data.car;
		jogo.letras[tamanho+1]='\0';	
		if(!flag){
				if(jogo.turno==1){
					jogo.jog1Erros++;
					if(_tcsicmp(jogo.nomesJog2,TEXT(""))!=0)
						jogo.turno=2;
					if(jogo.jog1Erros>=ERRO){
						resp=-1;
						jogo.vencedor=2;
						WriteFile(hPipe, &resp, sizeof(int),(DWORD *)&n, NULL);
						return;
					}
				}else{
					jogo.jog2Erros++;
					jogo.turno=1;
					if(jogo.jog2Erros>=ERRO){
						resp=-1;
						jogo.vencedor=1;
						WriteFile(hPipe, &resp, sizeof(int),(DWORD *)&n, NULL);
						return;
					}
				}
		}else{
			if(jogo.turno==1&&_tcsicmp(jogo.nomesJog2,TEXT(""))!=0){
				jogo.turno=2;
			}else{
				jogo.turno=1;
			}
		}
		if(_tcsicmp(jogo.palavra,jogo.palavraoculta)==0){
				resp=1;
				jogo.vencedor=jogo.turno;
				WriteFile(hPipe, &resp, sizeof(int),(DWORD *)&n, NULL);
				return;
		}
	}
	resp=0;
	WriteFile(hPipe, &resp, sizeof(int),(DWORD *)&n, NULL);
}
void Desistir(HANDLE hPipe,DATA data,JOGO &jogo,User *users,int nusers){
	int resp,n;
	
	if(_tcsicmp(jogo.nomesJog2,TEXT(""))!=0){
		if(_tcsicmp(data.str1,jogo.nomesJog1)==0){
			for(int i=0;i<nusers;++i){
				if(_tcsicmp(jogo.nomesJog2,users[i].nome)==0){
					jogo.vencedor=2;
				}
			}
		}else{
			for(int i=0;i<nusers;++i){
				if(_tcsicmp(jogo.nomesJog1,users[i].nome)==0){
					jogo.vencedor=1;
				}
			}
		}
	}
	resp=-1;
	WriteFile(hPipe, &resp, sizeof(int),(DWORD *)&n, NULL);
}
int adduser(HANDLE hPipe,DATA data,User *users,int nusers){
	int resp,n;
	if(_tcsicmp(data.str1,"REMOVE")==0){
		for(int i=0;i<nusers;++i){
			if(_tcsicmp(data.str2,users[i].nome)==0){
				resp=0;
				WriteFile(hPipe, &resp, sizeof(int),(DWORD *)&n, NULL);

				nusers--;
				users[i]=users[nusers];
				_stprintf(users[nusers].nome,TEXT(""));
				return nusers;
			}
		}
		resp=-1;
		WriteFile(hPipe, &resp, sizeof(int),(DWORD *)&n, NULL);
		return nusers;
	}
	for(int i=0;i<nusers;++i){
		if(_tcsicmp(data.str1,users[i].nome)==0){
			resp=-2;
			WriteFile(hPipe, &resp, sizeof(int),(DWORD *)&n, NULL);
			return nusers;
		}
	}
	_stprintf(users[nusers].nome,data.str1);
	_stprintf(users[nusers].pass,data.str2);
	users[nusers].jogos=0;
	users[nusers].admin=FALSE;
	users[nusers].logado=FALSE;
	users[nusers].wins=0;
	nusers++;
	resp=1;
	WriteFile(hPipe, &resp, sizeof(int),(DWORD *)&n, NULL);
	return nusers;
}
void logoff(HANDLE hPipe,DATA data,User *users,int nusers){
	BOOL ret = FALSE;
	int n, resp,i;
	resp=0;
	for(int i=0;i<nusers;i++){
		if(_tcsicmp(data.str1,users[i].nome)==0){
			users[i].logado=FALSE;
			resp=1;
			i=nusers;
		}
	}
	WriteFile(hPipe, &resp, sizeof(int),(DWORD *)&n, NULL);

	return;
}
