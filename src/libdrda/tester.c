/* 
 * Copyright (C) 2001 Brian Bruns (camber@ais.org)
 */

#include "drda.h"
#include <unistd.h>
#include <string.h>
#include <pwd.h>
#include <sys/types.h>

int main(int argc, char **argv)
{
struct sockaddr_in      sin;
unsigned char buf[BUFSIZ];
int     len, i, pos;
DRDA *drda;
unsigned bound_data[256][256];

	if (argc<2) {
		fprintf(stderr, "Usage: tester [drda://][user[.password]@]hostname[:port]/database[;option=value[...]]\n");
		fprintf(stderr, "current options are: collection, package\n");
		fprintf(stderr, "example: db2inst1@localhost:446/sample\n");
		exit(1);
	}	

	drda = drda_init();
	if (drda_set_by_url(drda,argv[1])) {
		exit(1);
	}
/*
	drda_connect(drda, "127.0.0.1", 3506);
	drda_set_user(drda,"db2inst1");
	drda_set_password(drda,"adaptive");
	drda_set_database(drda,"sample");
*/
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

	if (drda_prpsqlstt(drda,"select * from department for fetch only")<0) {
		fprintf(stderr,"SQL failed\n");
		exit(1);
	}
	fprintf(stderr, "num cols %d\n", drda->sqlda->num_cols);
	for (i=0;i< drda->sqlda->num_cols; i++) {
		drda->sqlda->columns[i]->bind_addr = bound_data[i];
	}
	if (drda_opnqry(drda)<0) {
		fprintf(stderr,"Execute SQL failed\n");
		exit(1);
	}
	drda->result_pos = 0;
	while (drda->result_pos<drda->result_len) {
		// fprintf(stderr, "pos %d len %d\n", drda->result_pos,drda->result_len);
		drda->result_pos += fdoca_read_qrydta_row(drda, drda->result_buf, drda->result_len, drda->result_pos);
		for (i=0;i< drda->sqlda->num_cols; i++) {
			fprintf(stderr,"%s\t",bound_data[i]);
		}
		fprintf(stderr,"\n");
	}
/*
	if (drda_excsqlimm(drda,"update department set location = 'BB' where deptno = 'A00'")<0) {
		fprintf(stderr,"SQL failed\n");
		exit(1);
	}
	if (drda_rdbcmm(drda)<0) {
		fprintf(stderr,"Commit failed\n");
		exit(1);
	}
*/
	drda_disconnect(drda);
	drda_release(drda);
	
	return 0;
}

