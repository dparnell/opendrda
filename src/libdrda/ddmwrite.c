/* 
 * Copyright (C) 2001 Brian Bruns (camber@ais.org)
 */

#include "drda.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int ddm_write_rqsdss(DRDA *drda, int typ)
{
char *buf;

	buf = &drda->out_buf[drda->out_pos];

	memset(buf, 0, 2); /* zero the length, to be written later */
	buf[2] = 0xd0;   /* DDMID */
	buf[3] = typ;   /* A simple request message */
	if ( (typ&0x10) == 0 ) ++drda->rqscrr;
	drda_put_int2(&buf[4], 42); //drda->rqscrr);  /* correlation id */
	drda->out_pos += 6;

	return 6;
}
int ddm_write_excsat(DRDA *drda)
{
char *buf;

	buf = &drda->out_buf[drda->out_pos];

	memset(buf, 0, 2); /* zero the length, to be written later */
	drda_put_int2(&buf[2], DDM_EXCSAT); 
	drda->out_pos += 4;

	return 4;
}
int ddm_write_extnam(DRDA *drda)
{
char *buf;
char pid[10];
int  pid_sz;

	buf = &drda->out_buf[drda->out_pos];

	sprintf(pid, "%d", getpid());
	pid_sz = strlen(pid);

	drda_put_int2(buf, pid_sz + 4); 
	drda_put_int2(&buf[2], DDM_EXTNAM); 
	//drda_put_int2(&buf[4], pid_sz + 4); 
	//drda_put_int2(&buf[6], DDM_CHRSTRDR); 
	memcpy(&buf[4],pid, pid_sz);
	drda->out_pos += pid_sz + 4;

	return pid_sz + 4;
}
int ddm_write_mgrlvlls(DRDA *drda)
{
char *buf;

	buf = &drda->out_buf[drda->out_pos];

	memset(buf, 0, 2); /* zero the length, to be written later */
	drda_put_int2(&buf[2], DDM_MGRLVLLS); 
	drda->out_pos += 4;

	return 4;
}
int ddm_write_mgrlvl(DRDA *drda, int codept, int level)
{
char *buf;

	/* mgrlvl is a little strange since it has no length field */

	buf = &drda->out_buf[drda->out_pos];

	drda_put_int2(buf, codept); 
	/* mgrlvln */
	//drda_put_int2(&buf[2], 8); 
	//drda_put_int2(&buf[4], DDM_MGRLVLN);
	drda_put_int2(&buf[2], level); 
	drda->out_pos += 4;

	return 4;
}
int ddm_write_srvclsnm(DRDA *drda)
{
    unsigned char *buf;
    char *libname = "QAS";
    int  name_len = strlen(libname);
    char *tmpstr;

	buf = &drda->out_buf[drda->out_pos];

	drda_put_int2(buf, name_len + 8); 
	drda_put_int2(&buf[2], DDM_SRVCLSNM); 
	drda_put_int2(&buf[4], name_len + 4); 
	drda_put_int2(&buf[6], DDM_CHRSTRDR); 
     tmpstr = (char *) malloc(name_len + 1);
	drda_local_string2remote(drda, (char*)&buf[4], name_len, tmpstr);
	memcpy(&buf[8],libname, name_len);
	free(tmpstr);
	drda->out_pos += name_len + 8;

	return name_len + 8;
}
int ddm_write_srvnam(DRDA *drda,char *servername)
{
char *buf;
int  name_len = strlen(servername);

	buf = &drda->out_buf[drda->out_pos];

	drda_put_int2(buf, name_len + 4); 
	drda_put_int2(&buf[2], DDM_SRVNAM); 
	memcpy(&buf[4],servername, name_len);
	drda->out_pos += name_len + 4;

	return name_len + 4;
}
int ddm_write_accsec(DRDA *drda)
{
char *buf;

	buf = &drda->out_buf[drda->out_pos];

	memset(buf, 0, 2); /* zero the length, to be written later */
	drda_put_int2(&buf[2], DDM_ACCSEC); 
	drda->out_pos += 4;

	return 4;
}
int ddm_write_secmgrnm(DRDA *drda, char *secmgr)
{
char *buf;
int secmgr_sz = 0;

	if (secmgr) {
		secmgr_sz = strlen(secmgr);
	}
	buf = &drda->out_buf[drda->out_pos];

	drda_put_int2(buf, secmgr_sz + 4); 
	drda_put_int2(&buf[2], DDM_SECMGRNM); 
	if (secmgr_sz) {
		memcpy(&buf[4], secmgr, secmgr_sz);
	}
	drda->out_pos += 4 + secmgr_sz;

	return 4 + secmgr_sz;
}
int ddm_write_secmec(DRDA *drda, int mechanism)
{
char *buf;

	buf = &drda->out_buf[drda->out_pos];

	drda_put_int2(buf, 6); 
	drda_put_int2(&buf[2], DDM_SECMEC); 
	drda_put_int2(&buf[4], mechanism); 
	drda->out_pos += 6;

	return 6;
}
int ddm_write_secchk(DRDA *drda)
{
char *buf;

	buf = &drda->out_buf[drda->out_pos];

	memset(buf, 0, 2); /* zero the length, to be written later */
	drda_put_int2(&buf[2], DDM_SECCHK); 
	drda->out_pos += 4;

	return 4;
}
int ddm_write_accrdb(DRDA *drda)
{
char *buf;

	buf = &drda->out_buf[drda->out_pos];

	memset(buf, 0, 2); /* zero the length, to be written later */
	drda_put_int2(&buf[2], DDM_ACCRDB); 
	drda->out_pos += 4;

	return 4;
}
int ddm_write_usrid(DRDA *drda, char *username)
{
char *buf, *tmpstr;
int username_len;
#if 0
char usrid[11];
#endif

	buf = &drda->out_buf[drda->out_pos];

	username_len = strlen(username);
	drda_put_int2(buf, username_len + 4); 
	drda_put_int2(&buf[2], DDM_USRID); 
     tmpstr = (char *) malloc(username_len + 1);
     drda_local_string2remote(drda, username, username_len, tmpstr);
	memcpy(&buf[4],tmpstr,username_len);
     free(tmpstr);
	drda->out_pos += 4 + username_len;

	return 4 + username_len;
#if 0
	memset(usrid,0x40,11);
	strncpy(usrid,username,10);
	drda_put_int2(buf, 14); 
	drda_put_int2(&buf[2], DDM_USRID); 
     tmpstr = (char *) malloc(11);
	memset(tmpstr,0,11);
     drda_local_string2remote(usrid, 10, tmpstr);
	memcpy(&buf[4],tmpstr,10);
     free(tmpstr);
	drda->out_pos += 14;

	return 14;
#endif
}
int ddm_write_password(DRDA *drda, char *password)
{
char *buf, *tmpstr;
int password_len;
#if 0
char passwd[11];
#endif

	buf = &drda->out_buf[drda->out_pos];
	password_len = strlen(password);
	drda_put_int2(buf, password_len + 4); 
	drda_put_int2(&buf[2], DDM_PASSWORD); 
     tmpstr = (char *) malloc(password_len + 1);
     drda_local_string2remote(drda, password, password_len, tmpstr);
	memcpy(&buf[4],tmpstr,password_len);
     free(tmpstr);
	drda->out_pos += 4 + password_len;

	return 4 + password_len;
#if 0
	memset(passwd,0x40,11);
	strncpy(passwd,password,10);
	drda_put_int2(buf, 14); 
	drda_put_int2(&buf[2], DDM_PASSWORD); 
     tmpstr = (char *) malloc(11);
	memset(tmpstr,0,11);
     drda_local_string2remote(passwd, 10, tmpstr);
	memcpy(&buf[4],tmpstr,10);
     free(tmpstr);
	drda->out_pos += 14;

	return 14;
#endif
}
int ddm_write_rdbnam(DRDA *drda, char *database)
{
char *buf;

	buf = &drda->out_buf[drda->out_pos];

	drda_put_int2(buf, 22); 
	drda_put_int2(&buf[2], DDM_RDBNAM); 

     drda_local_string2remote_pad(drda, database, 18, &buf[4]);

	drda->out_pos += 22;

	return 22;
}
int ddm_write_rdbacccl(DRDA *drda)
{
char *buf;
char dbname[19];

	buf = &drda->out_buf[drda->out_pos];

	drda_put_int2(buf, 6); 
	drda_put_int2(&buf[2], DDM_RDBACCCL); 
	drda_put_int2(&buf[4], DDM_SQLAM); 

	drda->out_pos += 6;

	return 6;
}
int ddm_write_typdefnam(DRDA *drda)
{
char *buf;
/*
** unless anybody can produce a server in which this typedef (QTDSQLASC, aka 
** network byte order) does not work, this will be the only typdef we support.
*/
/* char *typdefnam = "QTDSQLX86"; */
char typdefnam[] = {0xd8,0xe3,0xc4,0xe2,0xd8,0xd3,0xc1,0xe2,0xc3};

	buf = &drda->out_buf[drda->out_pos];

	drda_put_int2(buf, 13); 
	drda_put_int2(&buf[2], DDM_TYPDEFNAM); 
	memcpy(&buf[4], typdefnam, 9);

	drda->out_pos += 13;

	return 13;
}
int ddm_write_prdid(DRDA *drda)
{
char *buf;

	buf = &drda->out_buf[drda->out_pos];

	drda_put_int2(buf, 12); 
	drda_put_int2(&buf[2], DDM_PRDID); 
	drda_local_string2remote_pad(drda, DRDA_PRDID, 8, &buf[4]);

	drda->out_pos += 12;

	return 12;
}
int ddm_write_crrtkn(DRDA *drda)
{
char *buf;

	buf = &drda->out_buf[drda->out_pos];

	drda_put_int2(buf, 23); 
	drda_put_int2(&buf[2], DDM_CRRTKN);
	/* DRDA_PRDID (in drda.h) must be exactly 8 chars or this goes to hell */
	drda_local_string2remote(drda, drda->crrtkn, 19, &buf[4]);

	drda->out_pos += 23;

	return 23;
}
int ddm_write_typdefovr(DRDA *drda)
{
char *buf;

	buf = &drda->out_buf[drda->out_pos];

	drda_put_int2(buf, 10); 
	drda_put_int2(&buf[2], DDM_TYPDEFOVR);
	drda_put_int2(&buf[4], 6);
	drda_put_int2(&buf[6], DDM_CCSIDSBC);
	drda_put_int2(&buf[8], 819);

	drda->out_pos += 10;

	return 10;
}

