// header file of main.c

#include "filter.h"

// functions in main.c
BOOL ReadOption(int, char**);
BOOL SetParam();
BOOL Filter();




//--------------------------------------------------
// function.c

int GetArgv(char*,char[MAX_ARGC][CHR_BUF],int);
void ShowAbout();
BOOL GetCmdLineFromFile(char*,char*);
void ShowStatus(WAVEFORMATEX waveFmt,  
				char* szWriteFile, // name of output file
/* obsolete
				BOOL bCreatePipe, // pipe mode
*/
				LONGLONG u64DataSize,  // output size 
				LONGLONG u64TotalSize, // total size of output file
				double dPeak,	   // peak
/* obsolete
				BOOL bADPtrain, // ADP training mode
*/
				BOOL bNormalGain, // now, normalizer is searching peak
				BOOL bStdin // stdin mode
				);


#ifdef USEWIN32API
/* obsolete
BOOL ExecCommand(LPSTR lpszCommandLine,PROCESS_INFORMATION*,BOOL,BOOL);
BOOL ExecPipeCommand(LPSTR,PROCESS_INFORMATION*,HANDLE*);
*/
BOOL GetFileMappingData(int,char*[ ],LPSTR,LPSTR);
VOID GetFreeHddSpace64(ULONGLONG*,LPSTR);
#endif


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
					   WAVEFORMATEX waveFmt,
					   LONGLONG n64DataSize // data size
#ifndef DEF_WAVEFLT
					   ,HWND hWnd,  // hwnd of lockon
					   DWORD dwRecTime, // recording time(sec) : if n64DataSize = 0, use this parameter
					   BOOL bOpenConsole, // open console
					   WORD wSaveMode // save mode
#endif
					   );

/* obsolete
#ifdef USEWIN32API
BOOL StopChildStop(PROCESS_INFORMATION,BOOL);
#endif
*/

#ifdef USEWIN32API
VOID AddSpase(HANDLE hdWriteFile,DWORD dwSize,WAVEFORMATEX waveFmt);
#else 
void AddSpase(FILE* hdWriteFile,DWORD dwSize,WAVEFORMATEX waveFmt);
#endif

BOOL ExchangeTime(LPSTR lpTime,double* lpSec);


BOOL ShowPeakWave(char* lpszFile, // name of input file
			 DWORD dwBufSize,
			 WAVEFORMATEX waveFmt,
			 LONGLONG n64Offset,  // offset 
			 LONGLONG n64DataSize);

void GetGainForNormalizer(double dNormalGain[2],
							BOOL bStereoLink,
							DWORD dwCurrentNormalMode,
							double dNormalLevel,
							LONGLONG n64TotalOutSize,
							WAVEFORMATEX waveFmt);
/* obsolete
void OutputADPFilterChr(char* lpszSaveDir,DWORD dwChn,DWORD dwSampleRate);
*/

void OutputFIRFilterChr(char* lpszSaveDir,WAVEFORMATEX waveFmt,DWORD dwFIRnum);
void OutputIIRFilterChr(char* lpszSaveDir,WAVEFORMATEX waveFmt,DWORD dwFIRnum);
void printIIRcoef(DWORD dwIirNum);
void OutputRsmpChr(char* lpszSaveDir,WAVEFORMATEX waveFmt);

//--------------------------------------------------
// playwave.c
#ifdef USEWIN32API
BOOL OpenWaveDevice(UINT,WAVEFORMATEX,DWORD);
BOOL PlayWave(BYTE*,DWORD);
BOOL CloseWaveDevice(BOOL);
#endif



//--------------------------------------------------
// io.c

#ifdef USEWIN32API
BOOL OpenReadFile(HANDLE* hdReadFile,char* szReadFile,
#else
BOOL OpenReadFile(FILE** hdReadFile,char* szReadFile,
#endif
					BOOL bStdin // stdin 
					);

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
					);

#ifdef USEWIN32API
void ReadData(HANDLE,BYTE*,DWORD,DWORD*);
#else
void ReadData(FILE*,BYTE*,DWORD,DWORD*);
#endif

#ifdef USEWIN32API
BOOL WriteData(HANDLE hdWriteFile,BYTE* lpBuffer,DWORD dwWriteByte,DWORD* lpdwByte
/* obsolete
			   ,BOOL bCreatePipe,
			   PROCESS_INFORMATION hProcessInfo
			   */
			   );
#else
BOOL WriteData(FILE* hdWriteFile,BYTE* lpBuffer,DWORD dwWriteByte,DWORD* lpdwByte,
/* obsolete
			   BOOL bCreatePipe
*/
			   );
#endif

BOOL WriteTextData(HANDLE hdWriteFile,
				   double* lpFilterBuf[2], // buffer
				   DWORD dwPointsInBuf, // points of data in buffer
				   WAVEFORMATEX waveFmt);



// outfile.c
void SetOutputFileName(LPSTR lpszBaseFile,  // base name
						 LPSTR lpszOutputFile, // output file name
						 DWORD dwNum // number
						 );


