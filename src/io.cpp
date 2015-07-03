// I/O


#ifdef WIN32
#include <windows.h>
#define USEWIN32API  // use win32 APIs
#endif
#include <stdio.h>

#include "waveformat.h"

#define CHR_BUF 256 

/* obsolete
//-----------------------------------------
// execute pipe command
#define READ  0  
#define WRITE 1  
BOOL ExecPipeCommandOut(LPSTR lpszCommandLine,
					 PROCESS_INFORMATION* pProInfo,
					 HANDLE* lphPipeP2Cin,
					 DWORD dwPipeSize,
					 LPSTR lpszErr){
	
	HANDLE hOldIn;
	HANDLE hPipeP2C[2];  // handle of pipe (stdin)
	HANDLE hParent;
	BOOL bRet;
	SECURITY_ATTRIBUTES secAtt;
	STARTUPINFO startInfo;

	// SECURITY_ATTRIBUTES
	secAtt.nLength = sizeof(SECURITY_ATTRIBUTES);
	secAtt.lpSecurityDescriptor = NULL;
	secAtt.bInheritHandle = TRUE;  // inherit handle

	// STARTUPINFO
	ZeroMemory(&startInfo,sizeof(STARTUPINFO));
	startInfo.cb = sizeof(STARTUPINFO);

	// create pipe
	if(!CreatePipe(&hPipeP2C[READ],&hPipeP2C[WRITE],&secAtt,dwPipeSize)){
		if(lpszErr != NULL) wsprintf(lpszErr,"\nCould not create pipe.\n");
		return FALSE;
	}

	// don't inherit entrance side of pipe. so
	// handle of entrance side must be duplicated and then closed.
	hParent = GetCurrentProcess();
	if(!DuplicateHandle(hParent,hPipeP2C[WRITE],hParent,lphPipeP2Cin,0,FALSE,DUPLICATE_SAME_ACCESS)){
		if(lpszErr != NULL) wsprintf(lpszErr,"\nCould not duplicate handle.\n");
		CloseHandle(hPipeP2C[READ]);
		CloseHandle(hPipeP2C[WRITE]);
		return FALSE;
	}
	CloseHandle(hPipeP2C[WRITE]); // close entrance

	// set handle of stdin to exit side of pipe
	hOldIn = GetStdHandle(STD_INPUT_HANDLE);
	SetStdHandle(STD_INPUT_HANDLE,hPipeP2C[READ]);

	// execute child process
	if(!(bRet = CreateProcess(NULL,lpszCommandLine,NULL,NULL,TRUE,0,NULL,NULL,&startInfo,pProInfo))){
		if(lpszErr != NULL) wsprintf(lpszErr,"\nCould not execute pipe command.\n");
	}

	// reset handle of stdin to original
	SetStdHandle(STD_INPUT_HANDLE,hOldIn);
	CloseHandle(hPipeP2C[READ]);

	return bRet;
}
#undef READ
#undef WRITE 


//-------------------------------------
// wait until child process stop.
#define WAITCHILDCLOSETIME 10000 // msec
BOOL StopChildStop(PROCESS_INFORMATION hProcessInfo,BOOL bCreatePipe){
	

	DWORD dwExitCode;

	if(hProcessInfo.hProcess != NULL)
	{
		if(bCreatePipe)
		{
			Sleep(500);
			GetExitCodeProcess(hProcessInfo.hProcess,&dwExitCode);
			if(dwExitCode == STILL_ACTIVE) WaitForSingleObject(hProcessInfo.hProcess,WAITCHILDCLOSETIME);
			GetExitCodeProcess(hProcessInfo.hProcess,&dwExitCode);
			if(dwExitCode == STILL_ACTIVE) 
			{
				TerminateProcess(hProcessInfo.hProcess,2);
				fprintf(stderr,"Could not close child process.\n");
				return false;
			}
			fprintf(stderr,"child process closed (%d)\n",dwExitCode);
		}
	}
	return TRUE;
}
*/


//--------------------------------------------------------
// read data from file
#ifdef USEWIN32API
void ReadData(HANDLE hdReadFile,BYTE* lpBuffer,DWORD dwReadByte,DWORD* lpdwByte)
#else
void ReadData(FILE* hdReadFile,BYTE* lpBuffer,DWORD dwReadByte,DWORD* lpdwByte)
#endif
{
	DWORD dwByte,dwRemainByte,dwFoo;

	dwByte = 0;
	dwRemainByte = dwReadByte;
	while(dwByte < dwReadByte){
#ifdef USEWIN32API
		ReadFile(hdReadFile,lpBuffer+dwByte,dwRemainByte,&dwFoo,NULL);
#else
		dwFoo = fread(lpBuffer+dwByte,1,dwRemainByte,hdReadFile);
#endif
		dwByte += dwFoo;
		dwRemainByte -= dwFoo;
		if(dwFoo == 0 || dwRemainByte == 0) break;
	}
				
	*lpdwByte = dwByte;
}




//--------------------------------------------------------
// write data to file
#ifdef USEWIN32API
BOOL WriteData(HANDLE hdWriteFile,BYTE* lpBuffer,DWORD dwWriteByte,DWORD* lpdwByte
			   /* obsolete
			   ,BOOL bCreatePipe,
			   PROCESS_INFORMATION hProcessInfo
			   */
			   )
#else
BOOL WriteData(FILE* hdWriteFile,BYTE* lpBuffer,DWORD dwWriteByte,DWORD* lpdwByte
			   /* obsolete
			   ,BOOL bCreatePipe
			   */
			   )
