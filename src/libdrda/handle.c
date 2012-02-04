/* 
 * Copyright (C) 2001 Brian Bruns (camber@ais.org)
 */

#include "drda.h"
#include <sys/socket.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

/*
** All manner of routines dealing with the DRDA struct and sub structs.
** routines dealing with framing the protocol belong in drda.c instead.
*/ 

DRDA *drda_init()
{
DRDA *drda;

	drda = (DRDA *) malloc(sizeof(DRDA));
	memset(drda, 0, sizeof(DRDA));
	drda->collection = strdup("NULLID");
	drda->package = strdup("OPENDRDA");
    drda->local_encoding = strdup("UTF-8");
    drda->remote_encoding = strdup("EBCDIC");
	return drda;
}

int drda_connect(DRDA *drda)
{
    struct sockaddr_in      sin;
    int     fd;
    struct sockaddr_in name;
    socklen_t len = sizeof(struct sockaddr_in);
    time_t t;
    struct hostent   *host;
    char ip[100];

	host = gethostbyname(drda->server);
	if (host) {
		struct in_addr *ptr = (struct in_addr *) host->h_addr;
		strncpy(ip, inet_ntoa(*ptr), 17);
#if DRDALOG
		fprintf(stdout,"IP is %s\n",ip);
#endif
		sin.sin_addr.s_addr = inet_addr(ip);
	} else {
		sin.sin_addr.s_addr = inet_addr(drda->server);
	}
	sin.sin_family = AF_INET;
	sin.sin_port = htons(drda->port);

	if ((fd = socket (AF_INET, SOCK_STREAM, 0)) < 0) {
		perror ("socket");
		return 0;
	}

	/* begin tenger@idirect.com 8/6/2001 */

	/* C911, pages 281 and 434, rule CA2, say to use SO_LINGER
	   and SO_KEEPALIVE. */

	{ 
		int rc;
		int optval;
		struct linger {
			int l_onoff;
			int l_linger;
		} optlinger;

		optval =1;
		rc = setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE,
			&optval, sizeof(optval));
		if (rc<0) {
			perror("setsockopt,SO_KEEPALIVE");
			return 0;
		}

		optlinger.l_onoff  = 1;
		optlinger.l_linger = 0;
		rc = setsockopt(fd, SOL_SOCKET, SO_LINGER,
			&optlinger, sizeof(optlinger));
		if (rc<0) {
			perror("setsockopt,SO_LINGER");
			return 0;
		}
	}

	/* end tenger@idirect.com */


	if (connect(fd, (struct sockaddr *) &sin, sizeof(sin)) <0) {
        perror("connect");
        return 0;
     }



	drda->s = fd;
     if (getsockname(drda->s, (struct sockaddr*)&name, &len)>=0) {
		sprintf(drda->crrtkn, "%08x\n",htonl(*((unsigned long *) &name.sin_addr)));
		drda->crrtkn[8]='.';
		sprintf(&(drda->crrtkn[9]), "%04x\n",(unsigned int)name.sin_port);
		/* convert time() to big endian (network order) and used the six 
		** least significant bytes */
		t = htonl(time(NULL));
		memcpy(&drda->crrtkn[13], &t, 6);
	}
	return drda->s;
}
void drda_disconnect(DRDA *drda)
{
	if (drda->s) {
		close(drda->s);
		drda->s = 0;
	}
}
void drda_release(DRDA *drda)
{
	drda_clear_error(drda);
    if (drda->out_buf) { free(drda->out_buf); }
    if (drda->remote_encoding) { free(drda->remote_encoding); }
    if (drda->local_encoding) { free(drda->local_encoding); }
	if (drda->sat_extnam) { free(drda->sat_extnam); }
	if (drda->sat_srvclsnm) { free(drda->sat_srvclsnm); }
	if (drda->sat_srvnam) { free(drda->sat_srvnam); }
	if (drda->sat_srvrlslv) { free(drda->sat_srvrlslv); }
	if (drda->usrid) { free(drda->usrid); }
	if (drda->passwd) { free(drda->passwd); }
	if (drda->database) { free(drda->database); }
	if (drda->collection) { free(drda->collection); }
	if (drda->package) { free(drda->package); }
	if (drda->server) { free(drda->server); }
	if (drda->sqlca) { drda_free_sqlca(drda->sqlca); }
	free(drda);
}
void drda_clear_error(DRDA *drda)
{
	if (drda->err_set) {
		drda->err_set=0;
     	drda->severity_code=0;
     	drda->err_code=0;
     	drda->err_code_pt=0;
     	if (drda->err_diag_msg) {
			free(drda->err_diag_msg);
     		drda->err_diag_msg=NULL;
		}
	}
}
DRDA_SQLCA *drda_alloc_sqlca(DRDA *drda)
{
DRDA_SQLCA *sqlca;

	sqlca = (DRDA_SQLCA *) malloc(sizeof(DRDA_SQLCA));
	memset(sqlca, 0, sizeof(DRDA_SQLCA));

	return sqlca;
}
void drda_free_sqlca(DRDA_SQLCA *sqlca)
{
	if (sqlca->sqlerrmsg) free(sqlca->sqlerrmsg);
	free(sqlca);
}
DRDA_SQLDA *drda_alloc_sqlda(DRDA *drda)
{
DRDA_SQLDA *sqlda;

	sqlda = (DRDA_SQLDA *) malloc(sizeof(DRDA_SQLDA));
	memset(sqlda, 0, sizeof(DRDA_SQLDA));

	return sqlda;
}
void drda_free_sqlda(DRDA_SQLDA *sqlda)
{
int i;

	if (sqlda->columns) {
		for (i=0;i<sqlda->num_cols;i++) {
			if (sqlda->columns[i]->name) free(sqlda->columns[i]->name);
			if (sqlda->columns[i]->label) free(sqlda->columns[i]->label);
			if (sqlda->columns[i]->comments) free(sqlda->columns[i]->comments);
			free(sqlda->columns[i]);
		}
		free(sqlda->columns);
	}
	free(sqlda);
}
void drda_alloc_columns(DRDA_SQLDA *sqlda, int num_cols)
{
int i;

	sqlda->num_cols = num_cols;
	sqlda->columns = (DRDA_COLUMN **) malloc(sizeof(DRDA_COLUMN *) * num_cols);
	for (i=0;i<num_cols;i++) {
		sqlda->columns[i] = (DRDA_COLUMN *) malloc(sizeof(DRDA_COLUMN));
		memset(sqlda->columns[i], 0, sizeof(DRDA_COLUMN));
	}
}