BOOL PrepareOutputFileName(LPSTR lpszConfigFile, // configuration file
#ifdef WIN32		
						HANDLE hFileMap, // handle of file mapping
#endif
						char* lpszErr
						);

void unprepareOutputFileName();


// rand.c
//void init_genrand(unsigned long s);

//----------------------------------------------------
// global 

char argv[MAX_ARGC][CHR_BUF],szCmdLine[MAX_WAVFLTOPTBUF];
WAVEFORMATEX InputWaveFmt; // format of input
WAVEFORMATEX WriteWaveFmt; // format of output
char SzReadFile[MAX_PATH]; // input file name
char SzOrgWriteFile[MAX_PATH]; // original output file name specified in command line
char SzWriteFile[MAX_PATH]; // (virtual) output file name
char SzRealWriteFile[MAX_PATH]; // (actual) output file name 
char SzCurDir[MAX_PATH]; // current directry
unsigned long long N64FileOffset; // byte, header offset of input file
unsigned long long N64FileDataSize; // byte, data size of input file
DWORD DwBufSize = 10; // buffer size
BOOL BlHeadOffset = false; // specify header offset of input file 
BOOL BlNoChkHdd = false; // no check space of HDD
BOOL BlWaveHdrOut = true; // output wave header
BOOL BlExtChunkOfHdr = true; // use extra wave header
BOOL BlUseSSE2 = true; // use sse2

LONGLONG N64OutOffset; // byte, offset of input file

BOOL BlStdin = false;  // read from stdin
BOOL BlStdout = false;  // write to stdout
BOOL BlVerbose = true; // verbose
BOOL BlEndless = false; // endless mode
char SzUserDef[3][CHR_BUF]; // user defined strings
SYSTEMTIME SystemTime; // system time

/* obsolete
// pipe
#ifdef USEWIN32API
PROCESS_INFORMATION hProcessInfo; 
#endif
DWORD DwPipeBufSize = 20; // pipe buffer size
BOOL BlCreatePipe = false;  // use pipe
char SzPipeComand[CHR_BUF]; // command of pipe (before replacement of strings)
char SzPipeCmd[CHR_BUF]; // command of pipe (after replacement of strings)

// command
BOOL BlExecCom = false;  // コマンドを録音後に実行するか
BOOL BlOpenWin = true;	// 別にウィンドウを表示するか
char SzExecComand[CHR_BUF]; // 実行コマンド(置換前)
char SzExecCmd[CHR_BUF]; // 実際に実行するコマンド
*/

// config of blocks
unsigned long long N64TotalDataSize; // total data size of output
unsigned long long N64RealTotalDataSize; // total data size of 'real' output file
unsigned long long N64OffsetBlk[MAXCOPYBLOCK]; // byte, offset of each block
unsigned long long N64DataSizeBlk[MAXCOPYBLOCK]; //  byte, copy size of each block
double DbStartTime[MAXCOPYBLOCK]; // sec, start time of each block
double DbEndTime[MAXCOPYBLOCK]; // sec, end time of each bloc
DWORD DwCopyBlock = 1; // number of blocks
BOOL BlCutTail = false;  // specified -cuttail option
BOOL BlCutFile = false;  // execute cutting file


// shift output file
double DbShiftTime = 0; // msec


// saturation
DWORD DwSaturate = 0; // number of saturation

// normalize
DWORD DwNormalMode = NORMAL_NOT; // mode
double DbNormalLevel = 0; // db, level
BOOL BlNormalStereo = true; // stereo link

// play sound (waveout)
BOOL BlWaveOut = false;
UINT UnWaveDevID = -1;  // WAVE_MAPPER

// text output
BOOL BlTextOut = false;

// print information of input file
BOOL BlFileInfo = false;

// delete input file
BOOL BlDelFile = false;

// add no sound part
double DbAddSp[2] = {0,0};
DWORD DwAddSp[2] = {0,0};

// input is 'nosound'
BOOL BlNoSignal = false;

/* obsolete
// HWND of 'lockon'
HWND HdWndLkm = NULL;
*/

// print characteristics of resampling
BOOL BlRsmpOutFilter; 

/* obsolete
// print characteristics of ADP filter
BOOL BlADPOutFilter; 
DWORD DwADPTrainTime; // sec, training time of ADP filter
*/

// print characteristics of FIR filter
BOOL BlFIROutFilter;  

// print characteristics of IIR filter
BOOL BlIIROutFilter;  


// time of fade in/out
double DbFadeInTime; // sec
double DbFadeOutTime; // sec


BOOL BlOutFile;  // use -outfile option
char SzOutFile[MAX_PATH];
#ifdef WIN32
HANDLE HdOutFileMap;  // use filemapping for config of the output file name
#endif

// current file number
DWORD DwCurSplitNo = 0; 


// set filestamp to creation time
BOOL BlFileStamp = false;
#ifdef USEWIN32API
FILETIME FtCreationTime,FtLastAccessTime,FtLastWriteTime;
#endif


FILTER_DATA FDAT;


//EOF