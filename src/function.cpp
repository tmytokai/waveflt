#if 0

// common functions

#include "waveflt.h"
#include "wave.h"
#include "config.h"


//-------------------------------------------------------
// change strings, like '00:00:00', to time(sec)
// for example, 00:01:05 -> 65 
BOOL ExchangeTime(LPSTR lpszTime,double* lpSec){

	char szTemp[3][8],c;
	DWORD i,i2,i3,dwStrln;

	memset(szTemp,0,sizeof(char)*3*8);
	dwStrln = strlen(lpszTime);
	i2 = 0;i3 = 0;
	for(i=0;i<dwStrln;i++){
		c = lpszTime[i];
		if(c == ':'){
			i2++;
			i3 = 0;
			if(i2 > 2) return FALSE;
		}
		else szTemp[i2][i3++] = c;
	}
	if(i2 < 2) return FALSE;

	*lpSec = atof(szTemp[0])*3600+atof(szTemp[1])*60+atof(szTemp[2]);

	return TRUE;
}



//-------------------------------------------------------
// add no sound part in head and tail of file
#ifdef USEWIN32API
VOID AddSpace(HANDLE hdWriteFile,DWORD dwSize)
#else 
void AddSpace(FILE* hdWriteFile,DWORD dwSize)
#endif
{
	DWORD dwByte;
	BYTE* lpData;

	if(dwSize == 0) return;
	if(hdWriteFile == NULL) return;

	lpData = (BYTE*)malloc(dwSize+1024);
	memset(lpData,0,dwSize);
#ifdef USEWIN32API
	WriteFile(hdWriteFile, lpData,dwSize,&dwByte, NULL);
#else
	dwByte = fwrite(lpData,1,dwSize,hdWriteFile);
#endif
	free(lpData);
}





//----------------------------
// show current status
void ShowStatus(WaveFormat waveFmt,  
				char* szWriteFile, // name of output file
				LONGLONG u64DataSize,  // output size 
				LONGLONG u64TotalSize, // total size of output file
				double dPeak,	   // peak
				BOOL bNormalGain // now, normalizer is searching peak
				){
	
	double dTime;
	double dFoo;


	dTime = (double)u64DataSize/waveFmt.avgbyte();
	
		if(bNormalGain){ // normalizer is searching peak
		fprintf(stderr,"\015[%d %%]    ",
			(DWORD)((double)u64DataSize/u64TotalSize*100.)
			);
	}
	else{  

		if(u64DataSize < 1024*1024) fprintf(stderr,"\015[%lu k]",u64DataSize/1024);
		else fprintf(stderr,"\015[%lu M]",u64DataSize/1024/1024);

		fprintf(stderr,"[%.1lf s]",dTime);

		if(u64TotalSize > 0){
			dFoo = (double)u64DataSize/u64TotalSize*100.;
			fprintf(stderr,"[%d %%]",(DWORD)dFoo);
		}
		
		fprintf(stderr,"[peak: %6.3lf dB]",dPeak);

		fprintf(stderr,"   ");

	}

}




