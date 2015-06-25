// main 

#include "main.h"
#include "config.h"

//---------------------------------
// main 
int main(int argc,char *argv[]){

	double dFoo[2];
	char szVer[CHR_BUF];

	// show title
	GetVer(szVer);
	if(argc == 1)
		fprintf(stdout,"%s\n\n",szVer);	
	else
		fprintf(stderr,"%s\n\n",szVer);	

	//  Get Current Directory
	GetCurrentDirectory(MAX_PATH,SzCurDir); 
	fprintf(stderr,"current directory: %s\n",SzCurDir);
	wsprintf(SzCurDir,"%s\\",SzCurDir); // add '\'

	// read option
	if(!ReadOption(argc,argv)){
		fprintf(stderr,"Fatal error.\n");
		exit(1);
	}

	// initialize parameter 
	if(!SetParam()){
		fprintf(stderr,"Fatal error.\n");
		exit(1);
	}

	if(BlFileInfo) goto EXIT_WAVEFLT;

	// filter
	if(!Filter()){
		fprintf(stderr,"Fatal error.\n");
		exit(1);
	}

	GET_PEAK(dFoo);
	if(dFoo[0] != 0 || dFoo[1] != 0)
		fprintf(stderr,"peak: L = %6.3lf dB (%6.3lf), R = %6.3lf dB (%6.3lf)\n",
		20*log10(dFoo[0]),dFoo[0],20*log10(dFoo[1]),dFoo[1]);
	if(DwSaturate > 0) fprintf(stderr,"peak over: %d times.\n",DwSaturate);

EXIT_WAVEFLT:  

	// -del
	if(BlDelFile && !BlStdin){
		if(remove(SzReadFile) == 0){
			fprintf(stderr,"delete : %s\n",SzReadFile);
		}
		else 
			fprintf(stderr,"Could not delete '%s'\n",SzReadFile);
	}
	
	fprintf(stderr,"done. (waveflt)\n");
	exit(0);
}


//----------------------------------------------------------------
//----------------------------------------------------------------
//----------------------------------------------------------------