#endif
{
	
#ifdef USEWIN32API // use WIN32 API (for pipe)

//	DWORD dwFoo;

	if(hdWriteFile == NULL) return true;

	/*
	// check whether pipe has not been broken
	if(bCreatePipe){
		GetExitCodeProcess(hProcessInfo.hProcess,&dwFoo);
		if(dwFoo != STILL_ACTIVE){
			fprintf(stderr,"Pipe has been broken.");
			return false;
		}
	}
	*/
	
	WriteFile(hdWriteFile,lpBuffer,dwWriteByte, lpdwByte, NULL);

#else
	*lpdwByte = fwrite(lpBuffer,1,dwWriteByte,hdWriteFile);
#endif

	return true;

}



//-----------------------------------------
// open read file
#ifdef USEWIN32API
BOOL OpenReadFile(HANDLE* hdReadFile,char* szReadFile,
#else
BOOL OpenReadFile(FILE** hdReadFile,char* szReadFile,
#endif
					BOOL bStdin // stdin 
					)
{
		
#ifdef USEWIN32API // use win32 APIs

	if(bStdin) *hdReadFile = GetStdHandle(STD_INPUT_HANDLE);  // stdin
	else if(strcmp(szReadFile,"nosignal")==0){ // nosignal
			*hdReadFile = NULL;
	}
	else // HDD
	{
		*hdReadFile = CreateFile(szReadFile,GENERIC_READ, 0, 0,OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);
		if(*hdReadFile == INVALID_HANDLE_VALUE){
			fprintf(stderr,"\nCould not open '%s'.\n",szReadFile);
			return false;;
		}	
	}
		
		
#else
		
		if(bOpenReadFile){
			if(bStdin) {
				*hdReadFile = stdin;
#ifdef WIN32
				_setmode(_fileno(stdin),_O_BINARY); 
#endif
			}
			else{
				
				*hdReadFile = fopen(szReadFile,"rb");
				if(*hdReadFile == NULL) {
					fprintf(stderr,"\nCould not open %s\n",szReadFile);
					return false;
				}		
			}
		}

#endif
	
	return true;
}


//-----------------------------------------
// open write file 
#ifdef USEWIN32API
BOOL OpenWriteFile(HANDLE* hdWriteFile,char* szWriteFile,
				   /* obsolete
				   PROCESS_INFORMATION* pProInfo,
				   */
#else
BOOL OpenWriteFile(FILE** hdWriteFile,char* szWriteFile,
#endif
					BOOL bStdout // stdou4
/* obsolete
					BOOL bCreatePipe, // create pipe
					char* szPipeCmd, // command of pipe
					DWORD dwPipeSize  // buffer size of pipe
*/
					)
{
		
#ifdef USEWIN32API // use win32 APIs

//	char szErr[CHR_BUF];
	
	if(bStdout){ // stdout
		//if(!bCreatePipe) 
			*hdWriteFile = GetStdHandle(STD_OUTPUT_HANDLE); 
		/* obsolete
		else{ 
			// create pipe
			if(!ExecPipeCommandOut(szPipeCmd,pProInfo,hdWriteFile,dwPipeSize,szErr)){
				fprintf(stderr,"%s\n",szErr);
				return false;
			}
		}*/
	}
	else if(strcmp(szWriteFile,"null")==0){ // NULL
		*hdWriteFile = NULL;
	}
	else{ // HDD
		
		*hdWriteFile = CreateFile(szWriteFile,GENERIC_WRITE, 
			0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL); 
		
		if(*hdWriteFile == INVALID_HANDLE_VALUE){
			fprintf(stderr,"\nCould not open '%s'\n",szWriteFile);
			CloseHandle(*hdWriteFile);
			return false;;
		}	
	}
		
#else
		
		if(bStdout) {
			*hdWriteFile = stdout;
#ifdef WIN32
			_setmode(_fileno(stdout),_O_BINARY); 
#endif
		}
		else if(strcmp(szWriteFile,"null")==0){ // NULL
			*hdWriteFile = NULL;
		}
		else{
			*hdWriteFile = fopen(szWriteFile,"wb");
			
			if(*hdWriteFile == NULL) {
				fprintf(stderr,"\nCould not open %s\n",szWriteFile);
				if(!bStdin) fclose(*hdReadFile);
				return false;
			}
		}
#endif
	
	return true;
}




//--------------------------------------------------------
// write text data to file
BOOL WriteTextData(HANDLE hdWriteFile,
				   double* lpFilterBuf[2], // buffer
				   DWORD dwPointsInBuf, // points of data in buffer
				   WAVFMT waveFmt)
{

	DWORD dwPos,dwChrLn,dwByte;
	char szOut[CHR_BUF];
	LONG nLevel[2];

	if(hdWriteFile == NULL) return true;

	for(dwPos=0;dwPos<dwPointsInBuf;dwPos++){

		nLevel[0] = (LONG)lpFilterBuf[0][dwPos];
		wsprintf(szOut,"%12d",nLevel[0]);
		if(waveFmt.channels == 2){
			nLevel[1] = (LONG)lpFilterBuf[1][dwPos];
			wsprintf(szOut,"%s %12d",szOut,nLevel[1]);
		}
		wsprintf(szOut,"%s\r\n",szOut);
		
		dwChrLn = strlen(szOut);
		WriteFile(hdWriteFile,szOut,dwChrLn,&dwByte,NULL);
	}

	return true;

}


// EOF