//-------------------------------------------------------------------
// print out the usage
void ShowAbout(){

	fprintf(stdout,"usage : waveflt [options] \"input.wav\" \"out.wav\"\n\n");
	fprintf(stdout,"input.wav : input file (stdin, nosignal)\n");
	fprintf(stdout,"out.wav : output file (stdout, waveout, null)\n");
	fprintf(stdout,"\n");

	fprintf(stdout,"[options]\n");
	fprintf(stdout,"\n");
	fprintf(stdout,"-cfg \"file\" : configuration file\n");
	fprintf(stdout,"\n");

	/* obsolete
	fprintf(stdout,"-tstamp : set timestamp to timestamp of input file\n");
	*/
	fprintf(stdout,"-info : file information (waveflt -info input.wav)\n");
	fprintf(stdout,"-silent : silent mode\n");
	fprintf(stdout,"-buf n : buffer size\n");
	fprintf(stdout,"-headofs byte : header offset size\n");
	fprintf(stdout,"-format sec freq chn bit tag : format of input file\n");
	fprintf(stdout,"-nowavehdr : no wave header\n");
	/* obsolete
	fprintf(stdout,"-noextrahdr : no extra wave header\n");
	*/
	fprintf(stdout,"-endless : endless mode\n");	
	fprintf(stdout,"-del : delete input file\n");
	fprintf(stdout,"-nochkhdd : no check space of HDD\n");
	fprintf(stdout,"-nosse2 : no use sse\n");
	/* obsolete
	fprintf(stdout,"-lockon hwd : window handle of 'LockOn'\n");
	*/
	fprintf(stdout,"\n");

	fprintf(stdout,"-vol n : volume\n");
	fprintf(stdout,"-balance l r : L*=l R*=r\n");
	fprintf(stdout,"\n");

	fprintf(stdout,"-cut t1 t2 : cut file\n");
	fprintf(stdout,"-cuttail t1 t2 : cut file from tail\n");
	fprintf(stdout,"-cutm n t11 t12 t21 t22 ... tn1 tn2 : multiple cut\n");
	fprintf(stdout,"-shift msec : shift output\n");
	fprintf(stdout,"\n");

//	fprintf(stdout,"-ofs left right : DC offset adjustment\n");
	fprintf(stdout,"-autoofs sec : auto DC offset adjustment\n");
	fprintf(stdout,"\n");
	
	fprintf(stdout,"-mix level : LR-channel mixing\n");
	fprintf(stdout,"-mixfile  l1 l2 t1 t2 \"file\" : file mixing\n");
	fprintf(stdout,"\n");

	fprintf(stdout,"-fin sec : fade in\n");
	fprintf(stdout,"-fout sec : fade out\n");
	fprintf(stdout,"\n");

	/* obsolete
	fprintf(stdout,"-adp level : adaptive filter\n");
	fprintf(stdout,"-adp_detail level tap delay train db : adaptive filter\n");
	fprintf(stdout,"-adpchr : print out characteristics of ADP filter\n");
	fprintf(stdout,"\n");
	*/

	fprintf(stdout,"-fir_lpf f : FIR low  pass filter\n");
	fprintf(stdout,"-fir_hpf f : FIR high Pass filter\n");
	fprintf(stdout,"-fir_bpf f1 f2 : FIR band pass filter\n");
	fprintf(stdout,"-fir_bsf f1 f2 : FIR band stop filter\n");
	fprintf(stdout,"-firln tap : length of FIR filter\n");
	fprintf(stdout,"-firdb db : loss\n");
	fprintf(stdout,"-firchr : print out characteristics of FIR filter\n");
	fprintf(stdout,"\n");

	fprintf(stdout,"-fir_eq3 l1 l2 l3 : 3 bands FIR-EQ\n");
	fprintf(stdout,"-fir_eq3_detail tap db q l1 l2 l3 : 3 bands FIR-EQ\n");
	fprintf(stdout,"-fir_eq l1 l2 ... l10 : 10 bands FIR-EQ\n");
	fprintf(stdout,"-fir_eq_detail tap db q l1 l2 ... l10 : 10 bands FIR-EQ\n");
	fprintf(stdout,"\n");

	fprintf(stdout,"-iir_lpf f : IIR low pass filter\n");
	fprintf(stdout,"-iir_hpf f : IIR  high pass filter\n");
	fprintf(stdout,"-iir_bpf f1 f2 : IIR band pass filter\n");
	fprintf(stdout,"-iirchr : print out characteristics of IIR filter\n");
	fprintf(stdout,"\n");

	fprintf(stdout,"-demp : de-emphasis (50/15ms)\n");
	fprintf(stdout,"-emp : emphasis (50/15ms)\n");
	fprintf(stdout,"\n");

	fprintf(stdout,"-sveq_l freq db : shelving EQ low\n");
	fprintf(stdout,"-sveq_h freq db : shelving EQ high\n");
	fprintf(stdout,"-pkeq freq db Q : peaking EQ\n");
	fprintf(stdout,"\n");
	
	fprintf(stdout,"-dither amp : dither + noise shaper\n");
	fprintf(stdout,"\n");

	fprintf(stdout,"-rsmp : re-sampling\n");
	fprintf(stdout,"-rsmp_detail tap db f : re-sampling\n");
	fprintf(stdout,"-rsmpchr : print out characteristics of re-sampling\n");
	fprintf(stdout,"-bit8 : bits of output file is 8\n");
	fprintf(stdout,"-bit16 : bits of output file is 16\n");
	fprintf(stdout,"-bit24 : bits of output file is 24\n");
	fprintf(stdout,"-bit32long : bits of output file is 32 long\n");
	fprintf(stdout,"-bit32float : bits of output file is 32 float\n");
	fprintf(stdout,"-bit64 : bits of output file is 64 double\n");
	fprintf(stdout,"\n");

	fprintf(stdout,"-ngate_f th : noise gate in the freq domain\n");
	fprintf(stdout,"\n");

	fprintf(stdout,"-phinv : phase invert (180 degrees)\n");
	fprintf(stdout,"\n");

	fprintf(stdout,"-normal db : peak normalizer \n");
	fprintf(stdout,"-normal_avg db : average normalizer\n");
	fprintf(stdout,"-normal_rms db : RMS normalizer\n");
	fprintf(stdout,"-normal_comp ratio th atk rel rms : compressor after normalizer\n");
	fprintf(stdout,"-normal_nolink : no stereo link\n");
	fprintf(stdout,"\n");

	fprintf(stdout,"-comp ratio th atk rel rms : compressor\n");
	fprintf(stdout,"\n");

	fprintf(stdout,"-ngate_sb th atk rel rms : subband noise gate\n");
	fprintf(stdout,"\n");

	fprintf(stdout,"-outfile \"file\" : configuration file of names of outputs\n");
	fprintf(stdout,"\n");

	fprintf(stdout,"-headcutting db : remove no sound part from head\n");
	fprintf(stdout,"-nosound msec db : split at no sound part\n");
	fprintf(stdout,"-nosound_detail mode msec db1 db2 : split at no sound part\n");
	fprintf(stdout,"-nosound_detail2 mode msec db1 db2 tt1 tt2 tt3 : split at no sound part\n");
	fprintf(stdout,"-nosound_stop msec db : stop recording at no sound part\n");
	fprintf(stdout,"-nosound_fir type low high tap db : FIR filter before -nosound*\n");
	fprintf(stdout,"\n");

	fprintf(stdout,"-split t: split at each t sec\n");
	fprintf(stdout,"-split2 n t1 t2 ...tn : split at t1,t2,...,tn sec\n");
	fprintf(stdout,"-split3 n t1 t2 ...tn : split at t1,t2,...,tn sec of input file\n");
	fprintf(stdout,"-splitkbyte n: split at each n kbyte of output data\n");
	fprintf(stdout,"\n");

	fprintf(stdout,"-addspc t1 t2 : add no sound part at head(t1 sec) and tail(t2 sec)\n");
	fprintf(stdout,"\n");

	fprintf(stdout,"-addsin n freq1 db1 ph1 ... : synthesize sine waves\n");
	fprintf(stdout,"\n");

	/* obsolete
	fprintf(stdout,"-exec \"command\" : execute command after filtering\n");
	fprintf(stdout,"-exec_nowin \"command\" : execute command in the same terminal\n");
	fprintf(stdout,"\n");
	*/

	fprintf(stdout,"-device n : no. of sound device\n");
	fprintf(stdout,"\n");

	fprintf(stdout,"-text : text output\n");
	fprintf(stdout,"\n");

	/* obsolete
	fprintf(stdout,"-pipeout \"command\" : command of pipe\n");
	fprintf(stdout,"-pipebuf n : size of pipe buffer\n");
	*/

	fprintf(stdout,"-udef1 \"comment\" : user defined strings 1 (%%1)\n");
	fprintf(stdout,"-udef2 \"comment\" : user defined strings 2 (%%2)\n");
	fprintf(stdout,"-udef3 \"comment\" : user defined strings 3 (%%3)\n");
	fprintf(stdout,"\n");

	CONFIG::usage();
}