int ddm_write_excsqlimm(DRDA *drda)
{
char *buf;

	buf = &drda->out_buf[drda->out_pos];

	memset(buf, 0, 2); /* zero the length, to be written later */
	drda_put_int2(&buf[2], DDM_EXCSQLIMM); 
	drda->out_pos += 4;

	return 4;
}

int ddm_write_rdbcmtok(DRDA *drda)
{
char *buf;

	buf = &drda->out_buf[drda->out_pos];

	drda_put_int2(buf, 5); 
	drda_put_int2(&buf[2], DDM_RDBCMTOK); 
	buf[4] = 0xF1;

	drda->out_pos += 5;

	return 5;
}

int ddm_write_pkgnamcsn(DRDA *drda)
{
char *buf;

	buf = &drda->out_buf[drda->out_pos];

	drda_put_int2(buf, 68); 
	drda_put_int2(&buf[2], DDM_PKGNAMCSN); 

	/* rdbnam */
	drda_local_string2remote_pad(drda, drda->database, 18, &buf[4]);

	/* rdbcolid */
	drda_local_string2remote_pad(drda, drda->collection, 18, &buf[22]);
	
	/* pkgid */
	drda_local_string2remote_pad(drda, drda->package, 18, &buf[40]);
	
	/* pkgcstkn */
	//memset(&buf[58],0x40,8);
	memcpy(&buf[58],"OD000001",8);
	
	/* pkgsn */
	drda_put_int2(&buf[66], 0x01); 
	
	drda->out_pos += 68;

	return 68;
}

