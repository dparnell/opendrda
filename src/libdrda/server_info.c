/* 
 * Copyright (C) 2001 Brian Bruns (camber@ais.org)
 */

#include "drda.h"
#include <stdlib.h>

int main(int argc, char **argv)
{
DRDA *drda;
	
	if (argc < 3) {
	 	printf("Usage: %s <host> <port>\n\n", argv[0]);
		exit(1);
	}
	drda = drda_init();
	drda_set_server(drda,argv[1]);
	drda_set_port(drda,atoi(argv[2]));
	if (!drda_connect(drda)) {
		fprintf(stderr,"Failed to connect!\n");
		exit(1);
	}

	if (drda_excsat(drda)<0) {
		fprintf(stderr,"Failed to exchange attributes with server\n");
		exit(1);
	}
	printf("Server Name:     %s\n", 
		drda->sat_srvnam ? drda->sat_srvnam : "Unknown");
	printf("External Name:   %s\n", 
		drda->sat_extnam ? drda->sat_extnam : "Unknown");
	printf("Class Name:      %s\n", 
		drda->sat_srvclsnm ? drda->sat_srvclsnm : "Unknown");
	printf("Product Release: %s\n", 
		drda->sat_srvrlslv ? drda->sat_srvrlslv : "Unknown");
	printf("Agent Level:         %3d\n", drda->sat_agent); 
	printf("Security Mgr Level:  %3d\n", drda->sat_secmgr); 
	printf("TCP/IP Mgr Level:    %3d\n", drda->sat_cmntcpip); 
	printf("SQL Mgr Level:       %3d\n", drda->sat_sqlam); 
	printf("Database Level:      %3d\n", drda->sat_rdb); 
	printf("Code Page:           %3d\n", drda->sat_ccsidmgr); 
	drda_disconnect(drda);
	drda_release(drda); 

	return 0;
}