//-----------------------------------
// get strings of command line   5/19/2002 
int GetArgv(char* lpszCmdLine,char argv[MAX_ARGC][CHR_BUF],int start_pos){
	
	int cmdln;
	int i,i2,argc;
	char c;
	
	cmdln = strlen(lpszCmdLine);
	for(i=0;i<MAX_ARGC;i++) argv[i][0] = '\0';

	argc = start_pos;
	if(cmdln > 0){

		i = 0;
		while(i < cmdln && argc < MAX_ARGC){

			// cancel space and return
			while(lpszCmdLine[i] == ' ' 
				|| lpszCmdLine[i] == '\n' || lpszCmdLine[i] == '\r') i++;

			if(i>=cmdln) break;

			// in case that strings is enclosed between " and ".
			if(lpszCmdLine[i] == '"') {
				c = '"'; 
				i++;
			}
			else c = ' ';

			// copy strings
			i2 = 0;
			while(lpszCmdLine[i] != '\0' && lpszCmdLine[i] != c){
				argv[argc][i2] = lpszCmdLine[i];
				i++;i2++;
			}

			if(c ==  '"') i++;
			
			argv[argc][i2] = '\0';
			argc++;
			i++;
		}
	}

	return argc;
}



//---------------------------------------
// read configration from cofig file  5/21/2002
BOOL GetCmdLineFromFile(char* lpszCmdLine, // output
						char* lpszFile  // config file
						){

	FILE *f;
	char str[MAX_WAVFLTOPTBUF],str2[MAX_WAVFLTOPTBUF],szFile[MAX_PATH];
	int i,i2;

	strcpy(szFile,lpszFile);

	f = fopen(szFile,"r");
	if(f == NULL) {
		
#ifdef WIN32   // try agein in the directry of waveflt
		char szStr[MAX_PATH];
		char szDriveName[MAX_PATH],szPathName[MAX_PATH],szFileName[MAX_PATH],szExt[MAX_PATH];
		GetModuleFileName(NULL,szStr,MAX_PATH); 
		_splitpath(szStr,szDriveName,szPathName,NULL,NULL);	
		_splitpath(szFile,NULL,NULL,szFileName,szExt);
		wsprintf(szFile,"%s%s%s%s",szDriveName,szPathName,szFileName,szExt);
		f = fopen(szFile,"r");

		if(f == NULL)
#endif
		{
			fprintf(stderr,"Could not open '%s'\n",szFile);
			return false;
		}
	}
	
	i=0;
	while(1){
		fgets(str2,MAX_WAVFLTOPTBUF,f);

		// delete space of head at each line
		i2 = 0; while(str2[i2] == ' ') i2++; strcpy(str,str2+i2);
		if(str[0] != '#' && str[0] != '\r' && str[0] != '\n'){
			// delete return, space
			i2 = strlen(str)-1; 
			while(str[i2] == '\n' || str[i2] == '\r' || str[i2] == ' ') i2--;
			str[i2+1] = '\0';
			wsprintf(lpszCmdLine+i,"%s ",str);
			i+=(strlen(str)+1);
		}

		if(feof(f) != 0) break;
	}
	lpszCmdLine[i] = '\0';

	fclose(f);

	return true;
}



#ifdef WIN32

/* obsolete
//--------------------------------------------------------
// execute command
BOOL ExecCommand(LPSTR lpszCommandLine,
				 PROCESS_INFORMATION* pProInfo, 
				 BOOL bInheritHandles,
				 BOOL bOpenWin	// open another window
				 ){

	BOOL bReturn;
	STARTUPINFO startInfo;
	PROCESS_INFORMATION proInfo;
	PROCESS_INFORMATION* lpProInfo;
	
	if(pProInfo == NULL) lpProInfo = &proInfo;
	else lpProInfo = pProInfo;
	
	
	memset(&startInfo,0,sizeof(STARTUPINFO));
	startInfo.cb = sizeof(STARTUPINFO);
	bReturn = CreateProcess(NULL,lpszCommandLine,NULL,NULL,bInheritHandles,
		CREATE_NEW_CONSOLE*bOpenWin,NULL,NULL,&startInfo,lpProInfo);

	return bReturn;
}
*/