int ddm_write_sqlstt(DRDA *drda, char *sql)
{
char *buf;
int sql_len;

	sql_len = strlen(sql);
	buf = &drda->out_buf[drda->out_pos];

	drda_put_int2(buf, sql_len + 8); 
	drda_put_int2(&buf[2], DDM_SQLSTT); 

	drda_put_int2(&buf[4], 0); 
	drda_put_int2(&buf[6], sql_len); 
	//drda_local_string2remote(sql, sql_len, &buf[8]);
	memcpy(&buf[8],sql,sql_len);

	drda->out_pos += sql_len + 8;

	return sql_len + 8;
}

int ddm_write_rdbcmm(DRDA *drda)
{
char *buf;

	buf = &drda->out_buf[drda->out_pos];

	drda_put_int2(buf, 4); 
	drda_put_int2(&buf[2], DDM_RDBCMM); 

	drda->out_pos += 4;

	return 4;
}

int ddm_write_bgnbnd(DRDA *drda)
{
char *buf;

	buf = &drda->out_buf[drda->out_pos];

	memset(buf, 0, 2); /* zero the length, to be written later */
	drda_put_int2(&buf[2], DDM_BGNBND); 

	drda->out_pos += 4;

	return 4;
}

int ddm_write_endbnd(DRDA *drda)
{
char *buf;

	buf = &drda->out_buf[drda->out_pos];

	memset(buf, 0, 2); /* zero the length, to be written later */
	drda_put_int2(&buf[2], DDM_ENDBND); 

	drda->out_pos += 4;

	return 4;
}