//----------------------------------------------
// read option
BOOL ReadOption(int argc, char *argv_org[]){
	
	DWORD i,i2,i3,i4;
	double dFoo,dFoo2;
	int argc2;
	char szErr[CHR_BUF];
	
	if(argc < 3){
		ShowAbout();
		exit(1);
	}

	// initilize parameters
	SetDefaultOption(&FDAT);
	CONFIG::reset();

	memset(SzUserDef,0,sizeof(char)*3*CHR_BUF);
	memset(&InputWaveFmt,0,sizeof(WAVEFORMATEX));
	memset(&WriteWaveFmt,0,sizeof(WAVEFORMATEX));
	SetWaveFmt(&InputWaveFmt,2,44100,16,WAVE_FORMAT_PCM);
	N64FileDataSize = 180 * InputWaveFmt.nAvgBytesPerSec;

	// read config from file mapping or configuration file
	argc2 = 0; 
	for(i=0; i <(unsigned int)argc; i++, argc2++){

#ifdef USEWIN32API
		if(strcmp(argv_org[i],"-fmap")==0){ // file mapping

			if(!GetFileMappingData(argc,argv_org,szCmdLine,szErr)){
				fprintf(stderr,szErr);
				return false;
			}
			argc2 += (GetArgv(szCmdLine,argv+argc2,0)-1);
			argc--;
		}
		else 
#endif
			if(strcmp(argv_org[i],"-cfg")==0){ // configuration file
				
				if(!GetCmdLineFromFile(szCmdLine,argv_org[i+1])){
					return false;
				}
				argc2 += (GetArgv(szCmdLine,argv+argc2,0)-1);
				i++;
			}
			else  strcpy(argv[argc2],argv_org[i]);
	}
	argc = argc2;

	// information of input file
	if(strcmp(argv[1],"-info")==0){
		BlFileInfo = true;
		strcpy(SzReadFile,argv[2]);
		return true;
	}


	// read option
	i2 = 0;
	for(i=1;i<(unsigned int)argc-2;i++)
	{
		const int ret_config = CONFIG::analyze_argv( argv+i );
		if( ret_config ) i2 += ret_config;

		// silent mode
		else if(strcmp(argv[i],"-silent")==0) 
		{
			BlVerbose = false;
			i2++;
		}
/* obsolete

		// HWND of lockon
		else if(strcmp(argv[i],"-lockon")==0) 
		{
			HdWndLkm = (HWND)strtoul(argv[i+1],NULL,10);
			i++; i2+=2;
		}
*/
		// delete input file
		else if(strcmp(argv[i],"-del")==0) 
		{
			BlDelFile = true;
			i2++;
		}

		// don't use SSE2
		else if(strcmp(argv[i],"-nosse2")==0) {
			BlUseSSE2 = false;
			i2++;
		}

		// don't check free space of HDD
		else if(strcmp(argv[i],"-nochkhdd")==0) 
		{
			BlNoChkHdd = true;
			i2++;
		}

		// set user defined strings
		else if(strcmp(argv[i],"-udef1")==0) 
		{
			strcpy(SzUserDef[0],argv[i+1]);
			i++; i2+=2;
		}
		else if(strcmp(argv[i],"-udef2")==0) {
			strcpy(SzUserDef[1],argv[i+1]);
			i++; i2+=2;
		}
		else if(strcmp(argv[i],"-udef3")==0) {
			strcpy(SzUserDef[2],argv[i+1]);
			i++; i2+=2;
		}

		// endless mode
		else if(strcmp(argv[i],"-endless")==0) 
		{
			BlEndless = true;
			i2++;
		}		

		// dither 
		else if(strcmp(argv[i],"-dither")==0) 
		{
			FDAT.bDither = true;
			FDAT.dDitherAmp  = atof(argv[i+1]);
			if(FDAT.dDitherAmp <= 0) FDAT.dDitherAmp = 0.5;
			i++; i2+=2;
		}

		// peak normalizer(dB)
		else if(strcmp(argv[i],"-normal")==0) 
		{
			DwNormalMode = NORMAL_PEAKBYDB;
			DbNormalLevel = min(0,max(-192.0,atof(argv[i+1])));
			i++; i2+=2;
		}

		// average normalizer 
		else if(strcmp(argv[i],"-normal_avg")==0)
		{
			DwNormalMode = NORMAL_AVG;
			DbNormalLevel = min(0,max(-192.0,atof(argv[i+1])));
			i++; i2+=2;
		}

		// RMS normalizer
		else if(strcmp(argv[i],"-normal_rms")==0)
		{
			DwNormalMode = NORMAL_RMS;
			DbNormalLevel = min(0,max(-192.0,atof(argv[i+1])));
			i++; i2+=2;
		}		

		// no stereolink in  normalizer
		else if(strcmp(argv[i],"-normal_nolink")==0) 
		{
			BlNormalStereo = false;
			i2++;
		}

		// config of compressor after normalizer
		else if(strcmp(argv[i],"-normal_comp")==0)
		{
			FDAT.dNormalRatio = max(1,min(1000,atof(argv[i+1])));
			FDAT.dNormalTh=  min(0,max(-192.0,atof(argv[i+2])));
			FDAT.dNormalAttack  = max(0,min(5000,atof(argv[i+3])));
			FDAT.dNormalRelease  = max(0,min(5000,atof(argv[i+4])));
			FDAT.dwNormalCompRMS  = max(0,min(10000,atoi(argv[i+5])));

			i+=5; i2+=6;
		}

		// buffer size
		else if(strcmp(argv[i],"-buf")==0) 
		{
			DwBufSize = max(1,min(50,atoi(argv[i+1])));
			i++; i2+=2;
		}

		/* obsolete
		// buffer size of pipe
		else if(strcmp(argv[i],"-pipebuf")==0) 
		{
			DwPipeBufSize = max(1,min(50,atoi(argv[i+1])));
			i++; i2+=2;
		}
		*/

		// gain
		else if(strcmp(argv[i],"-vol")==0) 
		{
			FDAT.dVolume = max(0.0001,min(100.,atof(argv[i+1])));
			i++; i2+=2;
		}

		// balance
		else if(strcmp(argv[i],"-balance")==0) 
		{
			FDAT.bBalance = true;
			FDAT.dBalance[0] = max(0,min(100.,atof(argv[i+1])));
			FDAT.dBalance[1] = max(0,min(100.,atof(argv[i+2])));
			i+=2; i2+=3;
		}

		// fade in
		else if(strcmp(argv[i],"-fin")==0) {
			DbFadeInTime = max(0.1,min(10.,atof(argv[i+1])));
			i++; i2+=2;
		}

		// fade out
		else if(strcmp(argv[i],"-fout")==0) {
			DbFadeOutTime = max(0.1,min(10.,atof(argv[i+1])));
			i++; i2+=2;
		}
		
		// DC offset
		else if(strcmp(argv[i],"-ofs")==0) {
			FDAT.bOffset = true; 
			FDAT.dOffset[0] = atof(argv[i+1]);
			FDAT.dOffset[1] = atof(argv[i+2]);
			i+=2; i2+=3;
		}

		// auto adjust DC offset
		else if(strcmp(argv[i],"-autoofs")==0) {
			FDAT.bAutoOffset = true; 
			FDAT.dwAutoOffsetTime = max(5,atoi(argv[i+1]));
			i++;i2+=2;
		}
		
		// cut
		else if(strcmp(argv[i],"-cut")==0) {
			BlCutFile = true;
			DwCopyBlock = 1;
			if(!ExchangeTime(argv[i+1],&DbStartTime[0]))
				DbStartTime[0] = max(0,atof(argv[i+1]));
			if(!ExchangeTime(argv[i+2],&DbEndTime[0]))
				DbEndTime[0] = max(0,atof(argv[i+2]));
			i+=2; i2+=3;
		}
		
		else if(strcmp(argv[i],"-cutm")==0) {
			BlCutFile = true;
			DwCopyBlock = max(0,min(MAXCOPYBLOCK,atoi(argv[i+1])));
			i++; i2+=2;
			for(i3 = 0; i3 < DwCopyBlock; i3++){
				if(!ExchangeTime(argv[i+1],&DbStartTime[i3]))
					DbStartTime[i3] = max(0,atof(argv[i+1]));
				if(!ExchangeTime(argv[i+2],&DbEndTime[i3]))
					DbEndTime[i3] = max(0,atof(argv[i+2]));
				i+=2; i2+=2;
			}
		}
		
		else if(strcmp(argv[i],"-cuttail")==0) {
			BlCutFile = true;
			BlCutTail = true;
			DwCopyBlock = 1;
			if(!ExchangeTime(argv[i+1],&DbStartTime[0]))
				DbStartTime[0] = max(0,atof(argv[i+1]));
			if(!ExchangeTime(argv[i+2],&DbEndTime[0]))
				DbEndTime[0] = max(0,atof(argv[i+2]));
			i+=2; i2+=3;
		}


		// specify offset and size of blocks directly
#ifdef WIN32
		else if(strcmp(argv[i],"-cutb")==0) {
			BlCutFile = true;
			DwCopyBlock = 1;
			N64OffsetBlk[0] = _atoi64(argv[i+1]);
			N64DataSizeBlk[0] = _atoi64(argv[i+2]);
			i+=2; i2+=3;
		}
		
		else if(strcmp(argv[i],"-cutmb")==0) {
			BlCutFile = true;
			DwCopyBlock = max(0,min(MAXCOPYBLOCK,atoi(argv[i+1])));
			i++; i2+=2;
			for(i3 = 0; i3 < DwCopyBlock; i3++){
				N64OffsetBlk[i3] = _atoi64(argv[i+1]);
				N64DataSizeBlk[i3] = _atoi64(argv[i+2]);
				i+=2; i2+=2;
			}
		}
#endif

		// -shift
		else if(strcmp(argv[i],"-shift")==0) {
			DbShiftTime = max(0,atof(argv[i+1]));
			i++; i2+=2;
		}
		
		// mix
		else if(strcmp(argv[i],"-mix")==0) {
			FDAT.bMixLR = true;
			FDAT.dMixLRLevel = max(0,min(1,atof(argv[i+1])));
			i++; i2+=2;
		}

		// mixfile
		else if(strcmp(argv[i],"-mixfile")==0) {
			FDAT.bMixFile = true;
			FDAT.dMixLevel[0]  = max(0,min(10.,atof(argv[i+1])));
			FDAT.dMixLevel[1]  = max(0,min(10.,atof(argv[i+2])));
			if(!ExchangeTime(argv[i+3],&FDAT.dMixStartTime[0]))
				FDAT.dMixStartTime[0] = max(0,atof(argv[i+3]));
			if(!ExchangeTime(argv[i+4],&FDAT.dMixStartTime[1]))
				FDAT.dMixStartTime[1] = max(0,atof(argv[i+4]));
			strcpy(FDAT.szMixFile,argv[i+5]);
			i+=5; i2+=6;
		}

		// compressor
		else if(strcmp(argv[i],"-comp")==0){
			FDAT.bComp = true;
			FDAT.dCompRatio = max(1,min(1000,atof(argv[i+1])));
			FDAT.dCompTh=  min(0,max(-192.0,atof(argv[i+2])));
			FDAT.dCompAttack  = max(0,min(5000,atof(argv[i+3])));
			FDAT.dCompRelease  = max(0,min(5000,atof(argv[i+4])));
			FDAT.dwCompRMS  = max(0,min(10000,atoi(argv[i+5])));

			i+=5; i2+=6;
		}	

		// subband noise gate
		else if(!strcmp(argv[i],"-ngate_sb")){
			FDAT.bNgate = true;
			FDAT.dNgateTh = max(-500,min(0,atof(argv[i+1])));
			FDAT.dNgateAttack  = max(0,min(5000,atof(argv[i+2])));
			FDAT.dNgateRelease  = max(0,min(5000,atof(argv[i+3])));
			FDAT.dwNgateRMS  = max(0,min(100,atoi(argv[i+4])));
			i+=4; i2+=5;
		}


		/* obsolete
		// adaptic filter
		else if(strcmp(argv[i],"-adp")==0) {
			FDAT.bADP = true;
			FDAT.dADPlevel = max(0,min(1,atof(argv[i+1])));
			i++; i2+=2;
		}
		
		else if(strcmp(argv[i],"-adp_detail")==0) {
			FDAT.bADP = true;
			FDAT.dADPlevel = max(0,min(1,atof(argv[i+1])));
			FDAT.dwADPleng = max(4,min(MAXFILTERPOINT,atoi(argv[i+2])));
			FDAT.dwADPDelayTime = max(1,min(5000,atoi(argv[i+3])));
			DwADPTrainTime = max(0,min(60,atoi(argv[i+4])));
			FDAT.dADPDb = min(-20,max(-200,atof(argv[i+5])));
			i+=5; i2+=6;
		}

		else if(strcmp(argv[i],"-adpchr")==0) {
			BlADPOutFilter = true;
			i2++;
		}

		// ADP-MDCT filter
		else if(strcmp(argv[i],"-adp_mdct")==0){
			FDAT.bWaveSh = true;
			FDAT.dWaveShLevel = max(-100,min(0,atof(argv[i+1])));
			i++; i2+=2;
		}
*/
			
		// FIR filter
		else if(strcmp(argv[i],"-fir_lpf")==0) {
			FDAT.dwFIRFilter = LPF;
			FDAT.dwFIRCutLow= max(0,atoi(argv[i+1]));
			i++; i2+=2;
		}
		
		else if(strcmp(argv[i],"-fir_hpf")==0) {
			FDAT.dwFIRFilter = HPF;
			FDAT.dwFIRCutLow = max(0,atoi(argv[i+1]));
			i++; i2+=2;
		}
		
		else if(strcmp(argv[i],"-fir_bpf")==0) {
			FDAT.dwFIRFilter = BPF;
			FDAT.dwFIRCutLow = max(0,atoi(argv[i+1]));
			FDAT.dwFIRCutHigh = max(0,atoi(argv[i+2]));
			i+=2; i2+=3;
		}
		
		else if(strcmp(argv[i],"-fir_bsf")==0) {
			FDAT.dwFIRFilter = BSF;
			FDAT.dwFIRCutLow = max(0,atoi(argv[i+1]));
			FDAT.dwFIRCutHigh = max(0,atoi(argv[i+2]));
			i+=2; i2+=3;
		}
		
		else if(strcmp(argv[i],"-firdb")==0) { // loss
			FDAT.dFIRDb = min(-20,max(-200,atof(argv[i+1])));
			i++; i2+=2;
		}
		
		else if(strcmp(argv[i],"-firln")==0) {
			FDAT.dwFIRleng = max(3,min(MAXFILTERPOINT,atoi(argv[i+1])));
			i++; i2+=2;
		}
		
		else if(strcmp(argv[i],"-firchr")==0) {
			BlFIROutFilter = true;
			i2++;
		}

		// 10bands-EQ
		else if(strcmp(argv[i],"-fir_eq")==0){
			FDAT.bFIREQ = true;
			i2++;
			for(i3 = 0; i3 < FDAT.dwEQband; i3++){
				FDAT.dEQLevel[i3] = max(-MAX_EQDB,min(MAX_EQDB,atof(argv[i+1])));
				i++; i2++;
			}
		}

		// 10bands-EQ (detail)
		else if(strcmp(argv[i],"-fir_eq_detail")==0) {
			FDAT.bFIREQ = true;
			FDAT.dwEQleng = max(4,min(MAXFILTERPOINT,atoi(argv[i+1])));
			FDAT.dEQDb = min(-20,max(-200,atof(argv[i+2])));
			FDAT.dEQ_Q = max(0.01,atof(argv[i+3]));
			i+=3; i2+=4;
			for(i3 = 0; i3 < FDAT.dwEQband; i3++){
				FDAT.dEQLevel[i3] = max(-MAX_EQDB,min(MAX_EQDB,atof(argv[i+1])));
				i++; i2++;
			}
		}

		// 3bands-EQ
		else if(strcmp(argv[i],"-fir_eq3")==0){
			FDAT.bFIREQ = true;
			FDAT.dwEQband = 3;
			FDAT.dwEQleng = 255;
			FDAT.dEQDb = -60;
			FDAT.dEQ_Q = 0.643;
			i2++;
			for(i3 = 0; i3 < FDAT.dwEQband; i3++){
				FDAT.dEQLevel[i3] = max(-MAX_EQDB,min(MAX_EQDB,atof(argv[i+1])));
				i++; i2++;
			}
		}

		// 3bands-EQ (detail)
		else if(strcmp(argv[i],"-fir_eq3_detail")==0) {
			FDAT.bFIREQ = true;
			FDAT.dwEQband = 3;
			FDAT.dwEQleng = max(4,min(MAXFILTERPOINT,atoi(argv[i+1])));
			FDAT.dEQDb = min(-20,max(-200,atof(argv[i+2])));
			FDAT.dEQ_Q = max(0.01,atof(argv[i+3]));
			i+=3; i2+=4;
			for(i3 = 0; i3 < FDAT.dwEQband; i3++){
				FDAT.dEQLevel[i3] = max(-MAX_EQDB,min(MAX_EQDB,atof(argv[i+1])));
				i++; i2++;
			}
		}

		// IIR filter

		// LPF
		else if(strcmp(argv[i],"-iir_lpf")==0) {
			FDAT.dwIIRFilter = LPF;
			FDAT.dwIIRCutLow = max(0,atoi(argv[i+1]));
			i++; i2+=2;
		}

		// HPF
		else if(strcmp(argv[i],"-iir_hpf")==0) {
			FDAT.dwIIRFilter = HPF;
			FDAT.dwIIRCutLow = max(0,atoi(argv[i+1]));
			i++; i2+=2;
		}

		// BPF
		else if(strcmp(argv[i],"-iir_bpf")==0) {
			FDAT.dwIIRFilter = BPF;
			FDAT.dwIIRCutLow = max(0,atoi(argv[i+1]));
			FDAT.dwIIRCutHigh = max(0,atoi(argv[i+2]));
			i+=2; i2+=3;
		}

		// de-emphasis
		else if(strcmp(argv[i],"-demp")==0) {
			FDAT.bDemp = true;
			FDAT.dDempDb = 1;
			i2++;
		}

		// emphasis
		else if(strcmp(argv[i],"-emp")==0) {
			FDAT.bDemp = true;
			FDAT.dDempDb = -1;
			i2++;
		}

		// shelving EQ low
		else if(strcmp(argv[i],"-sveq_l")==0) {
			FDAT.bSVEQL = true;
			FDAT.dwSVEQLfreq = max(0,atoi(argv[i+1]));
			FDAT.dSVEQLdb = max(-20,min(20,atof(argv[i+2])));
			i+=2; i2+=3;
		}

		// shelving EQ high
		else if(strcmp(argv[i],"-sveq_h")==0){
			FDAT.bSVEQH = true;
			FDAT.dwSVEQHfreq = max(0,atoi(argv[i+1]));
			FDAT.dSVEQHdb = max(-20,min(20,atof(argv[i+2])));
			i+=2; i2+=3;
		}

		// peaking EQ
		else if(strcmp(argv[i],"-pkeq")==0){
			FDAT.bPKEQ = true;
			FDAT.dwPKfreq = max(0,atoi(argv[i+1]));
			FDAT.dPKdb = max(-20,min(20,atof(argv[i+2])));
			FDAT.dPKQ = max(0,min(100,atof(argv[i+3])));
			i+=3; i2+=4;
		}

		else if(strcmp(argv[i],"-iirchr")==0) {
			BlIIROutFilter = true;
			i2++;
		}

		// phase invert
		else if(strcmp(argv[i],"-phinv")==0) {
			FDAT.bPhaseInv = true;
			i2++;
		}

		// configuration file of names of output files
		else if(strcmp(argv[i],"-outfile")==0){
			BlOutFile = true;
			strcpy(SzOutFile,argv[i+1]);
			i++; i2+=2;
		}
		
#ifdef WIN32
		else if(strcmp(argv[i],"-outfile_fm")==0){
			BlOutFile = true;
			HdOutFileMap = (HANDLE)strtoul(argv[i+1],NULL,10);
			i++; i2+=2;
		}
#endif
		// remove no sound part of in head of data
		else if(strcmp(argv[i],"-headcutting")==0) {
			FDAT.dwNoSndMode = NOSND_HEADCUTONLY;
			FDAT.dNoSndBoundHead = min(0,max(-192,atof(argv[i+1])));
			i++; i2+=2;
		}

		// split at no sound 
		else if(strcmp(argv[i],"-nosound")==0){
			FDAT.dwNoSndMode = NOSND_SEARCH;
			FDAT.dwNoSndTime = max(1,min(10000,atoi(argv[i+1])));
			FDAT.dNoSndBoundHead = min(0,max(-192,atof(argv[i+2])));
			FDAT.dNoSndBound = FDAT.dNoSndBoundHead;

			i+=2; i2+=3;
		}

		else if(strcmp(argv[i],"-nosound_detail")==0) {
			FDAT.dwNoSndMode = max(0,min(2, atoi(argv[i+1]))) +1;
			FDAT.dwNoSndTime = max(1,min(10000,atoi(argv[i+2])));
			FDAT.dNoSndBoundHead = min(0,max(-192,atof(argv[i+3])));
			FDAT.dNoSndBound = min(0,max(-192,atof(argv[i+4])));

			i+=4; i2+=5;
		}

		/* obsolete

		else if(strcmp(argv[i],"-nosound_detail2")==0) {
			FDAT.dwNoSndMode = max(0,min(2, atoi(argv[i+1]))) +1;
			FDAT.dwNoSndTime = max(1,min(10000,atoi(argv[i+2])));
			FDAT.dNoSndBoundHead = min(0,max(-192,atof(argv[i+3])));
			FDAT.dNoSndBound = min(0,max(-192,atof(argv[i+4])));
			FDAT.dwNoSndSuspend		= max(0,min(14400000,atoi(argv[i+5])));
			FDAT.dwNoSndM1toM2	= max(0,min(14400000,atoi(argv[i+6])));
			FDAT.dwNoSndRecStop = max(0,min(14400000,atoi(argv[i+7])));
			i+=7; i2+=8;
		}
		*/

		// FIR filtering befor nosond
		else if(strcmp(argv[i],"-nosound_fir")==0) {
			FDAT.dwNoSndFIRFilter = max(0,atoi(argv[i+1]));
			FDAT.dwNoSndFIRCutLow = max(0,atoi(argv[i+2]));
			FDAT.dwNoSndFIRCutHigh = max(0,atoi(argv[i+3]));
			FDAT.dwNoSndFIRleng = max(4,min(MAXFILTERPOINT,atoi(argv[i+4])));
			FDAT.dNoSndFIRDb = min(-20,max(-200,atof(argv[i+5])));
			i+=5; i2+=6;
		}

		/* obsolete

		// stop recording at no sound
		else if(strcmp(argv[i],"-nosound_stop") ==0) {
			FDAT.dwNoSndMode = NOSND_STOP;
			FDAT.dwNoSndRecStop = max(0,min(14400000,atoi(argv[i+1])));
			FDAT.dNoSndBoundHead = min(0,max(-192,atof(argv[i+2])));
			FDAT.dNoSndBound = FDAT.dNoSndBoundHead;
			i+=2; i2+=3;

		}
		*/

		// split at time
		else if(!strcmp(argv[i],"-split")) {
			FDAT.bSplit = true;
			if(!ExchangeTime(argv[i+1],&FDAT.dSplitTime[0]))
				FDAT.dSplitTime[0] = max(1,min(72000,atof(argv[i+1])));
			i++; i2+=2;
		}

		// split2
		else if(strcmp(argv[i],"-split2")==0) {
			FDAT.bSplit = true;
			i4 = max(0,min(MAX_SPLIT,atoi(argv[i+1])));
			if(i+i4+1 >= (DWORD)argc-2){
				fprintf(stderr,"\n-split2: -split2 n t1 t2 ...\n");
				return false;
			}
			i++; i2+=2;
			for(i3 = 0; i3 < i4; i3++){
				if(!ExchangeTime(argv[i+1],&FDAT.dSplitTime[i3]))
				FDAT.dSplitTime[i3] = max(.1,min(72000,atof(argv[i+1])));
				i++; i2++;
			}
		}

		// split3
		else if(strcmp(argv[i],"-split3")==0) {
			FDAT.bSplit = true;
			i4 = max(0,min(MAX_SPLIT,atoi(argv[i+1])));
			if(i+i4+1 >= (DWORD)argc-2){
				fprintf(stderr,"\n-split3: -split3 n t1 t2 ...\n");
				return false;
			}
			i++; i2+=2;
			dFoo = 0;
			for(i3 = 0; i3 < i4; i3++){
				if(!ExchangeTime(argv[i+1],&FDAT.dSplitTime[i3]))
					FDAT.dSplitTime[i3] = max(.1,min(72000,atof(argv[i+1])));
				dFoo2 = FDAT.dSplitTime[i3];
				FDAT.dSplitTime[i3] -= dFoo;
				dFoo = dFoo2;
				i++; i2++;
			}
		}

		// split at size (kbyte) of output
		else if(!strcmp(argv[i],"-splitkbyte")) {
			FDAT.bSplit = true;
			FDAT.n64SplitByte = max(1,_atoi64(argv[i+1])) * 1024;  
			i++; i2+=2;
		}		

		
		// split at size (byte) of output
		else if(strcmp(argv[i],"-splitbm")==0) {
			FDAT.bSplit = true;
			i4 = max(0,min(MAX_SPLIT,atoi(argv[i+1])));
			if(i+i4+1 >= (DWORD)argc-2){
				fprintf(stderr,"\n-splitbm: -splitbm n b1 b2 ...\n");
				return false;
			}
			i++; i2+=2;
			for(i3 = 0; i3 < i4; i3++){
				FDAT.n64SplitByteMalti[i3] = _atoi64(argv[i+1]);
				i++; i2++;
			}
			FDAT.n64SplitByte = FDAT.n64SplitByteMalti[0]; 
		}

		// add no sound to head and tail
		else if(strcmp(argv[i],"-addspc")==0) {
			DbAddSp[0] = max(0,min(5,atof(argv[i+1])));
			DbAddSp[1] = max(0,min(5,atof(argv[i+2])));
			i+=2; i2+=3;
		}

		// synthesize sine waves
		else if(strcmp(argv[i],"-addsin")==0) {
			FDAT.dwAddSinNum = max(1,min(MAX_SINE_WAVES,atoi(argv[i+1])));
			if(i+FDAT.dwAddSinNum+1 >= (DWORD)argc-2){
				fprintf(stderr,"\n-addsin: -addsin n freq1 db1 ph1 ...\n");
				return false;
			}
			i++; i2+=2;
			for(i3 = 0; i3 < FDAT.dwAddSinNum; i3++){
				FDAT.dwAddSinFreq[i3] = max(0,atoi(argv[i+1]));
				FDAT.dAddSinDb[i3] = min(0,max(-192.0,atof(argv[i+2])));
				FDAT.dwAddSinPhase[i3] = max(0,min(360,atoi(argv[i+3])));
				i+=3; i2+=3;
			}
		}

		// re-sampling
		else if(strcmp(argv[i],"-rsmp")==0) {
			FDAT.bRsmp = true;
			i2++;
		}

		else if(strcmp(argv[i],"-rsmp_detail")==0) {
			FDAT.bRsmp = true;
			FDAT.dwRsmpLn = max(4,atoi(argv[i+1]));
			FDAT.dRsmpDB = min(-20,max(-200,atof(argv[i+2])));
			FDAT.dwRsmpCut= max(0,atoi(argv[i+3]));
			i+=3; i2+=4;
		}

		else if(strcmp(argv[i],"-rsmpchr")==0) {
			BlRsmpOutFilter = true;
			i2++;
		}

		// bit of output file
		else if(strcmp(argv[i],"-bit8")==0) {
			FDAT.dwBitChange = 8;
			i2++;
		}
	
		else if(strcmp(argv[i],"-bit16")==0) {
			FDAT.dwBitChange = 16;
			i2++;
		}

		else if(strcmp(argv[i],"-bit24")==0) {
			FDAT.dwBitChange = 24;
			i2++;
		}

		else if(strcmp(argv[i],"-bit64")==0) {
			FDAT.dwBitChange = 64;
			FDAT.dwBitChangeTag = WAVE_FORMAT_IEEE_FLOAT; 
			i2++;
		}

		else if(strcmp(argv[i],"-bit32float")==0) {
			FDAT.dwBitChange = 32;
			FDAT.dwBitChangeTag = WAVE_FORMAT_IEEE_FLOAT; 
			i2++;
		}

		else if(strcmp(argv[i],"-bit32long")==0) {
			FDAT.dwBitChange = 32;
			i2++;
		}

		// header offset
		else if(strcmp(argv[i],"-headofs")==0){
			BlHeadOffset = true;
			N64FileOffset = _atoi64(argv[i+1]);
			i+=1; i2+=2;
		}

		// specify input format
		else if(strcmp(argv[i],"-format")==0){
			N64FileDataSize = atoi(argv[i+1]);
			InputWaveFmt.nSamplesPerSec = atoi(argv[i+2]);
			InputWaveFmt.nChannels = atoi(argv[i+3]);
			InputWaveFmt.wBitsPerSample = atoi(argv[i+4]);
			InputWaveFmt.wFormatTag = atoi(argv[i+5]);
			if(InputWaveFmt.wBitsPerSample <= 24) InputWaveFmt.wFormatTag = WAVE_FORMAT_PCM;
			SetWaveFmt(&InputWaveFmt,InputWaveFmt.nChannels,InputWaveFmt.nSamplesPerSec,InputWaveFmt.wBitsPerSample,InputWaveFmt.wFormatTag);
			N64FileDataSize = N64FileDataSize*InputWaveFmt.nAvgBytesPerSec;
			i+=5; i2+=6;
		}

		// no wave header output
		else if(strcmp(argv[i],"-nowavehdr")==0)
		{
			BlWaveHdrOut = false;
			i2++;
		}

		// no extra wave header
		else if(strcmp(argv[i],"-noextrahdr")==0) 
		{
			BlExtChunkOfHdr = false;
			i2++;
		}

		// set timestamp to creation time
		else if(strcmp(argv[i],"-fstamp")==0 || strcmp(argv[i],"-tstamp")==0) 
		{
			BlFileStamp = true;
			i2++;
		}

		/* obsolete
		// pipe
		else if(strcmp(argv[i],"-pipeout")==0){
#ifdef USEWIN32API  
			BlCreatePipe = true;
#endif
			strcpy(SzPipeComand,argv[i+1]);
			i++; i2+=2;
		}

		// exec
		else if(strcmp(argv[i],"-exec")==0){
#ifdef USEWIN32API  
			BlExecCom = true;
#endif
			strcpy(SzExecComand,argv[i+1]);
			i++; i2+=2;
		}
		
		// exec_nowin
		else if(strcmp(argv[i],"-exec_nowin")==0){
#ifdef USEWIN32API  
			BlExecCom = true;
#endif
			BlOpenWin = false;
			strcpy(SzExecComand,argv[i+1]);
			i++; i2+=2;
		}
		*/

		// sound device
		else if(strcmp(argv[i],"-device")==0){
			UnWaveDevID = atoi(argv[i+1]);
			i++; i2+=2;
		}

		// text out
		else if(strcmp(argv[i],"-text")==0){
			BlTextOut = true;
			i2++;
		}
			
		else{
			ShowAbout();
			exit(1);
		}
	}
	
	if((unsigned int)argc-i2 != 3) {
		ShowAbout();
		exit(1);
	}

	// input file
	strcpy(SzReadFile,argv[1+i2]);

	// output file
	strcpy(SzOrgWriteFile,argv[2+i2]);
	if(strcmp(SzOrgWriteFile,"NULL") == 0) strcpy(SzOrgWriteFile,"null");

	// init output file names
	if(BlOutFile){
		if(!PrepareOutputFileName(SzOutFile,HdOutFileMap,szErr)){
			fprintf(stderr,szErr);
			return false;
		}
	}

	return true;
}