//-------------------------------------------------------------------
// set the command line strings  2003/1/16 
// note: this function is used in both 'waveflt' and 'lockon'
#define MAX_COM_LNG 1024 
VOID SetCommandStrings(
#ifndef DEF_WAVEFLT
					   BOOL bCommand,
#endif
					   BOOL bFstdin,  // use 'fstdin'
					   LPSTR lpszCommand, // output strings
					   LPSTR lpszCommandIn, // input strings
					   LPSTR lpszFile,     // file name
					   LPSTR lpszUserDef1, // user defined strings 
					   LPSTR lpszUserDef2,
					   LPSTR lpszUserDef3,
					   SYSTEMTIME sysTime, // current time
					   WaveFormat waveFmt
					   )
{
	CHAR fPath[MAX_PATH],fDrive[MAX_PATH],fName[MAX_PATH],fExt[MAX_PATH];
	CHAR fPathM[MAX_PATH],fDriveM[MAX_PATH];
	int i,i2,i3;
	CHAR szStr[MAX_COM_LNG],lpszStrings[MAX_COM_LNG];
	WORD wTime;
	CHAR* szWeek[7] = {"SUN","MON","TUE","WED","THU","FRI","SAT"};

	// get module path
	GetModuleFileName(NULL,szStr,CHR_BUF);
	_splitpath(szStr,fDriveM,fPathM,NULL,NULL);	
	
#ifndef DEF_WAVEFLT
	if(bCommand && strlen(lpszCommandIn))
#else
	if(strlen(lpszCommandIn))
#endif
	{
		
		// delete spaces
		i=0; while(lpszCommandIn[i] == ' ') i++;
		strcpy(lpszStrings,lpszCommandIn+i);

		// SCMPX or WM8EUTIL or wavedemp or ssrc or wmcmd.vbs
		if(bFstdin){
			if(_strnicmp(lpszStrings,"scmpx",5) == 0
				|| _strnicmp(lpszStrings,"wm8eutil",8) == 0
				|| _strnicmp(lpszStrings,"wavedemp",8) == 0
				|| _strnicmp(lpszStrings,"ssrc",4) == 0
				|| _strnicmp(lpszStrings,"wmcmd.vbs",9) == 0
				)
			{
				strcpy(szStr,lpszStrings);
				strcpy(lpszStrings,"fstdin -dos ");

				// option
				if(_strnicmp(szStr,"wm8eutil",8) ==0)
					strcat(lpszStrings,"-v -dmysize %B ");
				else if(_strnicmp(szStr,"scmpx",5) == 0)
					strcat(lpszStrings,"-v -hide -dmysize %B ");
				else if(_strnicmp(szStr,"wavedemp",8) ==0)
					strcat(lpszStrings,"-adjwave \"%a\" -dmysize %B ");
				else if(_strnicmp(szStr,"wmcmd.vbs",9) ==0)
				{
					strcat(lpszStrings,"-v -dmysize %B ");
#ifndef DEF_WAVEFLT
					if(!bOpenConsole) strcat(lpszStrings,"-hidecon ");
#endif
				}

				// command
				strcat(lpszStrings,"-command \"");

				if(_strnicmp(szStr,"wmcmd.vbs",9) ==0)
				{
					strcat(lpszStrings,"cscript.exe %^");
					strcat(lpszStrings,fDriveM);
					strcat(lpszStrings,fPathM);
				}

				i2 = strlen(lpszStrings);
				i = 0;
				while(szStr[i]!='\0' && i2 < MAX_COM_LNG)
				{
					// stdin -> dummy.dat 
					if(_strnicmp(szStr+i,"stdin",5)==0)
					{
						strcat(lpszStrings,"dummy.dat");
						i+=4; i2+= 8;
					}
					// wmcmd.vbs -> wmcmd.vbs%^ 
					else if(_strnicmp(szStr+i,"wmcmd.vbs",9)==0)
					{
						strcat(lpszStrings,"wmcmd.vbs%^");
						i+=8; i2+= 10;
					}
#ifndef DEF_WAVEFLT  // " -> %^ 
					else if(szStr[i] == '"')
					{
						strcat(lpszStrings,"%^");
						i2++;
					}
#else  
					// %^ -> %%^ , and then %%^ -> %^ below. (waveflt)
					else if(_strnicmp(szStr+i,"%^",2)==0)
					{
						strcat(lpszStrings,"%%^");
						i++; i2+= 2;
					}
#endif
					else {
						lpszStrings[i2] = szStr[i]; 
					}
					i++;
					i2++;
					lpszStrings[i2] = '\0';
				}
				strcat(lpszStrings,"\"");
			}
			
		}
		//---------------------------------------

		_splitpath(lpszFile,fDrive,fPath,fName,fExt);
		i=0;i2=0;
		
		// replacement
		while(lpszStrings[i]!='\0' && i2 < MAX_COM_LNG)
		{
		 	if(lpszStrings[i]=='%'){
				
				switch(lpszStrings[i+1]){

#ifdef DEF_WAVEFLT  
					// %^ -> "  (waveflt)
				case '^':

					lpszCommand[i2] = '"';
					i2++;
					i+=2;

					break;
#endif



#ifndef DEF_WAVEFLT  
					// %L -> hwnd (lockon)
				case 'L':

					wsprintf(szStr,"%lu",(DWORD)hWnd);
					i3 = 0;
					while(szStr[i3]!='\0' && i2< MAX_COM_LNG)
					{
						lpszCommand[i2] = szStr[i3];
						i2++;i3++;
					}
					i+=2;

					break;
#endif

					// %
				case '%':
					
					lpszCommand[i2] = '%';
					i2++;
					i+=2;
					
					break;
					
					// drive
				case 'd':
					
					i3 = 0;
					while(fDrive[i3]!='\0' && i2 < MAX_COM_LNG)
					{
						lpszCommand[i2] = fDrive[i3];
						i2++;i3++;
					}
					i+=2;
					
					break;
					
					// path
				case 'p':
					
					i3 = 0;
					while(fPath[i3]!='\0' && i2 < MAX_COM_LNG)
					{
						lpszCommand[i2] = fPath[i3];
						i2++;i3++;
					}
					i+=2;
					
					break;
					
					
					// file
				case 'f':
					
					i3 = 0;
					while(fName[i3]!='\0' && i2 < MAX_COM_LNG)
					{
						lpszCommand[i2] = fName[i3];
						i2++;i3++;
					}
					i+=2;
					
					break;
					
					// ext
				case 'e':
					
					i3 = 0;
					while(fExt[i3]!='\0' && i2 < MAX_COM_LNG)
					{
						lpszCommand[i2] = fExt[i3];
						i2++;i3++;
					}
					i+=2;
					
					break;
					
					// file name
				case 'a':
					
					i3 = 0;
					while(lpszFile[i3]!='\0' && i2 < MAX_COM_LNG)
					{
						lpszCommand[i2] = lpszFile[i3];
						i2++;i3++;
					}
					i+=2;
					
					break;

				// user defined strings 1
				case '1':
					
					i3 = 0;
					while(lpszUserDef1[i3]!='\0' && i2 < MAX_COM_LNG)
					{
						lpszCommand[i2] = lpszUserDef1[i3];
						i2++;i3++;
					}
					i+=2;
					
					break;

					// user defined strings 2
				case '2':
					
					i3 = 0;
					while(lpszUserDef2[i3]!='\0' && i2 < MAX_COM_LNG)
					{
						lpszCommand[i2] = lpszUserDef2[i3];
						i2++;i3++;
					}
					i+=2;
					
					break;

				// user defined strings 3
				case '3':
					
					i3 = 0;
					while(lpszUserDef3[i3]!='\0' && i2 < MAX_COM_LNG)
					{
						lpszCommand[i2] = lpszUserDef3[i3];
						i2++;i3++;
					}
					i+=2;
					
					break;

					
					// year, month, day, 
				case 'Y':  
					
					wsprintf(szStr,"%04d",sysTime.wYear);
					for(i3=0;i3<2;i3++,i2++) lpszCommand[i2] = szStr[i3+2];
					i+=2;
					
					break;
					
				case 'M': wTime = sysTime.wMonth;goto L_TIMESET;
				case 'D': wTime = sysTime.wDay;	goto L_TIMESET;
				case 'H': wTime = sysTime.wHour; goto L_TIMESET;
				case 'N': wTime = sysTime.wMinute; goto L_TIMESET;
				case 'S': wTime = sysTime.wSecond; goto L_TIMESET;
L_TIMESET:
					wsprintf(szStr,"%02d",wTime);  
					for(i3=0;i3<2;i3++,i2++) lpszCommand[i2] = szStr[i3];
					i+=2;
					break;

				case 'W': // szWeek[7] = {"SUN","MON","TUE","WED","THU","FRI","SAT"};
					for(i3=0;i3<3;i3++,i2++) 
						lpszCommand[i2] = szWeek[sysTime.wDayOfWeek][i3];
					i+=2;
					
					break;

					/*
#ifdef DEF_WAVEFLT // 分割開始時間

				case 's':

					switch(lpszStrings[i+2]){
						
						// 年月日
					case 'Y':  // 2002 年の場合は下二桁の 02 が返る
						
						wsprintf(szStr,"%04d",splitTime.wYear);
						for(i3=0;i3<2;i3++,i2++) lpszCommand[i2] = szStr[i3+2];
						i+=3;
						
						break;
						
					case 'M': wTime = splitTime.wMonth;goto L_STIMESET;
					case 'D': wTime = splitTime.wDay;	goto L_STIMESET;
					case 'H': wTime = splitTime.wHour; goto L_STIMESET;
					case 'N': wTime = splitTime.wMinute; goto L_STIMESET;
					case 'S': wTime = splitTime.wSecond; goto L_STIMESET;
L_STIMESET:
						wsprintf(szStr,"%02d",wTime);  
						for(i3=0;i3<2;i3++,i2++) lpszCommand[i2] = szStr[i3];
						i+=3;
						break;
						
					case 'W': // 週 szWeek[7] = {"SUN","MON","TUE","WED","THU","FRI","SAT"};
						for(i3=0;i3<3;i3++,i2++) 
							lpszCommand[i2] = szWeek[splitTime.wDayOfWeek][i3];
						i+=3;
						
						break;
						
					default:
						lpszCommand[i2] = lpszStrings[i];
						lpszCommand[i2+1] = lpszStrings[i+1];
						i+=2;i2+=2;
					}
#endif
*/
					
				default:
					lpszCommand[i2] = lpszStrings[i];
					i++;i2++;
				}
			}
			else {
				lpszCommand[i2] = lpszStrings[i];
				i++;i2++;
			}
		}
		
		lpszCommand[i2] = '\0';
        }
        else wsprintf(lpszCommand,"");
}