int ddm_write_pkgnamct(DRDA *drda)
{
char *buf;

	buf = &drda->out_buf[drda->out_pos];

	drda_put_int2(buf, 66); 
	drda_put_int2(&buf[2], DDM_PKGNAMCT); 

	/* rdbnam */
	drda_local_string2remote_pad(drda, drda->database, 18, &buf[4]);

	/* rdbcolid */
	drda_local_string2remote_pad(drda, drda->collection, 18, &buf[22]);
	
	/* pkgid */
	drda_local_string2remote_pad(drda, drda->package, 18, &buf[40]);
	
	/* pkgcstkn */
	memcpy(&buf[58],"OD000001",8);
	
	drda->out_pos += 66;

	return 66;
}

int ddm_write_pkgisolvl(DRDA *drda)
{
char *buf;

	buf = &drda->out_buf[drda->out_pos];

	drda_put_int2(buf, 6); 
	drda_put_int2(&buf[2], DDM_PKGISOLVL); 
	drda_put_int2(&buf[4], DDM_ISOLVLCHG); 

	drda->out_pos += 6;

	return 6;
}

int ddm_write_prpsqlstt(DRDA *drda)
{
char *buf;

	buf = &drda->out_buf[drda->out_pos];

	memset(buf, 0, 2); /* zero the length, to be written later */
	drda_put_int2(&buf[2], DDM_PRPSQLSTT); 
	drda->out_pos += 4;

	return 4;
}

int ddm_write_excsqlstt(DRDA *drda)
{
char *buf;

	buf = &drda->out_buf[drda->out_pos];

	memset(buf, 0, 2); /* zero the length, to be written later */
	drda_put_int2(&buf[2], DDM_EXCSQLSTT); 
	drda->out_pos += 4;

	return 4;
}

int ddm_write_opnqry(DRDA *drda)
{
char *buf;

	buf = &drda->out_buf[drda->out_pos];

	memset(buf, 0, 2); /* zero the length, to be written later */
	drda_put_int2(&buf[2], DDM_OPNQRY); 
	drda->out_pos += 4;

	return 4;
}
int ddm_write_cntqry(DRDA *drda)
{
char *buf;

	buf = &drda->out_buf[drda->out_pos];

	memset(buf, 0, 2); /* zero the length, to be written later */
	drda_put_int2(&buf[2], DDM_CNTQRY); 
	drda->out_pos += 4;

	return 4;
}

int ddm_write_qryblksz(DRDA *drda, int sz)
{
char *buf;

	buf = &drda->out_buf[drda->out_pos];

	drda_put_int2(buf, 8); 
	drda_put_int2(&buf[2], DDM_QRYBLKSZ); 
	drda_put_int4(&buf[4], sz); 
	drda->out_pos += 8;

	return 8;
}

int ddm_write_rtnsqlda(DRDA *drda)
{
char *buf;

	buf = &drda->out_buf[drda->out_pos];

	drda_put_int2(buf, 5); 
	drda_put_int2(&buf[2], DDM_RTNSQLDA); 
	buf[4] = 0xf1; /* true */

	drda->out_pos += 5;

	return 5;
}
