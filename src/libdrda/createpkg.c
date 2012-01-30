/* 
 * Copyright (C) 2001 Brian Bruns (camber@ais.org)
 */

#include "drda.h"

int main(int argc, char **argv)
{
DRDA *drda;
	
	if (argc != 2) {
		fprintf(stderr, "Usage: %s [drda://]"
			"[user[.password]@]hostname[:port]"
			"/database[;option=value[...]]\n",
			argv[0]);
		fprintf(stderr, "current options are: collection, package\n");
		exit(1);
	}
	drda = drda_init();
	if (drda_set_by_url(drda, argv[1])) {
		exit(1);
	}
	drda_connect(drda);

	if (drda_excsat(drda)<0) {
		fprintf(stderr,"Failed to exchange attributes with server\n");
		exit(1);
	}
	if (drda_accsec(drda)<0) {
		fprintf(stderr,"Failed to negotiate security with server\n");
		exit(1);
	}
	if (drda_secchk(drda)<0) {
		fprintf(stderr,"Failed to logon to server\n");
		exit(1);
	}
	if (drda_accrdb(drda)<0) {
		fprintf(stderr,"Failed to access database\n");
		exit(1);
	}
	if (drda_bgnbnd(drda)<0) {
		fprintf(stderr,"Begin bind failed\n");
		exit(1);
	}
	if (drda_endbnd(drda)<0) {
		fprintf(stderr,"End bind failed\n");
		exit(1);
	}
	drda_disconnect(drda);
	drda_release(drda);
	return 0;
}