//---------------------------------------------------------------------
// receive data from parent process by File Mapping Object.
// argv[argv-1] must be the handle of File Mapping Object.
BOOL GetFileMappingData(int argc, char *argv[ ],LPSTR lpszData,LPSTR lpszErr){

	HANDLE hFileMap; // handle of File Mapping 
	HANDLE hMapAddress; // address of Mapped View

	DWORD i,i2,strln;

	if(argc == 1){
		if(lpszErr != NULL) wsprintf(lpszErr,"\nCould no find handle of file mapping\n");
		return FALSE;
	}
	
	hFileMap = (HANDLE)strtoul(argv[argc-1],NULL,10);
	
	// mapping
	if((hMapAddress = MapViewOfFileEx(hFileMap,FILE_MAP_ALL_ACCESS,0,0,0,NULL))==NULL){
		if(lpszErr != NULL) wsprintf(lpszErr,"\nCould not map MappedView (%d)\n",(DWORD)hFileMap);
		return FALSE;
	}
	else{

		// receive data
		strcpy(lpszData,(LPSTR)hMapAddress);

		UnmapViewOfFile(hMapAddress);
		CloseHandle(hFileMap);

		// comment-out and return string
		strln = strlen(lpszData);
		for(i=0,i2=0;i<strln;i++,i2++){

			if(lpszData[i] == '#'){ // comment out
				while(i < strln && lpszData[i] != '\n'){i++;}
			}
			if(lpszData[i] == '\r' || lpszData[i] == '\n') lpszData[i] = ' ';  
			lpszData[i2] = lpszData[i];
		}
		lpszData[i2] = '\0';
	}

	return TRUE;
}








//-------------------------------------------------------------------
// get free space of HDD
typedef BOOL (WINAPI *PGETDISKEX)(LPCTSTR,PULARGE_INTEGER,PULARGE_INTEGER,PULARGE_INTEGER);