//-----------------------------------------------------
// initialize parameters
BOOL SetParam(){
	
	DWORD i,dwFoo;
	char szErr[CHR_BUF]; 
	double dFoo;
	
	char szNewName[MAX_PATH];
	char szDriveName[MAX_PATH],szPathName[MAX_PATH],szFileName[MAX_PATH];
	ULONGLONG n64HddSize;

	ULONGLONG n64Foo;

#ifdef USEWIN32API
	HANDLE hTimeFile;
	SYSTEMTIME sysTime;
	FILETIME ftLocal;
#endif

	// get current system time
	GetLocalTime(&SystemTime);

	//----------------------------------------------
	// analysis of input data
	//----------------------------------------------

	// empty file
	if(strcmp(SzReadFile,"nosignal")==0)
	{
		BlNoSignal = true;
	}

	// stdin
	if(strcmp(SzReadFile,"stdin")==0 || strcmp(SzReadFile,"-")==0)
	{
		strcpy(SzReadFile,"stdin");
		BlStdin = true;
	}

	if(BlStdin)
	{
		// no normalize
		DwNormalMode = NORMAL_NOT;
		DbNormalLevel = 0;

/* obsolete
		// no ADP training mode
		DwADPTrainTime = 0; 
*/

		// endless mode is set
		BlEndless = true;

		// no fade out
		DbFadeOutTime = 0;
	}

	fprintf(stderr,"input: ");

	// analyze the format of input file 
	if(!BlHeadOffset && !BlNoSignal)
	{
		if(!GetWaveFormat(SzReadFile,&InputWaveFmt,&N64FileDataSize,&N64FileOffset,szErr))
		{
			fprintf(stderr,"\n");
			fprintf(stderr,szErr);
			return false;
		}
	}
	else
	{ // -format
		if(!IsWaveFormatValid(InputWaveFmt,szErr))
		{
			fprintf(stderr,"\n");
			fprintf(stderr,szErr);
			return false;
		}
	}
	// notice: if input is stdin, file pointer has already moved at N64FileOffset byte.

	// set buffer size
	dwFoo = 1; 
	while((double)dwFoo/InputWaveFmt.nAvgBytesPerSec*1000. < DwBufSize*100.) dwFoo = dwFoo << 1;
	DwBufSize = dwFoo;

	/* obsolete
	dwFoo = 1; 
	while((double)dwFoo/InputWaveFmt.nAvgBytesPerSec*1000. < DwPipeBufSize*100.) dwFoo = dwFoo << 1;
	DwPipeBufSize = dwFoo;
	*/

	fprintf(stderr,"tag = %u, ",InputWaveFmt.wFormatTag);
	fprintf(stderr,"%u ch, ",InputWaveFmt.nChannels);
	fprintf(stderr,"%u Hz, ",InputWaveFmt.nSamplesPerSec);
	fprintf(stderr,"%u bit, ",InputWaveFmt.wBitsPerSample);
	fprintf(stderr,"offset = %d byte\n",N64FileOffset);

	if(N64FileDataSize < 4 * 1024 * 1024)
		fprintf(stderr,"data size = %u byte, ",N64FileDataSize);
	else
	{
		dFoo = (double)N64FileDataSize/1024/1024;
		fprintf(stderr,"data size = %.2lf Mbyte, ",dFoo);
	}
	dFoo = (double)N64FileDataSize/InputWaveFmt.nAvgBytesPerSec;
	fprintf(stderr,"time = %.2lf sec \n",dFoo);

	fprintf(stderr,"input file: ");
	if(BlStdin)	fprintf(stderr,"stdin\n");
	else if(BlNoSignal) fprintf(stderr,"nosignal\n");
	else fprintf(stderr,"%s\n",SzReadFile);

	// if -info option is specified, finish here.
	if(BlFileInfo)
	{
		ShowPeakWave(SzReadFile,DwBufSize,InputWaveFmt,N64FileOffset,N64FileDataSize); 
		return true;
	}

	//----------------------------------------------
	// analysis of output data
	//----------------------------------------------

	// get original file name
	strcpy(SzWriteFile,SzOrgWriteFile);
	if(((FDAT.dwNoSndMode != NOSND_NOT 
		&&  FDAT.dwNoSndMode != NOSND_HEADCUTONLY
		&&  FDAT.dwNoSndMode != NOSND_STOP)
		|| FDAT.bSplit)
		&& strcmp(SzOrgWriteFile,"null")!=0) 
		SetOutputFileName(SzOrgWriteFile,SzWriteFile,0);

#ifdef WIN32
	/* obsolete
	// -pipeout 
	if(BlCreatePipe) BlStdout = true;
	*/

	// wave out
	if(strcmp(SzOrgWriteFile,"waveout")==0)
	{
		BlStdout = true;
		BlWaveOut = true;
	}
#endif

	// stdout
	if(strcmp(SzOrgWriteFile,"stdout")==0)	BlStdout = true;
	
	if(BlStdout 
		/* obsolete
		&& !BlCreatePipe
		*/
		) // stdout
	{
		// don't split file
		FDAT.dwNoSndMode = NOSND_NOT;
		FDAT.bSplit = false;
		
		if(BlWaveOut) // waveout
		{
			// don't output text
			BlTextOut = false;
		}
	}

	// get output format

	SetWaveFmt(&WriteWaveFmt,InputWaveFmt.nChannels,InputWaveFmt.nSamplesPerSec,InputWaveFmt.wBitsPerSample,InputWaveFmt.wFormatTag);

	// LR-mixing,
	if(FDAT.bMixLR)
	{ 
		if(InputWaveFmt.nChannels == 1) FDAT.bMixLR = false;
		else SetWaveFmt(&WriteWaveFmt,1,WriteWaveFmt.nSamplesPerSec,WriteWaveFmt.wBitsPerSample,WriteWaveFmt.wFormatTag);
	}

	// change bit of output file
	if(FDAT.dwBitChange)
		SetWaveFmt(&WriteWaveFmt,WriteWaveFmt.nChannels,WriteWaveFmt.nSamplesPerSec,(WORD)FDAT.dwBitChange,(WORD)FDAT.dwBitChangeTag);

	// re-sampling
	if(FDAT.bRsmp)
	{
		FDAT.dwRsmpInFreq = WriteWaveFmt.nSamplesPerSec;
	
		if(FDAT.dwRsmpInFreq != 48000){
			fprintf(stderr,"\n-rsmp: Input frequency must be 48000 hz.\n");
			return false;
		}
		
		FDAT.dwRsmpOutFreq = 44100;
		SetWaveFmt(&WriteWaveFmt,WriteWaveFmt.nChannels,FDAT.dwRsmpOutFreq,WriteWaveFmt.wBitsPerSample,WriteWaveFmt.wFormatTag);
	}

	fprintf(stderr,"\noutput: ");
	fprintf(stderr,"tag = %u, ",WriteWaveFmt.wFormatTag);
	fprintf(stderr,"%u ch, ",WriteWaveFmt.nChannels);
	fprintf(stderr,"%u Hz, ",WriteWaveFmt.nSamplesPerSec);
	fprintf(stderr,"%u bit\n",WriteWaveFmt.wBitsPerSample);

	if(BlEndless)
	{
		fprintf(stderr,"endless mode\n");

		// no fade out
		DbFadeOutTime = 0;
	}

	if(BlTextOut)
	{
		if(WriteWaveFmt.wBitsPerSample == 64){
			fprintf(stderr,"\nCannot create 64bit text file.\n");
			return false;
		}

		fprintf(stderr,"text out\n");

		// no wave header
		BlWaveHdrOut = false;
	}

	if(!BlWaveHdrOut) fprintf(stderr,"no wave header\n");
	if(!BlExtChunkOfHdr) fprintf(stderr,"no extra wave header\n");

	//----------------------------------------------
	// get offsets and copy sizes of blocks.
	//----------------------------------------------
	
	// -cuttail 
	if(BlCutTail){
		DbEndTime[0] = (double)N64FileDataSize/InputWaveFmt.nAvgBytesPerSec - DbEndTime[0];
		if(DbEndTime[0] < 0) DbEndTime[0] = 0;
	}
	
	if(BlCutFile && DbStartTime[0] > 0 || DbEndTime[0] > 0){
		
		for(i=0;i<DwCopyBlock;i++){

			if(DbStartTime[i] > DbEndTime[i]) DbEndTime[i] = DbStartTime[i];
			
			N64OffsetBlk[i] = InputWaveFmt.nBlockAlign*(LONGLONG)(DbStartTime[i]*(double)InputWaveFmt.nSamplesPerSec);
			N64DataSizeBlk[i] = InputWaveFmt.nBlockAlign*(LONGLONG)((DbEndTime[i]-DbStartTime[i])*(double)InputWaveFmt.nSamplesPerSec);
		}

	}
	else{ 
		// if block size has been specified in -cutb and -cutmb directly, don't set value here.
		if(!BlCutFile) N64DataSizeBlk[0] = N64FileDataSize;
	}
	
	// adjust alignment
	for(i=0;i<DwCopyBlock;i++){
		
		N64OffsetBlk[i] = N64OffsetBlk[i] / (InputWaveFmt.nChannels * (InputWaveFmt.wBitsPerSample/8)) * InputWaveFmt.nBlockAlign;
		
		// add header offset to block offset
		N64OffsetBlk[i] += N64FileOffset; 
		
		N64DataSizeBlk[i] 
			= N64DataSizeBlk[i] / (InputWaveFmt.nChannels * (InputWaveFmt.wBitsPerSample/8)) * InputWaveFmt.nBlockAlign;
	}

	// offset and size of eace block
	for(i=0;i<DwCopyBlock;i++){
		if(N64OffsetBlk[i] > N64FileOffset+N64FileDataSize){
			N64OffsetBlk[i] = N64FileOffset+N64FileDataSize;
		}

		if(N64OffsetBlk[i]+N64DataSizeBlk[i] > N64FileOffset+N64FileDataSize){
			N64DataSizeBlk[i]  = N64FileOffset+N64FileDataSize - N64OffsetBlk[i];
		}
	}
	
	// get total output size
	N64TotalDataSize = 0;	
	for(i=0;i<DwCopyBlock;i++) N64TotalDataSize += N64DataSizeBlk[i];
	if(memcmp(&InputWaveFmt,&WriteWaveFmt,sizeof(WAVEFORMATEX)) != 0){
		N64RealTotalDataSize = 
			WriteWaveFmt.nBlockAlign * 
			(LONGLONG)((double)N64TotalDataSize 
			/ (double)InputWaveFmt.nAvgBytesPerSec 
			* (double)WriteWaveFmt.nSamplesPerSec
			);
	}
	else N64RealTotalDataSize = N64TotalDataSize;
	
	// add no sound part
	if(DbAddSp[0] > 0 || DbAddSp[1] > 0)
	{
		DwAddSp[0] = (DWORD)(DbAddSp[0] * WriteWaveFmt.nAvgBytesPerSec);
		DwAddSp[1] = (DWORD)(DbAddSp[1] * WriteWaveFmt.nAvgBytesPerSec);
	}

	// if data size < 4G, then don't use extra chunk
	n64Foo = N64RealTotalDataSize+DwAddSp[0]+DwAddSp[1];
	if(n64Foo <= 0xFFFFFFFF-44) BlExtChunkOfHdr = false;

	//-----------------------------
	// get file name
	//-----------------------------

	SetCommandStrings(false,SzWriteFile,SzWriteFile,SzWriteFile,SzUserDef[0],SzUserDef[1],SzUserDef[2],SystemTime,WriteWaveFmt,n64Foo);
	strcpy(SzRealWriteFile,SzWriteFile);

#ifdef WIN32
	/* obsolete
	// -exec 
	if(BlExecCom) 
		SetCommandStrings(false,SzExecCmd,SzExecComand,SzRealWriteFile,SzUserDef[0],SzUserDef[1],SzUserDef[2],SystemTime,WriteWaveFmt,n64Foo);
	
	// -pipeout 
	if(BlCreatePipe)
	{ 
		SetCommandStrings(true,SzPipeCmd,SzPipeComand,SzRealWriteFile,SzUserDef[0],SzUserDef[1],SzUserDef[2],SystemTime,WriteWaveFmt,n64Foo);
		strcpy(SzWriteFile,"stdout");
	}
	*/

	// wave out
	if(strcmp(SzOrgWriteFile,"waveout")==0)	strcpy(SzWriteFile,"waveout");
#endif

	// stdout
	if(strcmp(SzOrgWriteFile,"stdout")==0) strcpy(SzWriteFile,"stdout");

	fprintf(stderr,"output file: ");
	if(BlStdout) // stdout 
	{
		/* obsolete
		if(BlCreatePipe) fprintf(stderr,"[pipe] %s\n",SzPipeCmd);
		else */
			if(BlWaveOut)	fprintf(stderr,"[waveout]\n");
		else fprintf(stderr,"[stdout]\n");
	}
	// HDD or NULL
	else fprintf(stderr,"%s\n",SzWriteFile);

	// if name of output file is the same as input file, rename input file to *.bkp.
	if(strcmp(SzReadFile,SzOrgWriteFile)==0)
	{
		_splitpath(SzReadFile,szDriveName,szPathName,szFileName,NULL);	
		wsprintf(szNewName,"%s%s%s.bkp",szDriveName,szPathName,szFileName);
		DeleteFile(szNewName); // delete in advance
		if(MoveFile(SzReadFile,szNewName))
		{
			strcpy(SzReadFile,szNewName);
			fprintf(stderr,"\nChanged input file name to \"%s\".",SzReadFile);
		}
		else
		{
			fprintf(stderr,"\nCould not change input file name to \"%s\"\n",SzReadFile);
			return false;
		}
	}

	fprintf(stderr,"\n");

	//------------------------------------------------------

	//-----------------------------------------------
	// options
	//----------------------------------------------

	fprintf(stderr,"option(s):\n");

	// show timestamp
#ifdef USEWIN32API
	if(BlFileStamp && !BlNoSignal)
	{
		if(!BlStdin)
		{
			hTimeFile = CreateFile(SzReadFile,GENERIC_READ,0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL); 
			if(hTimeFile)
			{
				GetFileTime(hTimeFile,&FtCreationTime,&FtLastAccessTime,&FtLastWriteTime);
				CloseHandle(hTimeFile);
				FileTimeToLocalFileTime(&FtLastWriteTime,&ftLocal);
				FileTimeToSystemTime(&ftLocal,&sysTime);
		
			}
		}
		else if(!BlNoSignal)// get local time
		{
			GetLocalTime(&sysTime);
			SystemTimeToFileTime(&sysTime,&ftLocal);
			LocalFileTimeToFileTime(&ftLocal,&FtCreationTime);
			FtLastAccessTime = FtCreationTime;
			FtLastWriteTime = FtCreationTime;
		}

		fprintf(stderr,"set timestamp to: %d/%d/%d %d:%02d:%02d\n",
					sysTime.wYear,sysTime.wMonth,sysTime.wDay,
					sysTime.wHour,sysTime.wMinute,sysTime.wSecond
					);
	}
#endif	
	if(BlUseSSE2){
		if(CheckSSE2()) fprintf(stderr,"SSE2: supported\n");
	}
	
	CONFIG::status();

	/* obsolete
	if(BlExecCom)fprintf(stderr,"-exec: %s\n",SzExecCmd);	
	*/
	fprintf(stderr,"buffer size: %ldk\n",DwBufSize/1024);
	/* obsolete
	if(BlCreatePipe) fprintf(stderr,"buffer size of pipe: %ldk\n",DwPipeBufSize/1024);
	*/
	if(FDAT.dVolume != 1.0) fprintf(stderr,"volume: %lf\n",FDAT.dVolume);
	if(FDAT.bBalance) fprintf(stderr,"balance: L = %.3lf, R = %.3lf\n",FDAT.dBalance[0],FDAT.dBalance[1]);
/* obsolete
	if(HdWndLkm) fprintf(stderr,"window handle of LockOn: %lu",HdWndLkm);
*/
	if(strlen(SzUserDef[0])) fprintf(stderr,"udef1: %%1 = %s\n",SzUserDef[0]);
	if(strlen(SzUserDef[1])) fprintf(stderr,"udef2: %%2 = %s\n",SzUserDef[1]);
	if(strlen(SzUserDef[2])) fprintf(stderr,"udef3: %%3 = %s\n",SzUserDef[2]);
	if(FDAT.bOffset) fprintf(stderr,"DC offset: left += %lf right += %lf\n",FDAT.dOffset[0],FDAT.dOffset[1]);
	if(FDAT.bPhaseInv) fprintf(stderr,"invert phase:\n");
	if(DbShiftTime != 0) fprintf(stderr,"shift output: %.2lf msec\n",DbShiftTime);
	if(FDAT.bDemp){
		if(FDAT.dDempDb == 1)
			fprintf(stderr,"de-emphasis:\n");
		else
			fprintf(stderr,"emphasis:\n");
	}
	
	// shelving EQ low
	if(FDAT.bSVEQL){
		fprintf(stderr,"shelving EQ: low, ");
		fprintf(stderr,"%d Hz, ",FDAT.dwSVEQLfreq);
		fprintf(stderr,"%.3lf dB\n",FDAT.dSVEQLdb);
	}

	// shelving EQ high
	if(FDAT.bSVEQH){
		fprintf(stderr,"shelving EQ: high, ");
		fprintf(stderr,"%d Hz, ",FDAT.dwSVEQHfreq);
		fprintf(stderr,"%.3lf dB\n",FDAT.dSVEQHdb);
	}

	// peaking EQ high
	if(FDAT.bPKEQ){
		fprintf(stderr,"peaking EQ: ");
		fprintf(stderr,"%d Hz(%d - %d Hz), ",
			FDAT.dwPKfreq,
			FDAT.dwPKfreq-(DWORD)(FDAT.dwPKfreq/2/FDAT.dPKQ),
			FDAT.dwPKfreq+(DWORD)(FDAT.dwPKfreq/2/FDAT.dPKQ)
			);
		fprintf(stderr,"%.3lf dB, Q = %.3lf\n",FDAT.dPKdb,FDAT.dPKQ);
	}

	// auto DC offset 
	if(FDAT.bAutoOffset){ // -autoofs
		FDAT.bOffset = false;  // if -autoofs is specified, then -ofs is canceled.
		FDAT.dOffset[0] = 0;
		FDAT.dOffset[1] = 0;
		fprintf(stderr,"Auto DC offset: %d sec\n",FDAT.dwAutoOffsetTime);
	}

	// -outfile
	if(BlOutFile){
		if(HdOutFileMap) fprintf(stderr,"outfile: file mapping\n");
		else fprintf(stderr,"outfile: %s\n",SzOutFile);
	}

	// add no sound part
	if(DbAddSp[0] > 0 || DbAddSp[1] > 0)
	{
		fprintf(stderr,"add space: %f sec (head), %f sec (tail)\n"
			,DbAddSp[0],DbAddSp[1]);
	}

	// dither
	if(WriteWaveFmt.wBitsPerSample >= 24) FDAT.bDither = false;
	if(FDAT.bDither) fprintf(stderr,"dither: x%6.3lf\n",FDAT.dDitherAmp);

	// LR-mixing
	if(FDAT.bMixLR) fprintf(stderr,"LR-mixing: L:R = %6.3lf : %6.3lf\n",FDAT.dMixLRLevel,1-FDAT.dMixLRLevel);

	// file mixing
	if(FDAT.bMixFile){
		fprintf(stderr,"\n");
		fprintf(stderr,"file mixing: %s\n",FDAT.szMixFile);
		fprintf(stderr,"level = %6.3lf : %6.3f, ",FDAT.dMixLevel[0],FDAT.dMixLevel[1]);
		fprintf(stderr,"time =  %.1f sec : %.1lf sec\n",FDAT.dMixStartTime[0],FDAT.dMixStartTime[1]);
	}

	// compressor
	if(FDAT.bComp){
		fprintf(stderr,"\n");
		fprintf(stderr,"compressor:\nratio = %4.1lf, threshold = %6.3lf dB\n",
			FDAT.dCompRatio,FDAT.dCompTh);
		fprintf(stderr,"attack = %4.0lf msec, release = %4.0lf msec, RMS = %d\n",
			FDAT.dCompAttack,FDAT.dCompRelease,FDAT.dwCompRMS);
	}

	// subband noise gate
	if(FDAT.bNgate){
		fprintf(stderr,"\n");
		fprintf(stderr,"subband noise gate:\n");
		fprintf(stderr,"threshold =  %6.3lf dB\n",FDAT.dNgateTh);
		fprintf(stderr,"attack = %4.0lf msec, release = %4.0lf msec, RMS = %d\n",
			FDAT.dNgateAttack,FDAT.dNgateRelease,FDAT.dwNgateRMS);
	}
	
	/* obsolete
	// MDCT shrinkage filter
	if(FDAT.bWaveSh){
		fprintf(stderr,"\n");
		fprintf(stderr,"MDCT shrinkage filter:\n");
		fprintf(stderr,"level = %lf, ",FDAT.dWaveShLowLevel);
		
		fprintf(stderr,"length =  %d, ",FDAT.dwWaveShleng);
		fprintf(stderr,"threshold = %lf\n",FDAT.dWaveShLevel);
		
		if(FDAT.dwWaveShleng & (FDAT.dwWaveShleng-1)){
			fprintf(stderr,"\nFilter length of MDCT must be 2^m.\n");
			return false;
		}
	}
	*/

/* obsolete
	// ADP
	if(FDAT.bADP){
		fprintf(stderr,"\n");
		fprintf(stderr,"ADP filter:\n");
		fprintf(stderr,"level = %6.3lf, ",FDAT.dADPlevel);
		fprintf(stderr,"length = %d, ",FDAT.dwADPleng);
			fprintf(stderr,"delay = %d(%.2lf msec)\n"
				,(FDAT.dwADPleng-1)/2,(double)(FDAT.dwADPleng-1)/2*1000/InputWaveFmt.nSamplesPerSec);
		fprintf(stderr,"delay time of input = %d msec, ",FDAT.dwADPDelayTime);
		fprintf(stderr,"training time = %d sec, ",DwADPTrainTime);
		fprintf(stderr,"loss = %6.3lf db\n",FDAT.dADPDb);
		
		// filter length must be odd number
		if(FDAT.dwADPleng % 2 == 0){
			fprintf(stderr,"\nLength of ADP filter must be odd number.\n");
			return false;
		}
		
	}
*/

	//----------------
	
	// FIR
	if(FDAT.dwFIRFilter != NO_FILTER){
		fprintf(stderr,"\n");
		fprintf(stderr,"FIR filter: ");
		
		switch(FDAT.dwFIRFilter){
		case LPF: fprintf(stderr,"LPF\n"); break;
		case HPF: fprintf(stderr,"HPF\n"); break;
		case BPF: fprintf(stderr,"BPF\n"); break;
		case BSF: fprintf(stderr,"BSF\n"); break;
		}
		
		fprintf(stderr,"length = %d, ",FDAT.dwFIRleng);
		fprintf(stderr,"delay = %d(%.2lf msec), "
			,(FDAT.dwFIRleng-1)/2,(double)(FDAT.dwFIRleng-1)/2*1000/InputWaveFmt.nSamplesPerSec);
		fprintf(stderr,"loss = %.2lf db, ",FDAT.dFIRDb);
		if(FDAT.dwFIRCutLow > FDAT.dwFIRCutHigh) FDAT.dwFIRCutHigh = FDAT.dwFIRCutLow;
		
		if(FDAT.dwFIRFilter == BPF || FDAT.dwFIRFilter == BSF){
			fprintf(stderr,"%d hz - ",FDAT.dwFIRCutLow);
			fprintf(stderr,"%d hz\n",FDAT.dwFIRCutHigh);
		}
		else fprintf(stderr,"%d hz\n",FDAT.dwFIRCutLow);
	
		
	}
	
	// FIR-EQ
	if(FDAT.bFIREQ){
		double dCntFreq[10] = {31.5, 63, 125, 250, 500, 1000, 2000, 4000, 8000, 16000};
		
		if(FDAT.dwEQband == 3){
			dCntFreq[0] = 125;
			dCntFreq[1] = 1000;
			dCntFreq[2] = 8000;
		}		

		fprintf(stderr,"\n");
		fprintf(stderr,"FIR-EQ:\n");
		fprintf(stderr,"length = %d, ",FDAT.dwEQleng);
		fprintf(stderr,"delay = %d(%.2lf msec), "
			,(FDAT.dwEQleng-1)/2,(double)(FDAT.dwEQleng-1)/2*1000/InputWaveFmt.nSamplesPerSec);
		fprintf(stderr,"loss = %.2lf db, ",FDAT.dEQDb);
		fprintf(stderr,"Q = %.2lf\n",FDAT.dEQ_Q);

		fprintf(stderr,"freq.: ");
		for(i=0;i<FDAT.dwEQband;i++) fprintf(stderr,"%6.0lf ",dCntFreq[i]);
		fprintf(stderr,"\nlevel: ");
		for(i=0;i<FDAT.dwEQband;i++) fprintf(stderr,"%6.0lf ",FDAT.dEQLevel[i]);
		fprintf(stderr,"\n");
	}
	
	// FIR filter length must be odd number
	if(FDAT.dwFIRleng % 2 == 0 || FDAT.dwEQleng % 2 == 0){
		fprintf(stderr,"\nLength of FIR filter must be odd number.\n");
		return false;
	}	

	//-----
	
	// IIR
	if(FDAT.dwIIRFilter != NO_FILTER){

		fprintf(stderr,"\n");
		fprintf(stderr,"IIR filter: ");
		
		switch(FDAT.dwIIRFilter){
		case LPF: fprintf(stderr,"LPF  "); break;
		case HPF: fprintf(stderr,"HPF  "); break;
		case BPF: fprintf(stderr,"BPF  "); break;
		case BSF: fprintf(stderr,"BSF  "); break;
		}
		
		if(FDAT.dwIIRCutLow > FDAT.dwIIRCutHigh) FDAT.dwIIRCutHigh = FDAT.dwIIRCutLow;

		if(FDAT.dwIIRFilter == BPF || FDAT.dwIIRFilter == BSF){
			fprintf(stderr,"%d hz - ",FDAT.dwIIRCutLow);
			fprintf(stderr,"%d hz\n",FDAT.dwIIRCutHigh);
		}
		else fprintf(stderr,"%d hz\n",FDAT.dwIIRCutLow);
	}



	// resampling
	if(FDAT.bRsmp){
		fprintf(stderr,"\n");
		fprintf(stderr,"re-sampling: ");
		fprintf(stderr,"%d hz -> %d hz\n",FDAT.dwRsmpInFreq,FDAT.dwRsmpOutFreq);
		fprintf(stderr,"length = %d, ",FDAT.dwRsmpLn);
		fprintf(stderr,"loss = %6.3lf db, ",FDAT.dRsmpDB);
		fprintf(stderr,"cut off = %d hz, ",FDAT.dwRsmpCut);
		
		if(FDAT.dwRsmpInFreq == 48000 && FDAT.dwRsmpOutFreq == 44100){  
			FDAT.dwRsmpUp = 147;   
			FDAT.dwRsmpDown = 160;
		}

		fprintf(stderr,"up to %d hz\n",FDAT.dwRsmpInFreq*FDAT.dwRsmpUp);
		
	}


	// synthesize sine waves
	if(FDAT.dwAddSinNum){
		fprintf(stderr,"\n");

		fprintf(stderr,"synthesize sine waves:\n");
		for(i=0;i<FDAT.dwAddSinNum;i++){
			fprintf(stderr,"[%2d] %d hz, %lf db, %d degrees\n",i,
				FDAT.dwAddSinFreq[i],FDAT.dAddSinDb[i],FDAT.dwAddSinPhase[i]);
		}
	}

	// split at no sound, or  delete no sound part in head
	if(FDAT.dwNoSndMode != NOSND_NOT){
		
		fprintf(stderr,"\n");
		
		if(FDAT.dwNoSndMode == NOSND_HEADCUTONLY){
			fprintf(stderr,"headcutting: level = %6.2lf dB\n",FDAT.dNoSndBoundHead);
		}
		/* obsolete
		else if(FDAT.dwNoSndMode == NOSND_STOP && HdWndLkm){
			fprintf(stderr,"nosound_stop: level = %6.2lf dB, stop recording = %d msec\n",
				FDAT.dNoSndBoundHead,FDAT.dwNoSndRecStop);
		}
		*/
		else{
			fprintf(stderr,"nosound: mode %d\n",FDAT.dwNoSndMode-1);
			fprintf(stderr,"%d msec, sound level = %6.2lf dB, nosound leve = %6.2lf dB\n",
				FDAT.dwNoSndTime,FDAT.dNoSndBoundHead,FDAT.dNoSndBound);
			fprintf(stderr,"suspend time = %d msec, ",FDAT.dwNoSndSuspend);
			fprintf(stderr,"mode 1 to 2 = %d msec\n",FDAT.dwNoSndM1toM2);
			fprintf(stderr,"stop recording = %d msec\n",FDAT.dwNoSndRecStop);
		}
		
		// no normalizer
		DwNormalMode = NORMAL_NOT;
		DbNormalLevel = 0;
		
		// no fade out
		DbFadeOutTime = 0;
		
		// FIR filter befor nosound
		if(FDAT.dwNoSndFIRFilter != NO_FILTER){

			fprintf(stderr,"FIR filter before nosound: ");
			
			switch(FDAT.dwNoSndFIRFilter){
			case LPF: fprintf(stderr,"LPF\n"); break;
			case HPF: fprintf(stderr,"HPF\n"); break;
			case BPF: fprintf(stderr,"BPF\n"); break;
			case BSF: fprintf(stderr,"BSF\n"); break;
			}
			
			fprintf(stderr,"length = %d, ",FDAT.dwNoSndFIRleng);
			fprintf(stderr,"delay = %d(%.2lf msec), "
				,(FDAT.dwNoSndFIRleng-1)/2,(double)(FDAT.dwNoSndFIRleng-1)/2*1000/InputWaveFmt.nSamplesPerSec);
			fprintf(stderr,"loss = %.2lf db, ",FDAT.dNoSndFIRDb);
			if(FDAT.dwNoSndFIRCutLow > FDAT.dwNoSndFIRCutHigh) 
				FDAT.dwNoSndFIRCutHigh = FDAT.dwNoSndFIRCutLow;
			
			if(FDAT.dwNoSndFIRFilter == BPF || FDAT.dwNoSndFIRFilter == BSF){
				fprintf(stderr,"%d hz - ",FDAT.dwNoSndFIRCutLow);
				fprintf(stderr,"%d hz\n",FDAT.dwNoSndFIRCutHigh);
			}
			else fprintf(stderr,"%d hz\n",FDAT.dwNoSndFIRCutLow);
		}
		
	}
	else FDAT.dwNoSndFIRFilter = NO_FILTER;  // don't use FIR filter

	//--------------

	// split file at specified time or byte
	if(FDAT.bSplit){
		
		if(FDAT.dSplitTime[DwCurSplitNo]){  // -split1,2,3
			FDAT.n64SplitByte = (LONGLONG)((double)InputWaveFmt.nAvgBytesPerSec*FDAT.dSplitTime[DwCurSplitNo]);
		} 

		// no nomilizer
		DwNormalMode = NORMAL_NOT;
		DbNormalLevel = 0;
		
		// no fade out
		DbFadeOutTime = 0;
	}

	//fade in/out
	FDAT.dwFadeIn = 0;
	FDAT.dwFadeOut = 0;
	if(DbFadeInTime > 0 || DbFadeOutTime > 0){

		fprintf(stderr,"\n");
		
		if(DbFadeInTime > 0){
			FDAT.dwFadeIn  = InputWaveFmt.nBlockAlign*(DWORD)(DbFadeInTime*(double)WriteWaveFmt.nSamplesPerSec);
			fprintf(stderr,"fade in: %lf sec\n",DbFadeInTime);
		}
		if(DbFadeOutTime > 0){
			FDAT.dwFadeOut = InputWaveFmt.nBlockAlign*(DWORD)(DbFadeOutTime*(double)WriteWaveFmt.nSamplesPerSec);
			fprintf(stderr,"fade out: %lf sec\n",DbFadeOutTime);
		}
		
		if(FDAT.dwFadeIn > N64TotalDataSize) FDAT.dwFadeIn = (DWORD)N64TotalDataSize;
		if(FDAT.dwFadeOut > N64TotalDataSize) FDAT.dwFadeOut = (DWORD)N64TotalDataSize;
	}

#ifdef USEWIN32API
	// check space of HDD
	if(!BlStdout && !BlNoChkHdd && strcmp(SzWriteFile,"null")!=0){

		GetFreeHddSpace64(&n64HddSize,SzWriteFile);

		n64Foo = (ULONGLONG)((double)(N64RealTotalDataSize+DwAddSp[0]+DwAddSp[1])
			*((double)WriteWaveFmt.nAvgBytesPerSec/InputWaveFmt.nAvgBytesPerSec));

		if(n64HddSize < WAVEHDRSIZE(BlExtChunkOfHdr)+n64Foo){
			fprintf(stderr,"\nSpace of hdd (=%d Mbyte) is not enough.\n",n64HddSize/1024/1024);
			return false;
		}
	}
#endif

	return true;
}



