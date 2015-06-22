// counfiguration of output file name


#include "filter.h"

char* OUTFILE_szName[MAX_SPLIT]; 
DWORD OUTFILE_dwNum = 0;


//-------------------------------------------------------------------
// set the output file names from file mapping , or configuration file specified at -outfile
BOOL PrepareOutputFileName(LPSTR lpszConfigFile, // configuration file
#ifdef WIN32		
						HANDLE hFileMap, // handle of file mapping
#endif
						char* lpszErr
						){
	
#ifdef WIN32		
	HANDLE hMapAddress; // address of Mapped View
#endif
	char* lpszMap;
	FILE* f;
	char szFile[MAX_PATH],szStr[MAX_PATH];
	char szDriveName[MAX_PATH],szPathName[MAX_PATH],szFileName[MAX_PATH],szExt[MAX_PATH];

	long i,i2,pos;

	for(i=0;i<MAX_SPLIT;i++) OUTFILE_szName[i] = NULL;
	OUTFILE_dwNum = 0;

#ifdef WIN32		
	// from file mapping
	if(hFileMap){
		
		// get address of address of Mapped View
		if((hMapAddress = MapViewOfFileEx(hFileMap,FILE_MAP_ALL_ACCESS,0,0,0,NULL))==NULL){
			if(lpszErr) wsprintf(lpszErr,"Failed in MapViewOfFileEx (handle = %d)\n",(DWORD)hFileMap);
			return FALSE;
		}
			lpszMap = (LPSTR)hMapAddress;
			
			i2 = 0;
			for(i=0;i<MAX_SPLIT;i++){
				if(!OUTFILE_szName[i]) OUTFILE_szName[i] = (char*)malloc(MAX_PATH);
				pos = 0;
				while(lpszMap[i2] != '\n' // file names should be devided by '\n'
					&& lpszMap[i2] != '\0') 
					OUTFILE_szName[i][pos++] = lpszMap[i2++];
				OUTFILE_szName[i][pos] = '\0';
				OUTFILE_dwNum++;
				i2++;

				if(lpszMap[i2] == '\0') break;
			}
			
			UnmapViewOfFile(hMapAddress);
			CloseHandle(hFileMap);
	}
	else  
#endif
		// from file
	if(strlen(lpszConfigFile)){
		
		strcpy(szFile,lpszConfigFile);
		
		f = fopen(szFile,"r");
		if(f == NULL){ // retry in the same directry of waveflt
#ifdef WIN32   
			GetModuleFileName(NULL,szStr,MAX_PATH); 
			_splitpath(szStr,szDriveName,szPathName,NULL,NULL);	
			_splitpath(lpszConfigFile,NULL,NULL,szFileName,szExt);
			wsprintf(szFile,"%s%s%s%s",szDriveName,szPathName,szFileName,szExt);
			
			f = fopen(szFile,"r");
			
			if(f == NULL)
#endif
			{
				if(lpszErr) wsprintf(lpszErr,"Could not open '%s'\n",szFile);
				return false;
			}
		}
		
		while(feof(f) == 0){
			
			if(!OUTFILE_szName[OUTFILE_dwNum]) OUTFILE_szName[OUTFILE_dwNum] = (char*)malloc(MAX_PATH);
			
			fgets(OUTFILE_szName[OUTFILE_dwNum],MAX_PATH,f);
			
			// delete ' '  from head of file name
			i=i2=0;
			while(OUTFILE_szName[OUTFILE_dwNum][i2] == ' ') i2++; 
			while(OUTFILE_szName[OUTFILE_dwNum][i2] != '\0') 
				OUTFILE_szName[OUTFILE_dwNum][i++] = OUTFILE_szName[OUTFILE_dwNum][i2++];
			OUTFILE_szName[OUTFILE_dwNum][i] = '\0'; 

			// delete '\n', '\r' and ' ' from tail of file name
			i = strlen(OUTFILE_szName[OUTFILE_dwNum])-1;
			while( 
				(OUTFILE_szName[OUTFILE_dwNum][i] == '\n'
				|| OUTFILE_szName[OUTFILE_dwNum][i] == '\r'
				|| OUTFILE_szName[OUTFILE_dwNum][i] == ' ')
				&& i >= 0) 
				OUTFILE_szName[OUTFILE_dwNum][i--] = '\0';

			// this line is not empty
			if(i>0)	OUTFILE_dwNum++;
			
		}
		fclose(f);
	}

	return true;
}


//------------------------------
// unprepare
void unprepareOutputFileName(){

	long i;

	for(i=0;i<MAX_SPLIT;i++){
		if(OUTFILE_szName[i]) free(OUTFILE_szName[i]);
		OUTFILE_szName[i] = NULL;
	}
	OUTFILE_dwNum = 0;
}



//-------------------------------------------------------------------
// make the numbered name of output file ,(for example, output-000.wav), or
// set output name from configuration file
void SetOutputFileName(LPSTR lpszBaseFile,  // base name
						 LPSTR lpszOutputFile, // output file name
						 DWORD dwNum // number
						 ){
	
	DWORD i,N,pos;
	char fExt[CHR_BUF];
	
	if(dwNum < OUTFILE_dwNum)		
		strcpy(lpszOutputFile,OUTFILE_szName[dwNum]);
	else{
		
		N = strlen(lpszBaseFile);
		pos = N;
		for(i=N-1;i>=0;i--){
			if(lpszBaseFile[i] == '\\') break;
			else if(lpszBaseFile[i] == '.'){
				pos = i;
				break;
			}
		}
		
		memcpy(lpszOutputFile,lpszBaseFile,pos);
		lpszOutputFile[pos] = '\0';
		memcpy(fExt,lpszBaseFile+pos,N-pos);
		fExt[N-pos] = '\0';
		wsprintf(lpszOutputFile,"%s-%03d%s",lpszOutputFile,dwNum,fExt);
	}
}

// EOF