VOID GetFreeHddSpace64(ULONGLONG* lpullSize,LPSTR lpszFileName){
	
	CHAR fDrive[CHR_BUF],str[CHR_BUF];
	DWORD sectorClus,byteSect,freeClus,totalClus;
	DWORD dwType;
	HMODULE hModKer32;
	PGETDISKEX pGetDiskEx = NULL;
	ULARGE_INTEGER uliFreeByteAvailCaller;
	ULARGE_INTEGER uliTotalByte;
	
	hModKer32 = GetModuleHandle("kernel32.dll");
	
	if(hModKer32 != NULL){
		pGetDiskEx = (PGETDISKEX)GetProcAddress(hModKer32,"GetDiskFreeSpaceExA");
	}
	
	_splitpath(lpszFileName,str,NULL,NULL,NULL);
	wsprintf(fDrive,"%s\\",str);
	
	dwType = GetDriveType(fDrive);
	
	// if there isn't HDD , then space is 0
	if(dwType == DRIVE_NO_ROOT_DIR || dwType == DRIVE_UNKNOWN) uliFreeByteAvailCaller.QuadPart = 0;
	else {
		// old Windows
		if(pGetDiskEx == NULL){
			GetDiskFreeSpace(fDrive,&sectorClus,&byteSect,&freeClus,&totalClus);
			uliFreeByteAvailCaller.QuadPart = sectorClus*byteSect*freeClus;
		}
		else // FAT32,NTFS  
			(*pGetDiskEx)(fDrive,&uliFreeByteAvailCaller,&uliTotalByte,NULL);
	}
	
	*lpullSize = uliFreeByteAvailCaller.QuadPart;
}



#endif  // USEWIN32API





//-------------------------------------------------------------------
// get peak, average , RMS from file
void GetPeakFromFile(
#ifdef USEWIN32API
				HANDLE hdFile,  // file to inspect
#else
				FILE* hdFile,
#endif
				WaveFormat waveFmt,
				LONGLONG n64Offset, 
				LONGLONG n64DataSize,

				BYTE* lpBuffer, 
				DWORD dwBufSize, // buffer size

				double* lpdPeak,
				double* lpdAvg,
				double* lpdRms
				){
	
	DWORD dwByte;
	DWORD dwReadByte; 
	LONGLONG n64SearchedByte; 
	double dLevel[2]; 

	double dPeak[2],dAvg[2],dRms[2];
	double dMaxWaveLevel;

#ifdef USEWIN32API
	LARGE_INTEGER LI;
#endif
	
	DWORD i,i2;

#ifdef USEWIN32API
	LI.QuadPart = n64Offset; 
	SetFilePointer(hdFile,LI.LowPart, &LI.HighPart,FILE_BEGIN);
#else
	fseek(hdFile,n64Offset,SEEK_SET);
#endif

	for(i=0;i<waveFmt.channels();i++){dPeak[i] = dAvg[i] = dRms[i] = 0;}
	dMaxWaveLevel = waveFmt.get_max_level();
	n64SearchedByte = 0;
	while(n64SearchedByte < n64DataSize){
		
		if(n64DataSize > n64SearchedByte + dwBufSize) dwReadByte = dwBufSize;
		else dwReadByte = (DWORD)(n64DataSize - n64SearchedByte);
		
		if(waveFmt.bits() == 24) 
			dwReadByte =  dwReadByte / (waveFmt.channels() * (waveFmt.bits()/8)) * waveFmt.block();

#ifdef USEWIN32API
		ReadFile(hdFile,lpBuffer,dwReadByte,&dwByte, NULL);
#else
		dwByte = fread(lpBuffer,1,dwReadByte,hdFile);
#endif
		n64SearchedByte += dwByte;

		if(dwByte == 0) break;
		
		for(i = 0; i < dwByte;  i += waveFmt.block())
		{
			WaveLevel(dLevel,lpBuffer+i,waveFmt);
			
			for(i2=0;i2<waveFmt.channels();i2++){

				// normalize (-1 -> 1)
				dLevel[i2] /= dMaxWaveLevel; 
				
				// RMS
				dRms[i2] += dLevel[i2]*dLevel[i2];
				
				// average
				dAvg[i2] += fabs(dLevel[i2]);
				
				// peak
				dPeak[i2] = max(fabs(dLevel[i2]),dPeak[i2]);
			}
		}
		
		i = (DWORD)((double)n64SearchedByte/(double)n64DataSize*100);
		fprintf(stderr,"\015(%d %%)",i);
	}
	
	// result
	for(i=0;i<waveFmt.channels();i++){
		dAvg[i] /= (double)(n64SearchedByte/waveFmt.block());
		dRms[i] /= (double)(n64SearchedByte/waveFmt.block());
		dRms[i] = sqrt(dRms[i]);
	}

	for(i=0;i<waveFmt.channels();i++){
		lpdAvg[i] = dAvg[i];
		lpdRms[i] = dRms[i];
		lpdPeak[i] = dPeak[i];
	}

	fprintf(stderr,"\015");
}