//-------------------------------------------------------
// in case of Ctrl+C 
#ifdef USEWIN32API
BOOL WINAPI ExitCtrlC(DWORD dwCtrlType){
#else
void ExitCtrlC(int nRet){
#endif
	
#ifdef USEWIN32API
	// close wave device
	CloseWaveDevice(true);
	/* obsolete
	StopChildStop(hProcessInfo,BlCreatePipe);
	*/
#endif	
	
	
#ifdef USEWIN32API
	fprintf(stderr,"\nstop (waveflt)\n");
	return false;
#else
	fprintf(stderr,"\n\nstop.(%d)(waveflt)\n",nRet);
	exit(nRet);
#endif
}
	


//-------------------------------------------------------
// filter
BOOL Filter(){
	
	double* lpFilterBuf[2] = {NULL,NULL}; 	// LR-buffer for filter(double)
	DWORD dwFilterBufSize2; // points, size of buffer 
	DWORD dwFilterBufChn; // channels of buffer
	BYTE* lpBuffer = NULL; // buffer for fread()
	BYTE* lpWriteBuffer = NULL; // buffer for fwrite()
	DWORD dwOutBufSize; // buffer size of lpWriteBuffer

	LONGLONG n64TotalOutSize; // byte, total output data size
	LONGLONG n64OutSize; // byte, output data size in each block
	DWORD dwPointsInBuf = 0; // points of data in 'lpFilterBuf'

	LONGLONG n64InputSize; 	// byte, input data size in each block

	// if re-sampling is not executed,  n64RealTotalOutSize = n64TotalOutSize.
	LONGLONG n64RealTotalOutSize; // byte, total 'real' output data size. 
	LONGLONG n64RealOutSize; // byte, 'real' output data size in each block
	DWORD dwRealPointsInBuf = 0; // 'real' points of data in 'lpFilterBuf'

	LONGLONG n64BlockDataSize; // byte, copy size of each block
	DWORD dwSetSize; // byte, size of data set to 'lpBuffer' when reading
	DWORD dwRemainByte; //byte, unused data size remained in buffer.

	// hadle of files
#ifdef USEWIN32API
	HANDLE hdReadFile = NULL;
	HANDLE hdWriteFile = NULL;
	LARGE_INTEGER LI; // for SetFilePointer()
#else
	FILE* hdReadFile = NULL;
	FILE* hdWriteFile = NULL;
#endif

	BOOL bChangeFile = false; // if true, change output file
	/* obsolete
	DWORD dwADPTrainDataSize = 0; // byte, training size in ADP training mode
	*/
	double dNormalGain[2] = {0,0}; // gain for normalizer
	DWORD dwCurrentNormalMode; // current mode of normalizer
	LONGLONG n64PointerStdin;  // byte, current file pointer of stdin
	DWORD dwCutHeadOutPoints; // points for shiftting(-shift ). unless dwCutHeadOutPoints >0 , cut the head of output
	DWORD dwAddTailOutPoints; // points for shiftting([-shift). add space to the tail of output
	
	// ,and others
	DWORD dwBlockNo,dwNormalLoop,dwNormalExec;
	DWORD i,dwFoo;
	BOOL bReturn = false; // return value
	DWORD dwReadByte;  // read size of data specified at fread()
	DWORD dwWriteByte;  // write size of data returned after fwrite()
	DWORD dwSeed; // for seed of rand
	double dFoo;
	LONGLONG n64Foo;
	char szErr[CHR_BUF];


	//-------------------------------------------------------------------

	//--------------------------
	// initializing
	
	dwSeed = timeGetTime();

	/* obsolete
#ifdef USEWIN32API
	hProcessInfo.hProcess = NULL;
#endif	
	*/

	// in case of Ctrl+C 
#ifdef USEWIN32API
	SetConsoleCtrlHandler(ExitCtrlC,TRUE);
#else
	signal(SIGINT,ExitCtrlC);
#endif

	// buffer size of filter
	dwFilterBufSize2 = DwBufSize/InputWaveFmt.nBlockAlign;
	i = 1; while(i<dwFilterBufSize2) i = i << 1; // set align = 2^x  (for 24 bit file)
	dwFilterBufSize2 = i;

	// number of channels of output file
	dwFilterBufChn = WriteWaveFmt.nChannels;
	
	if(!InitFilters(&FDAT,
		dwFilterBufSize2,
		InputWaveFmt,
		WriteWaveFmt,
		szErr)){
		fprintf(stderr,szErr);
		goto L_ERR;
	}

	// write characteristics of FIR filter, then exit
	if(BlFIROutFilter){

		if(FDAT.dwNoSndFIRFilter != NO_FILTER) dwFoo = ID_FIR_NOSND;
		else if(FDAT.dwFIRFilter != NO_FILTER) dwFoo = ID_FIR_NORMAL;
		else if(FDAT.bFIREQ) dwFoo = ID_FIR_EQ;

		OutputFIRFilterChr(SzCurDir,InputWaveFmt,dwFoo);
		bReturn = true;
		goto L_ERR;
	}

	// write characteristics of IIR filter, then exit
	if(BlIIROutFilter){
		
		if(FDAT.dwIIRFilter != NO_FILTER) dwFoo = ID_IIR_NORMAL;
		else if(FDAT.bDemp) dwFoo = ID_IIR_DEMP;
		else if(FDAT.bSVEQL) dwFoo = ID_IIR_SVEQL;
		else if(FDAT.bSVEQH) dwFoo = ID_IIR_SVEQH;
		else if(FDAT.bPKEQ) dwFoo = ID_IIR_PKEQ;

		printIIRcoef(dwFoo);
		OutputIIRFilterChr(SzCurDir,InputWaveFmt,dwFoo);
		bReturn = true;
		goto L_ERR;
	}
	

	if(BlRsmpOutFilter){
		OutputRsmpChr(SzCurDir,InputWaveFmt);
		bReturn = true;
		goto L_ERR;
	}

	// create buffer for read and write file
	lpBuffer = (BYTE*)malloc(DwBufSize+1024); 
	dwOutBufSize = DwBufSize;
	if(FDAT.bRsmp) dwOutBufSize *= 2; // size *= 2 when re-sampling
	if(WriteWaveFmt.wBitsPerSample > InputWaveFmt.wBitsPerSample){

		if(InputWaveFmt.wBitsPerSample == 8){
			if(WriteWaveFmt.wBitsPerSample == 24) dwOutBufSize *= 4;
			else dwOutBufSize *= WriteWaveFmt.wBitsPerSample/InputWaveFmt.wBitsPerSample;
		}
		if(InputWaveFmt.wBitsPerSample == 16){
			if(WriteWaveFmt.wBitsPerSample == 24) dwOutBufSize *= 2;
			else dwOutBufSize *= WriteWaveFmt.wBitsPerSample/InputWaveFmt.wBitsPerSample;
		}
		else if(InputWaveFmt.wBitsPerSample == 24){
			if(WriteWaveFmt.wBitsPerSample == 32) dwOutBufSize *= 2;
			else dwOutBufSize *= 4;
		}
		else dwOutBufSize *= WriteWaveFmt.wBitsPerSample/InputWaveFmt.wBitsPerSample;

	}
	lpWriteBuffer = (BYTE*)malloc(dwOutBufSize+1024); 

	// create buffer for filter
	dwFoo = dwFilterBufSize2;
	if(FDAT.bRsmp) dwFoo *= 2; // size *= 2 when re-sampling
	for(i=0;i<InputWaveFmt.nChannels;i++) lpFilterBuf[i] = (double*)malloc(sizeof(double)*dwFoo+1024); 

	// open input file
	if(!OpenReadFile(&hdReadFile,SzReadFile,BlStdin)){
		goto L_ERR;
	}
	
	// open output file
	if(!OpenWriteFile(&hdWriteFile,SzWriteFile,
		/* obsolete
#ifdef USEWIN32API
		&hProcessInfo,
#endif   
		*/
		BlStdout
		/* obsolete
		,BlCreatePipe,SzPipeCmd,DwPipeBufSize
		*/
		)){
		goto L_ERR;
	}

#ifdef USEWIN32API
	// open wave device
	if(BlWaveOut){
		if(!OpenWaveDevice(UnWaveDevID,WriteWaveFmt,dwOutBufSize+1024)){
			goto L_ERR;
		}
	}
#endif

	// if input is stdin and -headofs is specified, move file pointer of stdin here in advance.
	if(BlStdin && BlHeadOffset)	SeekStdin(lpBuffer,DwBufSize,N64FileOffset,0);
	n64PointerStdin = N64FileOffset;

	if(BlWaveHdrOut){

		// if output file is stdout and not waveout, write wave header here
		if(!BlWaveOut){ // not waveout
			
			if(BlStdout)
			{
				n64Foo = N64RealTotalDataSize+DwAddSp[0]+DwAddSp[1];
				WriteWaveHeader(hdWriteFile,&WriteWaveFmt,n64Foo,BlExtChunkOfHdr);
			}
			else if(hdWriteFile != NULL){
				// move file pointer of output file
#ifdef USEWIN32API
				LI.QuadPart = WAVEHDRSIZE(BlExtChunkOfHdr);
				SetFilePointer(hdWriteFile,LI.LowPart, &LI.HighPart,FILE_BEGIN);
#else
				fseek(hdWriteFile,WAVEHDRSIZE(BlExtChunkOfHdr),SEEK_SET);
#endif
			}
			
		}
	}

	// add space to head
	AddSpase(hdWriteFile,DwAddSp[0],WriteWaveFmt);

	//-------------------------------------------------------------------
	//-------------------------------------------------------------------
	//-------------------------------------------------------------------


	//-------------------
	// initialize normalizer
	if(DwNormalMode != NORMAL_NOT){

		fprintf(stderr,"\n");
		
		if(DwNormalMode == NORMAL_PEAKBYDB)
			fprintf(stderr,"normalizer (peak): level = %5.3lf dB\n",DbNormalLevel);
		if(DwNormalMode == NORMAL_AVG)
			fprintf(stderr,"normalizer (average): level = %5.3lf dB\n",DbNormalLevel);
		if(DwNormalMode == NORMAL_RMS)
			fprintf(stderr,"normalizer (RMS): level = %5.3lf dB\n",DbNormalLevel);
		
		dwNormalExec = 1;

	}
	else dwNormalExec = 0;

	dNormalGain[0] = dNormalGain[1] = 0;
	dwCurrentNormalMode = DwNormalMode;

	//--------------------------
	// loop 2 times when normalizer is executed.
	for(dwNormalLoop = 0; dwNormalLoop <= dwNormalExec; dwNormalLoop++)
	{
		
		//-----------------------
		// initialize 
		//-----------------------
		
//		init_genrand(dwSeed);
		srand(dwSeed);
		
		n64RealTotalOutSize = 0; 
		n64TotalOutSize = 0;

		ClearAllFilters();

/* obsolete
		// set ADP training mode
		if(FDAT.bADP && DwADPTrainTime) dwADPTrainDataSize = DwADPTrainTime*InputWaveFmt.nAvgBytesPerSec;
		else dwADPTrainDataSize = 0;
*/

		dwFoo = (DWORD)(DbShiftTime * WriteWaveFmt.nSamplesPerSec / 1000);
		dwCutHeadOutPoints = dwFoo;
		dwAddTailOutPoints = dwFoo;

		//-----------------------------
		// main loop
		//-----------------------------
		for(dwBlockNo = 0; dwBlockNo < DwCopyBlock ;dwBlockNo++){
			
			dwSetSize = 0;
			dwRemainByte = 0;
			n64OutSize = 0;
			dwPointsInBuf = 0;
			n64InputSize = 0;
			n64RealOutSize = 0;
			bChangeFile = false;
			n64BlockDataSize = N64DataSizeBlk[dwBlockNo];
			
			if(
				/* obsolete
				!dwADPTrainDataSize && 
				*/
				(dwCurrentNormalMode == NORMAL_NOT || dwCurrentNormalMode == NORMAL_EXEC)){
				fprintf(stderr,"\n----------------\n");
				fprintf(stderr,"[block %d : ",dwBlockNo);

				dFoo = (double)N64OffsetBlk[dwBlockNo]/1024/1024;
				fprintf(stderr,"offset = %.2lf M",dFoo);
				
				if(!(BlEndless && !BlCutFile)){ 
					dFoo = (double)N64OffsetBlk[dwBlockNo]/InputWaveFmt.nAvgBytesPerSec;
					fprintf(stderr,"(%.2lf sec), ",dFoo);
					
					dFoo = (double)n64BlockDataSize/1024/1024;
					fprintf(stderr,"size = %.2lf M",dFoo);
					
					dFoo = (double)n64BlockDataSize/InputWaveFmt.nAvgBytesPerSec;
					fprintf(stderr,"(%.2lf sec)",dFoo);
				}
				else fprintf(stderr," (endless mode)");
				fprintf(stderr,"]\n");

				fprintf(stderr,"output: ");
				if(BlStdout){
					/* obsolete
					if(BlCreatePipe) fprintf(stderr,"[pipe] %s\n",SzPipeCmd);
					else 
					*/
						if(BlWaveOut) fprintf(stderr,"[waveout]\n");
					else fprintf(stderr,"[stdout]\n");
				}
				else fprintf(stderr,"%s\n",SzWriteFile);
	
				if(FDAT.bSplit)
					fprintf(stderr,"split file: [%3d] %.2lf sec (%lu M)\n",
					DwCurSplitNo,
					(double)FDAT.n64SplitByte/InputWaveFmt.nAvgBytesPerSec,
					FDAT.n64SplitByte/1024/1024);
			}
			/* obsolete
			else if(dwADPTrainDataSize && 
				(dwCurrentNormalMode == NORMAL_NOT || dwCurrentNormalMode == NORMAL_EXEC)) 
				fprintf(stderr,"\nNow, trainig the coefficients of ADP filter ...\n");
				*/

			
			// move file pointer of input file
			if(!BlStdin)  // hdd
			{
#ifdef USEWIN32API
				LI.QuadPart = N64OffsetBlk[dwBlockNo];
				SetFilePointer(hdReadFile,LI.LowPart, &LI.HighPart,FILE_BEGIN);
#else
				fseek(hdReadFile,N64OffsetBlk[dwBlockNo],SEEK_SET);
#endif
			}
			else // stdin
			{
				n64PointerStdin += SeekStdin(lpBuffer,DwBufSize,
					N64OffsetBlk[dwBlockNo],
					n64PointerStdin
					);

			}

			//-----------------------------
			// filtering in each block
			while(1){
				
				//----------------------------------------------------------
				// input
				//----------------------------------------------------------
				
				// caluculate read size of data from input file
				if(n64BlockDataSize > (n64InputSize-dwRemainByte) + DwBufSize) dwReadByte = DwBufSize;
				else dwReadByte = (DWORD)(n64BlockDataSize - (n64InputSize-dwRemainByte));

				// in case of endless mode, 
				if(BlEndless && !BlCutFile) dwReadByte = DwBufSize;  
				
				// align the boundary if bit of input file is 24
				if(InputWaveFmt.wBitsPerSample == 24) 
					dwReadByte =  dwReadByte / (InputWaveFmt.nChannels * (InputWaveFmt.wBitsPerSample/8)) * InputWaveFmt.nBlockAlign;

				// when current block is over, 
				if(dwReadByte == 0){

					// shiftting.  (at the last block, shiftting should be done.)
					if(dwBlockNo == DwCopyBlock-1 && dwAddTailOutPoints){

						// shiftting. add space to the tail of buffer
						if(InputWaveFmt.wBitsPerSample == 8) memset(lpBuffer,0x80,DwBufSize);
						else memset(lpBuffer,0,DwBufSize);
							
						if(InputWaveFmt.wBitsPerSample == 24)
							dwFoo =  DwBufSize / (InputWaveFmt.nChannels * (InputWaveFmt.wBitsPerSample/8)) * InputWaveFmt.nBlockAlign;
						
						if(dwAddTailOutPoints > dwFoo){
							dwSetSize = dwFoo;
							dwAddTailOutPoints -= dwFoo/InputWaveFmt.nBlockAlign;
						}
						else{
							dwSetSize = dwAddTailOutPoints*InputWaveFmt.nBlockAlign;
							dwAddTailOutPoints = 0;
						}
					}
					else goto L_EXITBLOCK; // exit loop

				}
				else // read the sound data from input stream
				{
				
					// from input file
					if(!BlNoSignal){
						
						if(dwRemainByte){
							
							// move remained data to head
							memmove(lpBuffer,
								lpBuffer+dwPointsInBuf*InputWaveFmt.nBlockAlign,
								dwRemainByte);
							
							// fill buffer from input file
							if(dwReadByte > dwRemainByte){
								ReadData(hdReadFile,lpBuffer+dwRemainByte,dwReadByte-dwRemainByte,&dwSetSize);
								n64PointerStdin += dwSetSize;
								n64InputSize += dwSetSize;
								
								dwSetSize += dwRemainByte;
							}
							else dwSetSize = dwReadByte;
							
							dwRemainByte = 0;
						}
						else{ // read data from input file
							ReadData(hdReadFile,lpBuffer,dwReadByte,&dwSetSize);
							n64PointerStdin += dwSetSize;
							n64InputSize += dwSetSize;
						}
						
					}
					else // if input file's name is 'nosignal'
					{
						if(InputWaveFmt.wBitsPerSample == 8) memset(lpBuffer,0x80,dwReadByte);
						else memset(lpBuffer,0,dwReadByte);
						dwSetSize = dwReadByte;
						n64InputSize += dwSetSize;
					}

					// if EOF, then exit loop
					if(dwSetSize == 0) goto L_EXITBLOCK;
					
				}

				// set data to buffer(double type)
				CopyBufferBtoD(lpBuffer,dwSetSize,lpFilterBuf,&dwPointsInBuf,InputWaveFmt);

				//-----------------------------------------------
				// filtering
				//-----------------------------------------------

				WFLT_FILTER(
					&FDAT,  
					
					lpFilterBuf,  // buffer
					&dwPointsInBuf, // points
					&dwRealPointsInBuf, // output points
					
					&bChangeFile,
					/* obsolete
					dwADPTrainDataSize,
					*/
					dwCurrentNormalMode,
					dNormalGain,
					DwCurSplitNo,

					/* obsolete
					HdWndLkm,
					*/
					
					n64TotalOutSize+n64OutSize,N64TotalDataSize,
					InputWaveFmt,
					WriteWaveFmt);

				//------------------------------------------------
				// output 
				//------------------------------------------------

				/* obsolete
				// In ADP training mode, don't output data
				if(dwADPTrainDataSize == 0){
				*/
					
					// if normalizer is searching the peak, don't output data
					if(dwCurrentNormalMode == NORMAL_NOT || dwCurrentNormalMode == NORMAL_EXEC)
					{
						
						// shiftting. (cut some data from the head of output.)
						if(dwCutHeadOutPoints){
							if(dwCutHeadOutPoints > dwRealPointsInBuf){
								dwCutHeadOutPoints -= dwRealPointsInBuf;
								dwRealPointsInBuf = 0;
								dwPointsInBuf = 0;
							}
							else // dwCutHeadOutPoints <= dwRealPointsInBuf
							{
								dwRealPointsInBuf -= dwCutHeadOutPoints;
								dwPointsInBuf -= (DWORD)((double)dwCutHeadOutPoints/WriteWaveFmt.nSamplesPerSec*InputWaveFmt.nSamplesPerSec);
								for(i=0;i<WriteWaveFmt.nChannels;i++)
									memmove(lpFilterBuf[i],lpFilterBuf[i]+dwCutHeadOutPoints,sizeof(double)*dwRealPointsInBuf);
								dwCutHeadOutPoints = 0; // not do shiftting anymore
							}
						}
						
						if(dwRealPointsInBuf){
							
							if(BlTextOut){ // text out
								WriteTextData(hdWriteFile,lpFilterBuf,dwRealPointsInBuf,WriteWaveFmt);
							}
							else
							{
								
								CopyBufferDtoB(lpWriteBuffer,lpFilterBuf,
									dwRealPointsInBuf,
									WriteWaveFmt,
									FDAT.bDither,
									
									&DwSaturate
									);
								
								// write to the output file
								if(!BlWaveOut){
									
#ifdef USEWIN32API
									if(!WriteData(hdWriteFile,lpWriteBuffer,dwRealPointsInBuf*WriteWaveFmt.nBlockAlign,&dwWriteByte
										/* obsolete
										,BlCreatePipe,hProcessInfo
										*/
										)){
										// if fail, then write header and exit.
										if(BlWaveHdrOut && !BlStdout) 
											WriteWaveHeader(hdWriteFile,&WriteWaveFmt,n64RealTotalOutSize+DwAddSp[0],BlExtChunkOfHdr);
										goto L_ERR;
									}
#else
									WriteData(hdWriteFile,lpWriteBuffer,dwRealPointsInBuf*WriteWaveFmt.nBlockAlign,&dwWriteByte,BlCreatePipe);
#endif
									
									
								}
#ifdef USEWIN32API
								// waveout
								else 
								{
									dwWriteByte = dwRealPointsInBuf*WriteWaveFmt.nBlockAlign;
									PlayWave(lpWriteBuffer,dwWriteByte);
								}
#endif
							}
							
						}
						
						// renew the size of output
						n64RealOutSize += dwRealPointsInBuf*WriteWaveFmt.nBlockAlign;
						n64OutSize += dwPointsInBuf * InputWaveFmt.nBlockAlign;
						
						// show the current status
						if(BlVerbose){
							double dPeak[2];
							GET_PEAK(dPeak);
							dFoo = max(dPeak[0],dPeak[1]);
							dFoo = 20*log10(fabs(dFoo));
							
							ShowStatus(WriteWaveFmt,SzRealWriteFile,
								/* obsolte
								BlCreatePipe,
								*/
								n64RealTotalOutSize+n64RealOutSize,
								N64RealTotalDataSize,
								dFoo,
								/* obsolete
								false,
								*/
								false,
								(BlEndless && !BlCutFile)
								);
						}
						
						//--------------------------------------
						// change output file
						if(bChangeFile)
						{
							n64RealTotalOutSize += n64RealOutSize;
							n64TotalOutSize += n64OutSize;
							dwRemainByte = dwSetSize - dwPointsInBuf *InputWaveFmt.nBlockAlign;
							
							// add space to tail
							AddSpase(hdWriteFile,DwAddSp[1],WriteWaveFmt);
							
							// write wave header
							if(BlWaveHdrOut && !BlStdout) WriteWaveHeader(hdWriteFile,&WriteWaveFmt,n64RealTotalOutSize+DwAddSp[0]+DwAddSp[1],BlExtChunkOfHdr);
							
							// close output file handle
#ifdef USEWIN32API
							if(hdWriteFile != NULL) CloseHandle(hdWriteFile);
							hdWriteFile = NULL;
#else
							if(hdWriteFile != NULL) fclose(hdWriteFile);
							hdWriteFile = NULL;
#endif
							
#ifdef USEWIN32API
							/* obsolete
							// wait until child process is terminated
							if(!StopChildStop(hProcessInfo,BlCreatePipe)){
								hProcessInfo.hProcess = NULL;
								goto L_ERR;
							}
							hProcessInfo.hProcess = NULL;
							*/
							
							// set timestamp
							if(BlFileStamp && !BlNoSignal)
							{
								HANDLE hTimeFile;
								hTimeFile = CreateFile(SzRealWriteFile,GENERIC_WRITE,0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL); 
								if(hTimeFile)
								{
									SetFileTime(hTimeFile,&FtCreationTime,&FtLastAccessTime,&FtLastWriteTime);
									CloseHandle(hTimeFile);
									Sleep(50);
								}
							}
#endif	

							/* obsolete
#ifdef WIN32
							// execute command
							if(BlExecCom && strcmp(SzWriteFile,"null")!=0){
								fprintf(stderr,"\nexecute: %s\n",SzExecCmd);
								if(!ExecCommand(SzExecCmd,NULL,FALSE,BlOpenWin)){
									fprintf(stderr,"Could not execute %s.\n",SzExecCmd);
								}
							}							
#endif
							*/

							//---------------------------------------
							
							// open new file
							
							// initialize
							if(!BlEndless)
							{
								N64TotalDataSize -= n64TotalOutSize;
								N64RealTotalDataSize -= n64RealTotalOutSize;

								// if data size < 4G, then don't use extra chunk
								n64Foo = N64RealTotalDataSize+DwAddSp[0]+DwAddSp[1];
								if(n64Foo <= 0xFFFFFFFF-44) BlExtChunkOfHdr = false;
							}

							n64OutSize = 0;
							n64TotalOutSize = 0;
							n64RealOutSize = 0;
							n64RealTotalOutSize = 0;
							bChangeFile = false;
							ClearNOSOUND(); 
							
							// get current system time
							GetLocalTime(&SystemTime);
							
							// set new name of output file
							DwCurSplitNo++;
							if(strcmp(SzOrgWriteFile,"null")!=0)
								SetOutputFileName(SzOrgWriteFile,SzWriteFile,DwCurSplitNo);
							else strcpy(SzWriteFile,"null");

							n64Foo = N64RealTotalDataSize+DwAddSp[0]+DwAddSp[1];
							SetCommandStrings(false,SzWriteFile,SzWriteFile,SzWriteFile,SzUserDef[0],SzUserDef[1],SzUserDef[2],SystemTime,WriteWaveFmt,n64Foo);
							strcpy(SzRealWriteFile,SzWriteFile);
			
							/* obsolete
#ifdef WIN32
							// -exec command
							if(BlExecCom) 
								SetCommandStrings(false,SzExecCmd,SzExecComand,SzRealWriteFile,SzUserDef[0],SzUserDef[1],SzUserDef[2],SystemTime,WriteWaveFmt,n64Foo);

							// -pipeout command
							if(BlCreatePipe)
							{
								SetCommandStrings(true,SzPipeCmd,SzPipeComand,SzRealWriteFile,SzUserDef[0],SzUserDef[1],SzUserDef[2],SystemTime,WriteWaveFmt,n64Foo);
								strcpy(SzWriteFile,"stdout");
							}
#endif
							*/
							
							// open new output file
							if(!OpenWriteFile(&hdWriteFile,SzWriteFile,
								/* obsolete
#ifdef USEWIN32API
								&hProcessInfo,
#endif   
								*/
								BlStdout
								/* obsolete
								,BlCreatePipe,SzPipeCmd,DwPipeBufSize
								*/
								))
							{
								goto L_ERR;
							}
							
							if(BlWaveHdrOut)
							{
								if(!BlWaveOut)
								{
									// write wave header if output is stdout
									if(BlStdout)
									{ 
										n64Foo = N64RealTotalDataSize+DwAddSp[0]+DwAddSp[1];
										WriteWaveHeader(hdWriteFile,&WriteWaveFmt,n64Foo,BlExtChunkOfHdr);
									}
									else if(hdWriteFile != NULL)
									{
										// move file pointer
#ifdef USEWIN32API
										LI.QuadPart = WAVEHDRSIZE(BlExtChunkOfHdr);
										SetFilePointer(hdWriteFile,LI.LowPart, &LI.HighPart,FILE_BEGIN);
#else
										fseek(hdWriteFile,WAVEHDRSIZE(BlExtChunkOfHdr),SEEK_SET);
#endif
									}
									
								}
							}
							
#ifdef USEWIN32API
							// get local time
							if(BlFileStamp && !BlNoSignal)
							{
								SYSTEMTIME sysTime;
								FILETIME ftLocal;
								GetLocalTime(&sysTime);
								SystemTimeToFileTime(&sysTime,&ftLocal);
								LocalFileTimeToFileTime(&ftLocal,&FtCreationTime);
								FtLastAccessTime = FtCreationTime;
								FtLastWriteTime = FtCreationTime;
								fprintf(stderr,"set timestamp to: %d/%d/%d %d:%02d:%02d\n",
									sysTime.wYear,sysTime.wMonth,sysTime.wDay,
									sysTime.wHour,sysTime.wMinute,sysTime.wSecond
									);
							}
#endif	
							
							fprintf(stderr,"\n\n----------------\nchange output\n");
							fprintf(stderr,"output: ");
							if(BlStdout){
								/* obsolete
								if(BlCreatePipe) fprintf(stderr,"[pipe] %s\n\n",SzPipeCmd);
								else */
									fprintf(stderr,"[stdout]\n");
							}
							else fprintf(stderr,"%s\n",SzWriteFile);
							
							if(FDAT.bSplit)
								fprintf(stderr,"split file: [%3d] %.2lf sec (%lu M)\n"
								,DwCurSplitNo,
								(double)FDAT.n64SplitByte/InputWaveFmt.nAvgBytesPerSec,
								FDAT.n64SplitByte/1024/1024);
							
							// add space to head
							AddSpase(hdWriteFile,DwAddSp[0],WriteWaveFmt);

					}  
					// change output file
					//--------------------------------------
					
				}
				else 
				{
					// now, normalizer is searching the peak of output.
					
					n64OutSize += dwPointsInBuf * InputWaveFmt.nBlockAlign;

					if(BlVerbose)
						ShowStatus(WriteWaveFmt,SzRealWriteFile,
						/* obsolete
						BlCreatePipe,
						*/
						n64TotalOutSize+n64OutSize,
						N64TotalDataSize,
						0,
						/* obsolete
						false,
						*/
						true,
						(BlEndless && !BlCutFile)
						);

				}
				
			/* obsolete
			}			
			else // ADP trainging mode
			{
				n64OutSize += dwPointsInBuf * InputWaveFmt.nBlockAlign;
				
				if(BlVerbose)
					ShowStatus(WriteWaveFmt,SzRealWriteFile,BlCreatePipe,
					n64TotalOutSize+n64OutSize,
					N64TotalDataSize,
					0,
					true,false,
					(BlEndless && !BlCutFile)
					);
				
				// If ADP training mode is finished, then exit loop
				if(dwADPTrainDataSize 
					&& dwADPTrainDataSize < n64TotalOutSize+n64OutSize) goto L_EXITBLOCK;
			}
			*/
			
			
		}
		// loop from while(1)...    
		
L_EXITBLOCK:
		
		if(dwCurrentNormalMode == NORMAL_NOT || dwCurrentNormalMode == NORMAL_EXEC)
			fprintf(stderr,"\n");

		n64RealTotalOutSize += n64RealOutSize;
		n64TotalOutSize += n64OutSize;
		
		/* obsolete
		// ADP training mode is over.
		if(dwADPTrainDataSize && dwADPTrainDataSize < n64TotalOutSize){
			ClearADPBuf();
			dwADPTrainDataSize = 0; 
			dwBlockNo = -1;
			
			n64RealTotalOutSize = 0;
			n64TotalOutSize = 0;
		}
		*/
		
	}
	// loop from for(dwBlockNo = 0; ...
	//-----------------------------

	
	// get gain for normalizer
	if(dwCurrentNormalMode != NORMAL_NOT && dwCurrentNormalMode != NORMAL_EXEC)
	{
		GetGainForNormalizer(dNormalGain,BlNormalStereo,dwCurrentNormalMode,DbNormalLevel,n64TotalOutSize,InputWaveFmt);

		if(dwCurrentNormalMode != NORMAL_PEAKBYDB) FDAT.bNormalUseCompressor = true;

		// use compressor
		if(FDAT.bNormalUseCompressor){
			fprintf(stderr,"limiter after normalizer:\n");
			fprintf(stderr,"ratio = %4.1lf, threshold = %6.3lf dB\n",FDAT.dNormalRatio,FDAT.dNormalTh);
			fprintf(stderr,"attack = %4.0lf msec, release = %4.0lf msec, RMS = %d\n",
				FDAT.dNormalAttack,FDAT.dNormalRelease,FDAT.dwNormalCompRMS);
		}

		dwCurrentNormalMode = NORMAL_EXEC;
	}
	
	}
	// loop from for(dwNormalLoop = 0; ...
	//-----------------------------


	//-----------------------------------------------------------------------
	//-----------------------------------------------------------------------


	fprintf(stderr,"\n");

	// add space to tail
	AddSpase(hdWriteFile,DwAddSp[1],WriteWaveFmt);

	// write wave header
	if(BlWaveHdrOut && !BlStdout) WriteWaveHeader(hdWriteFile,&WriteWaveFmt,n64RealTotalOutSize+DwAddSp[0]+DwAddSp[1],BlExtChunkOfHdr);

	// close file handle of output
#ifdef USEWIN32API
	if(hdWriteFile != NULL) CloseHandle(hdWriteFile);
	hdWriteFile = NULL;
#else
	if(hdWriteFile != NULL) fclose(hdWriteFile);
	hdWriteFile = NULL;
#endif
	
#ifdef USEWIN32API
	/* obsolete
	// wait until child process is terminated
	if(!StopChildStop(hProcessInfo,BlCreatePipe)){
		hProcessInfo.hProcess = NULL;
		goto L_ERR;
	}
	hProcessInfo.hProcess = NULL;
	*/
	
	// set timestamp
	if(BlFileStamp && !BlNoSignal)
	{
		HANDLE hTimeFile;
		hTimeFile = CreateFile(SzRealWriteFile,GENERIC_WRITE,0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL); 
		if(hTimeFile)
		{
			SetFileTime(hTimeFile,&FtCreationTime,&FtLastAccessTime,&FtLastWriteTime);
			CloseHandle(hTimeFile);
			Sleep(50);
		}
	}
#endif	
	
	
	/* obsolete
#ifdef WIN32
	// execute command
	if(BlExecCom && strcmp(SzWriteFile,"null")!=0){
		fprintf(stderr,"execute: %s\n",SzExecCmd);
		if(!ExecCommand(SzExecCmd,NULL,FALSE,BlOpenWin)){
			fprintf(stderr,"Could not execute %s.\n",SzExecCmd);
		}
	}
#endif
	*/

/* obsolete
	// write characteristics of ADP filter 
	if(BlADPOutFilter && FDAT.bADP)
		OutputADPFilterChr(SzCurDir,WriteWaveFmt.nChannels,WriteWaveFmt.nSamplesPerSec);
*/

	if(FDAT.bAutoOffset && BlVerbose){
		
		double dOffset[2],dMaxLevel;
		GetAutoOffset(dOffset);		
		
		dMaxLevel = GetMaxWaveLevel(InputWaveFmt);
		
		fprintf(stderr,"\nauto DC offset: ");
		dFoo = dOffset[0];
		fprintf(stderr,"L =  %6.3lf dB (%.2lf), ",-20*log10(fabs(dFoo)),-dFoo*dMaxLevel);
		dFoo = dOffset[1];
		fprintf(stderr,"R =  %6.3lf dB (%.2lf)\n",-20*log10(fabs(dFoo)),-dFoo*dMaxLevel);
	}

	bReturn = true;

	//----------------------------------------------------

	// if error occured, jump here 
L_ERR: 

	UnprepareAllFilters();
	unprepareOutputFileName();

	// if input is stdin, wait for EOF
	if(BlStdin && bReturn) while(SeekStdin(lpBuffer,DwBufSize,DwBufSize,0));

	// close file handles
#ifdef USEWIN32API
	if(hdReadFile) CloseHandle(hdReadFile);
	if(hdWriteFile) CloseHandle(hdWriteFile);
	CloseWaveDevice(!bReturn);
#else
	if(hdReadFile) fclose(hdReadFile);
	if(hdWriteFile) fclose(hdWriteFile);
#endif
	
	// free mem
	if(lpBuffer) free(lpBuffer);
	if(lpWriteBuffer) free(lpWriteBuffer);
	for(i=0;i<2;i++) if(lpFilterBuf[i]) free(lpFilterBuf[i]);
	
	/* obsolete
#ifdef USEWIN32API
	// wait until child process is terminated.
	if(!StopChildStop(hProcessInfo,BlCreatePipe)) bReturn = false;
#endif	
	*/
	
	return bReturn;
}

//EOF