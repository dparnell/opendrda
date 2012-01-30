/* 
 * Copyright (C) 2001 Brian Bruns (camber@ais.org)
 */

#include "drda.h"
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <pwd.h>
#include <sys/types.h>
#include <stdlib.h>

void drda_set_user(DRDA *drda, char *usrid)
{
	if (drda->usrid) { free(drda->usrid); }
	drda->usrid = strdup(usrid);
}
void drda_set_password(DRDA *drda, char *passwd)
{
	if (drda->passwd) { free(drda->passwd); }
	drda->passwd = strdup(passwd);
}
void drda_set_database(DRDA *drda, char *database)
{
	if (drda->database) { free(drda->database); }
	drda->database = strdup(database);
}
void drda_set_collection(DRDA *drda, char *collection)
{
	if (drda->collection) { free(drda->collection); }
	drda->collection = strdup(collection);
}
void drda_set_package(DRDA *drda, char *package)
{
	if (drda->package) { free(drda->package); }
	drda->package = strdup(package);
}
void drda_set_server(DRDA *drda, char *server)
{
	if (drda->server) { free(drda->server); }
	drda->server = strdup(server);
}
void drda_set_port(DRDA *drda, unsigned short port)
{
	drda->port = port;
}
int drda_set_by_url(DRDA *drda, char *url) 
{
char *s = url, *p1, *p2;
char *url_des = "drda";
int need_passwd = 0;
char *pw;
char *host;
int port, i;

	if (p1 = strstr(s,"://")) {
		*p1 = '\0';
		/* URL schemes should be matched case insensitive (RFC 1738) */
		for (i=0;i<strlen(s);i++) {
			s[i] = tolower(s[i]);
		}
		if (strcmp(s,url_des)) {
			fprintf(stderr, "unsupported URL scheme %s\n",s);
			return 1;
		}
		s += strlen(s) + 3;
	}

	/* user name and password */
	if (p1 = strchr(s,'@')) {
		*p1 = '\0';
		p2 = strchr(s,':');
		if (!p2) p2 = strchr(s,'.');
		if (p2) {
			*p2 = '\0';
			drda_set_user(drda,s);
			s += strlen(s) + 1;
			drda_set_password(drda,s);
			s += strlen(s) + 1;
		} else {
			drda_set_user(drda,s);
			s += strlen(s) + 1;
			need_passwd = 1;
		}
	} else {
		struct passwd *pw = getpwent();
		drda_set_user(drda,pw->pw_name);
		need_passwd = 1;
	}

	/* host and port */
	if (!(p1 = strchr(s,'/'))) {
		fprintf(stderr, "malformed url, no database specified\n");
		return 1;
	}
	*p1 = '\0';
	if (p2 = strchr(s,':')) {
		*p2 = '\0';
		drda_set_server(drda,s);
		s += strlen(s) + 1;
		drda_set_port(drda,atoi(s));
		s += strlen(s) + 1;
	} else {
		drda_set_server(drda,s);
		drda_set_port(drda,446);
		s += strlen(s) + 1;
	}

	if (need_passwd) {
		pw = getpass("Password: ");
		drda_set_password(drda,pw);
	}
	if (p1 = strchr(s,';')) {
		*p1 = '\0';
		drda_set_database(drda,s);
		s += strlen(s) + 1;
		while (p2 = strchr(s,';')) {
			*p2 = '\0';
			s += drda_set_by_keyvalue(drda, s);
		}
		/* catch the last one */
		drda_set_by_keyvalue(drda, s);
	} else if (p1 = strchr(s,':')) {
	/* we'd prefer the use of ;, but that needs to be escaped when using a unix
	   so we also accept ':' */
		*p1 = '\0';
		drda_set_database(drda,s);
		s += strlen(s) + 1;
		while (p2 = strchr(s,':')) {
			*p2 = '\0';
			s += drda_set_by_keyvalue(drda, s);
		}
		/* catch the last one */
		drda_set_by_keyvalue(drda, s);
	} else {
		drda_set_database(drda,s);
	}

	return 0;
}
int drda_set_by_keyvalue(DRDA *drda, char *k) 
{
char *v;
int klen = strlen(k) + 1;

	v = strchr(k,'=');
	if (!v) {
		fprintf(stderr,"malformed option %s, skipping...\n",k);
		return 0;
	}
	*v='\0';
	v++;
	if (!strcmp(k, "collection")) {
		drda_set_collection(drda,v);
	} else if (!strcmp(k, "package")) {
		drda_set_package(drda,v);
	} else {
		fprintf(stderr,"unrecognized option %s, skipping...\n",k);
	}
	return klen;
}