//---------------------------------
// show peak,avg,RMS of file
BOOL ShowPeakWave(char* lpszFile, // name of input file
			 DWORD dwBufSize,
			 WaveFormat waveFmt, // format of file
			 LONGLONG n64Offset,  // offset 
			 LONGLONG n64DataSize) // size of data
{

#ifdef USEWIN32API
	HANDLE hdFile;
#else
	FILE* hdFile;
#endif

	BYTE* lpBuffer = NULL;
	double dPeak[2];
	double dAvarage[2],dRms[2];

	// open file
	if(strcmp(lpszFile,"stdin")==0){ // stdin
		
#ifdef USEWIN32API
		hdFile	= GetStdHandle(STD_INPUT_HANDLE); 
#else
		hdFile = stdin;
#ifdef WIN32
		_setmode(_fileno(stdin),_O_BINARY); 
#endif
		
#endif
	}
	else{ // file
		
#ifdef USEWIN32API
		hdFile = CreateFile(lpszFile,GENERIC_READ, 0, 0,OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);
		if(hdFile == INVALID_HANDLE_VALUE){
			fprintf(stderr,"Could not open file '%s'\n",lpszFile);
			return false;
		}						
#else
		hdFile = fopen(lpszFile,"rb");
		if(hdFile == NULL) {
			fprintf(stderr,"Could not open file '%s'\n",lpszFile);
			return false;
		}	
#endif		
	}

	lpBuffer = (BYTE*)malloc(dwBufSize+1024); 

	GetPeakFromFile(
				hdFile,
				waveFmt,
				n64Offset, 
				n64DataSize,

				lpBuffer, 
				dwBufSize, 
				dPeak,
				dAvarage,
				dRms);

	fprintf(stderr,"peak: L = %6.3lf dB (%6.3lf), L = %6.3lf dB (%6.3lf)\n",
		20.*log10(dPeak[0]),dPeak[0],20.*log10(dPeak[1]),dPeak[1]);
	fprintf(stderr,"AV. : L = %6.3lf dB, L = %6.3lf dB\n"
		,20.*log10(dAvarage[0]),20.*log10(dAvarage[1]));
	fprintf(stderr,"RMS : L = %6.3lf dB, R = %6.3lf dB\n"
		,20.*log10(dRms[0]),20.*log10(dRms[1]));

	// close file
#ifdef USEWIN32API
	CloseHandle(hdFile);
#else
	fclose(hdFile);
#endif
	
	if(lpBuffer != NULL) free(lpBuffer);

	return true;
}



//------------------------------------------------
// get gain for normalizer
void GetGainForNormalizer(double dNormalGain[2],
							BOOL bStereoLink,
							DWORD dwCurrentNormalMode,
							double dNormalLevel,
							LONGLONG n64TotalOutSize,
							WaveFormat waveFmt){
	
	double dMaxPeak[2],dMax;
	double dFoo[2];
	
	GET_PEAK(dFoo);
	fprintf(stderr,"\015peak: L = %6.3lf dB (%6.3lf), R = %6.3lf dB (%6.3lf)\n",
		20*log10(dFoo[0]),dFoo[0],20*log10(dFoo[1]),dFoo[1]);
	dMaxPeak[0] = dFoo[0];
	dMaxPeak[1] = dFoo[1];
	
	if(dwCurrentNormalMode == NORMAL_AVG){
		GET_AVG(dFoo);
		dFoo[0] /= (n64TotalOutSize/waveFmt.block());
		dFoo[1] /= (n64TotalOutSize/waveFmt.block());
		fprintf(stderr,"average: L = %6.3lf dB, R = %6.3lf dB\n",
			20*log10(dFoo[0]),20*log10(dFoo[1]));
	}
	else if(dwCurrentNormalMode == NORMAL_RMS){
		GET_RMS(dFoo);
		dFoo[0] /= (n64TotalOutSize/waveFmt.block());
		dFoo[1] /= (n64TotalOutSize/waveFmt.block());
		dFoo[0] = sqrt(dFoo[0]);
		dFoo[1] = sqrt(dFoo[1]);
		fprintf(stderr,"RMS: L = %6.3lf dB, R = %6.3lf dB\n",
			20*log10(dFoo[0]),20*log10(dFoo[1]));
	}		

	dNormalGain[0] = pow(10,dNormalLevel/20)/dFoo[0];
	dNormalGain[1] = pow(10,dNormalLevel/20)/dFoo[1];

	if(bStereoLink){
		dMax = min(dNormalGain[0],dNormalGain[1]);
		dNormalGain[0] = dMax;
		dNormalGain[1] = dMax;
		fprintf(stderr,"gain(stereo link): L = R = %6.3lf\n",dNormalGain[0]);
	}
	else{
		fprintf(stderr,"gain: L = %6.3lf, R = %6.3lf\n",dNormalGain[0],dNormalGain[1]);
	}
	fprintf(stderr,"peak*gain: L = %6.3lf dB (%6.3lf), R= %6.3lf dB (%6.3lf)\n",
		20*log10(dMaxPeak[0] * dNormalGain[0]),dMaxPeak[0] * dNormalGain[0],
		20*log10(dMaxPeak[1] * dNormalGain[1]),dMaxPeak[1] * dNormalGain[1]);
}





//-----------------------------------
// write out the characteristics of filter to file
void WriteFilterChr(char* lpszImpulse, 
					char* lpszChara, 
					double* dImpulse,
					DWORD dwLength,
					DWORD dwSampleRate, // sampling rate
					DWORD dwMaxFreq
					){
	
	DWORD i;
	FILE *f;
	COMPLEX* cData;
	double dDb;
	double dFreq;

	// copy impulse
	cData = (COMPLEX*)malloc(sizeof(COMPLEX)*dwLength);
	memset(cData,0,sizeof(COMPLEX)*dwLength);
	for(i=0;i<dwLength;i++) cData[i].r = dImpulse[i];

	if(lpszImpulse != NULL){
		f = fopen(lpszImpulse,"w");
		if(f == NULL) fprintf(stderr,"Could not open '%s'\n",lpszImpulse);
		else {
			for(i=0;i<dwLength;i++) fprintf(f,"%4d %lf\n",i,cData[i].r);
			fprintf(stderr,"%s\n",lpszImpulse);
			fclose(f);
		}
	}

	// characteristics of freq domain
	if(lpszChara != NULL){
		f = fopen(lpszChara,"w");
		if(f == NULL) fprintf(stderr,"Could not open '%s'\n",lpszImpulse);
		else {
			for(dDb=-60. ;dDb <0; dDb+= .5){ 
				dFreq = dwMaxFreq * pow(10.,dDb/20);
				fprintf(f,"%5d %lf\n",
				(LONG)dFreq,20*log10(CalcSpec(cData,dwLength,dFreq,(double)dwSampleRate)));
			}
			fprintf(stderr,"%s\n",lpszChara);
			fclose(f);
		}
	}

	free(cData);
}




