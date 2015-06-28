// main 

#include <string>

#include "filter.h"

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
		if(!Filter()){
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
