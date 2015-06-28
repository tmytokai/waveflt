// main 

#ifdef WIN32
#include <windows.h>
#endif
#include <stdio.h>
#include <string>

DWORD  GetVer(char* szVer);
BOOL ReadOption(int argc, char *argv_org[]);
BOOL SetParam();
BOOL FilterBody();
void DeleteInputFile();

#define CHR_BUF 256 

int main(int argc,char *argv[])
{
	char szVer[CHR_BUF];

	// show title
	GetVer(szVer);
	if(argc == 1)
		fprintf(stdout,"%s\n\n",szVer);	
	else
		fprintf(stderr,"%s\n\n",szVer);	

	try{
		// read options
		if(!ReadOption(argc,argv)){
			fprintf(stderr,"Fatal error.\n");
			exit(1);
		}

		// initialize parameters
		if(!SetParam()){
			fprintf(stderr,"Fatal error.\n");
			exit(1);
		}

		// filter
		if(!FilterBody()){
			fprintf(stderr,"Fatal error.\n");
			exit(1);
		}

		DeleteInputFile();
	}
	catch( std::string& errmsg )
	{
		fprintf(stderr,"\nwaveflt: %s\n",errmsg.c_str());
		exit(1);
	}
	
	return 0;
}