/* obsolete
//---------------------------------
//  write out characteristics of ADP filter to file
void OutputADPFilterChr(char* lpszSaveDir,DWORD dwChn,DWORD dwSampleRate)
{
	
	char szADPImp[MAX_PATH],szADPChr[MAX_PATH]; 
	double* dCoeff[2];
	DWORD dwLength;

	fprintf(stderr,"\n");
	fprintf(stderr,"Write characteristics of ADP filter to:\n");

	dwLength = GetADPCoef(dCoeff);

	if(dwChn == 2){

		// left
		wsprintf(szADPImp,"%sadp_imp_l.txt",lpszSaveDir);
		wsprintf(szADPChr,"%sadp_chr_l.txt",lpszSaveDir);
		WriteFilterChr(szADPImp,szADPChr,dCoeff[0],dwLength,dwSampleRate,dwSampleRate/2);
		
		// right
		wsprintf(szADPImp,"%sadp_imp-r.txt",lpszSaveDir);
		wsprintf(szADPChr,"%sadp_chr-r.txt",lpszSaveDir);
		WriteFilterChr(szADPImp,szADPChr,dCoeff[1],dwLength,dwSampleRate,dwSampleRate/2);
	}
	else{ // mono
		wsprintf(szADPImp,"%sadp_imp.txt",lpszSaveDir);
		wsprintf(szADPChr,"%sadp_chr.txt",lpszSaveDir);
		WriteFilterChr(szADPImp,szADPChr,dCoeff[0],dwLength,dwSampleRate,dwSampleRate/2);
	}

	fprintf(stderr,"\n");
}
*/


//---------------------------------
//  write out characteristics of FIR filter to file
void OutputFIRFilterChr(char* lpszSaveDir,WaveFormat waveFmt,DWORD dwFIRnum)
{
	char szImp[MAX_PATH],szChr[MAX_PATH]; 
	double *coef;
	DWORD dwFIRleng;

	dwFIRleng = GetFIRCoef(&coef,dwFIRnum,0);

	wsprintf(szImp,"%s\\fir_imp.txt",lpszSaveDir);
	wsprintf(szChr,"%s\\fir_chr.txt",lpszSaveDir);

	fprintf(stderr,"\n");
	fprintf(stderr,"Write characteristics of FIR filter to:\n");
	WriteFilterChr(szImp,szChr,coef,dwFIRleng,waveFmt.rate(),waveFmt.rate()/2);

}



//---------------------------------
//  write out characteristics of IIR filter to file
void OutputIIRFilterChr(char* lpszSaveDir,WaveFormat waveFmt,DWORD dwIirNum){
	
	double* impulse;
	DWORD dwLength;

	char szImp[MAX_PATH];
	char szChr[MAX_PATH]; 
	
	wsprintf(szImp,"%s\\iir_imp.txt",lpszSaveDir);
	wsprintf(szChr,"%s\\iir_chr.txt",lpszSaveDir);

	dwLength = 2048;
	impulse = (double*)malloc(sizeof(double)*dwLength);

	GetIIRimpulse(impulse,dwLength,dwIirNum,0);

	fprintf(stderr,"\n");
	fprintf(stderr,"Write characteristics of IIR filter to:\n");
	WriteFilterChr(szImp,szChr,impulse,dwLength,waveFmt.rate(),waveFmt.rate()/2);

	free(impulse);
}



//---------------------------------------
// print out IIR coefficients
void printIIRcoef(DWORD dwIirNum){

	double* dCoef[2];
	DWORD dwDim;
	DWORD i;

	dwDim = GetIIRCoef(dCoef,dwIirNum,0);

	fprintf(stderr,"\n");
	if(dwIirNum == ID_IIR_NORMAL)fprintf(stderr,"coefficients of IIR filter:\n");
	else if(dwIirNum == ID_IIR_DEMP)fprintf(stderr,"coefficients of (de)emphasys:\n");
	else if(dwIirNum == ID_IIR_SVEQL)fprintf(stderr,"coefficients of shelving EQ low:\n");
	else if(dwIirNum == ID_IIR_SVEQH)fprintf(stderr,"coefficients of shelving EQ high:\n");
	else if(dwIirNum == ID_IIR_PKEQ)fprintf(stderr,"coefficients of pieaking EQ:\n");

	for(i=0;i<=dwDim;i++) fprintf(stderr,"b%d = %lf ",i,dCoef[1][i]);
	fprintf(stderr,"\n");
	for(i=0;i<=dwDim;i++) fprintf(stderr,"a%d = %lf ",i,dCoef[0][i]);
	fprintf(stderr,"\n");
}



//---------------------------------
//  write out characteristics of FIR filter of re-sampling to file
void OutputRsmpChr(char* lpszSaveDir,WaveFormat waveFmt)
{
	DWORD dwFIRleng,dwUp;
	char szChr[MAX_PATH]; 
	double *h;

	wsprintf(szChr,"%s\\rsmp_chr.txt",lpszSaveDir);
	
	dwFIRleng = GetRSMPCoef(&h,waveFmt.rate(),&dwUp);
	
	fprintf(stderr,"\n");
	fprintf(stderr,"Write characteristics of re-sampling to:\n");
	WriteFilterChr(NULL,szChr,h,dwFIRleng,waveFmt.rate()*dwUp,waveFmt.rate()/2);

	free(h);
	
}



//EOF

#endif