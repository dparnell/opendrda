/* 
 * Copyright (C) 2001 Brian Bruns (camber@ais.org)
 */

#include "drda.h"
#include <string.h>
#include <stdlib.h>

int ddm_read_qryprctyp(DRDA *drda, unsigned char *buf);
int ddm_read_sqlcsrhld(DRDA *drda, unsigned char *buf);
int fdoca_read_qrydsc(DRDA *drda, char *buf);
int fdoca_read_qrydta(DRDA *drda, unsigned char *buf, int len);

static int bad_code_point(int expected, int codept, char *funcname, char *objname)
{
	if (codept != expected) {
		fprintf(stderr,"%s called on something that is not a %s object!\n",
			funcname, objname);
		return -1;
	}
	return 0;
}

static int bad_length(int expected, int len, char *funcname)
{
	if (len != expected) {
		fprintf(stderr,"%s has incorrect length, should be %d but found %d!\n", funcname, expected, len);
		return -1;
	}
	return 0;
}

int ddm_read_syntaxrm(DRDA *drda, unsigned char *buf)
{
    char *funcname = "ddm_read_syntaxrm";
    char *cpname = "SYNTAXRM";
    int len, codept;

	len = drda_get_int2(buf);
	codept = drda_get_int2(&buf[2]);
	if (bad_code_point(DDM_SYNTAXRM, codept, funcname, cpname)) {
		return -1;
	}

	drda_log(0,stderr, "%s object\n",cpname);
	drda_log(0,stderr, "Len of message: %d\n", len);

	return ddm_read_rpymsg_subclass(drda, buf);
}

int ddm_read_secchkcd(DRDA *drda, unsigned char *buf)
{
    int len, codept;
    char *funcname = "ddm_read_secchkcd";
    char *cpname = "SECCHKCD";

	len = drda_get_int2(buf);
	codept = drda_get_int2(&buf[2]);
	if (bad_code_point(DDM_SECCHKCD, codept, funcname, cpname)) {
		return -1;
	}
	if (bad_length(5, len, funcname)) {
		return -1;
	}

	drda->err_code = (int) buf[4];
	drda->err_set = 1;

	drda_log(0,stderr, "%s object\n", cpname);
	drda_log(0,stderr, "Len of message: %d\n", len);
	drda_log(0,stderr, "Security Check Code: %02x\n", buf[4]);
    
    return 0;
}

int ddm_read_svrcod(DRDA *drda, unsigned char *buf)
{
    int len, codept;
    char *funcname = "ddm_read_svrcod";
    char *cpname = "SVRCOD";

	len = drda_get_int2(buf);
	codept = drda_get_int2(&buf[2]);
	if (bad_code_point(DDM_SVRCOD, codept, funcname, cpname)) {
		return -1;
	}
	if (bad_length(6, len, funcname)) {
		return -1;
	}

	drda->severity_code = drda_get_int2(&buf[4]);
	drda->err_set = 1;

	drda_log(0,stderr, "%s object\n", cpname);
	drda_log(0,stderr, "Len of message: %d\n", len);
	drda_log(0,stderr, "Severity Code: %d\n", drda->severity_code);
	return 0;
}

int ddm_read_synerrcd(DRDA *drda, unsigned char *buf)
{
    char *funcname = "ddm_read_synerrcd";
    char *cpname = "SYNERRCD";
    int len, codept;

	len = drda_get_int2(buf);
	codept = drda_get_int2(&buf[2]);
	if (bad_code_point(DDM_SYNERRCD, codept, funcname, cpname)) {
		return -1;
	}
	if (bad_length(5, len, funcname)) {
		return -1;
	}


	drda->err_code = (int) buf[4];
	drda->err_set = 1;

	drda_log(0,stderr, "%s object\n",cpname);
	drda_log(0,stderr, "Len of message: %d\n", len);
	drda_log(0,stderr, "Error Code: %02x\n", drda->err_code);

	return 0;
}

int ddm_read_srvdgn(DRDA *drda, unsigned char *buf)
{
    int len, codept;
    char *tmpstr; 
    char *funcname="ddm_read_srvdgn";
    char *cpname="SRVDGN";

	len = drda_get_int2(buf);
	codept = drda_get_int2(&buf[2]);
	if (bad_code_point(DDM_SRVDGN, codept, funcname, cpname)) {
		return -1;
	}

	drda_log(0,stderr, "%s object\n", cpname);
	drda_log(0,stderr, "Len of message: %d\n", len);

	tmpstr = (char *) malloc(len - 3);
	drda_remote_string2local(drda, (char*)&buf[4], len - 4, tmpstr);
	drda_log(0,stderr, "Message: %s\n", tmpstr);
	if (drda->err_diag_msg) {
		free(drda->err_diag_msg);
	}
	drda->err_diag_msg = strdup(tmpstr);
	drda->err_set = 1;
	free(tmpstr);

	return 0;
}

int ddm_read_codpnt(DRDA *drda, unsigned char *buf)
{
int len, codept;
char *funcname="ddm_read_codpnt";
char *cpname="CODPNT";


/* found a codpnt with a length of 5 but according to doc that is way too short */
	len = drda_get_int2(buf);
	codept = drda_get_int2(&buf[2]);
	if (bad_code_point(DDM_CODPNT, codept, funcname, cpname)) {
		return -1;
	}

	drda->err_code_pt = drda_get_int2(&buf[4]);
/*
	drda->err_set = 1;
*/

	drda_log(0,stderr, "%s object\n",cpname);
	drda_log(0,stderr, "Len of message: %d\n", len);
	drda_log(0,stderr, "code point: %04x\n", drda->err_code_pt);

	return 0;
}
int ddm_read_prmnsprm(DRDA *drda, unsigned char *buf)
{
int len, codept;
char *funcname="ddm_read_prmnsprm";
char *cpname="PRMNSPRM";

	len = drda_get_int2(buf);
	codept = drda_get_int2(&buf[2]);
	if (bad_code_point(DDM_PRMNSPRM, codept, funcname, cpname)) {
		return -1;
	}

	drda_log(0,stderr, "%s object\n", cpname);
	drda_log(0,stderr, "Len of message: %d\n", len);

	return ddm_read_rpymsg_subclass(drda, buf);
}
int ddm_read_mgrlvlrm(DRDA *drda, unsigned char *buf)
{
int len, codept;
char *funcname="ddm_read_mgrlvlrm";
char *cpname="MGRLVLRM";

	len = drda_get_int2(buf);
	codept = drda_get_int2(&buf[2]);
	if (bad_code_point(DDM_MGRLVLRM, codept, funcname, cpname)) {
		return -1;
	}

	drda_log(0,stderr, "%s object\n",cpname);
	drda_log(0,stderr, "Len of message: %d\n", len);

	return ddm_read_rpymsg_subclass(drda, buf);
}
int ddm_read_mgrlvlls(DRDA *drda, unsigned char *buf)
{
int len, codept, pos, val;
char *funcname = "ddm_read_mgrlvlls";
char *cpname = "MGRLVLLS";

	len = drda_get_int2(buf);
	codept = drda_get_int2(&buf[2]);
	if (bad_code_point(DDM_MGRLVLLS, codept, funcname, cpname)) {
		return -1;
	}

	drda_log(0,stderr, "%s object\n",cpname);
	drda_log(0,stderr, "Len of message: %d\n", len);

	pos = 4;
	while (pos < len) {
		codept = drda_get_int2(&buf[pos]);
		val = drda_get_int2(&buf[pos+2]);
		pos += 4;
		drda_log(0,stderr,"Manager: %04x Value: %d\n",codept, val);
		switch (codept) {
			case DDM_AGENT:
				drda->sat_agent = val;
			case DDM_SECMGR:
				drda->sat_secmgr = val;
			case DDM_CMNTCPIP:
				drda->sat_cmntcpip = val;
			case DDM_SQLAM:
				drda->sat_sqlam = val;
			case DDM_CCSIDMGR:
				drda->sat_ccsidmgr = val;
			case DDM_RDB:
				drda->sat_rdb = val;
		}
	}

	return 0;
}

int ddm_read_rpymsg_subclass(DRDA *drda, unsigned char *buf) 
{
int len, codept, pos, sublen;

	len = drda_get_int2(buf);
	pos = 4;
	while (pos < len) {
		sublen = drda_get_int2(&buf[pos]);
		codept = drda_get_int2(&buf[pos+2]);
		switch (codept) {
			case DDM_SVRCOD:
				ddm_read_svrcod(drda, &buf[4]);
				break;
			case DDM_SECCHKCD:
				ddm_read_secchkcd(drda, &buf[pos]);
				break;
			case DDM_SYNERRCD:
				ddm_read_synerrcd(drda, &buf[pos]);
				break;
			case DDM_CODPNT:
				ddm_read_codpnt(drda, &buf[pos]);
				break;
			case DDM_SRVDGN:
				ddm_read_srvdgn(drda, &buf[pos]);
				break;
			case DDM_PRDID:
				ddm_read_prdid(drda, &buf[pos]);
				break;
			case DDM_TYPDEFNAM:
				ddm_read_typdefnam(drda, &buf[pos]);
				break;
			case DDM_TYPDEFOVR:
				ddm_read_typdefovr(drda, &buf[pos]);
				break;
			case DDM_MGRLVLLS:
				ddm_read_mgrlvlls(drda, &buf[pos]);
				break;
			case DDM_QRYPRCTYP:
				ddm_read_qryprctyp(drda, &buf[pos]);
				break;
			case DDM_SQLCSRHLD:
				ddm_read_sqlcsrhld(drda, &buf[pos]);
				break;
			default:
				fprintf(stderr,"ddm_read_rpymsg_subclass: Unhandled code point %04x Length %04x\n", codept, sublen);
				break;
			
		}
		pos += sublen;
	}
	return 0;
}

int ddm_read_excsatrd(DRDA *drda, unsigned char *buf)
{
    int len, codept, pos, sublen;
    char *funcname = "ddm_read_excsatrd";
    char *cpname = "EXCSATRD";

	len = drda_get_int2(buf);
	codept = drda_get_int2(&buf[2]);
	if (bad_code_point(DDM_EXCSATRD, codept, funcname, cpname)) {
		return -1;
	}

	drda_log(0,stderr, "%s object\n", cpname);
	drda_log(0,stderr, "Len of message: %d\n", len);

	pos = 4;
	while (pos < len) {
		sublen = drda_get_int2(&buf[pos]);
		codept = drda_get_int2(&buf[pos+2]);
		switch (codept) {
			case DDM_EXTNAM:
				ddm_read_extnam(drda, &buf[pos]);
				break;
			case DDM_SRVCLSNM:
				ddm_read_srvclsnm(drda, &buf[pos]);
				break;
			case DDM_SRVNAM:
				ddm_read_srvnam(drda, &buf[pos]);
				break;
			case DDM_SRVRLSLV:
				ddm_read_srvrlslv(drda, &buf[pos]);
				break;
			case DDM_MGRLVLLS:
				ddm_read_mgrlvlls(drda, &buf[pos]);
				break;
			default:
				fprintf(stderr,"%s: Unhandled code point %04x\n",funcname, codept);
				break;
		}
		pos += sublen;
	}
    
    return 0;
}

int ddm_read_extnam(DRDA *drda, unsigned char *buf)
{
    int len, codept;
    char *tmpstr;
    char *funcname = "ddm_read_extnam";
    char *cpname = "EXTNAM";

	len = drda_get_int2(buf);
	codept = drda_get_int2(&buf[2]);
	if (bad_code_point(DDM_EXTNAM, codept, funcname, cpname)) {
		return -1;
	}

	tmpstr = (char *) malloc(len - 3);
	drda_local_string2remote(drda, (char*)&buf[4], len - 4, tmpstr);

	if (drda->sat_extnam) { free(drda->sat_extnam); }
	drda->sat_extnam = strdup(tmpstr);

	drda_log(0,stderr, "%s object\n",cpname);
	drda_log(0,stderr, "Len of message: %d\n", len);
	drda_log(0,stderr, "External Name: %s\n", tmpstr);

	free(tmpstr);
	return 0;
}

int ddm_read_srvclsnm(DRDA *drda, unsigned char *buf)
{
    int len, codept;
    char *tmpstr;
    char *funcname = "ddm_read_srvclnm";
    char *cpname = "SRVCLSNM";

	len = drda_get_int2(buf);
	codept = drda_get_int2(&buf[2]);
	if (bad_code_point(DDM_SRVCLSNM, codept, funcname, cpname)) {
		return -1;
	}

	tmpstr = (char *) malloc(len - 3);
	drda_local_string2remote(drda, (char*)&buf[4], len - 4, tmpstr);

	if (drda->sat_srvclsnm) { free(drda->sat_srvclsnm); }
	drda->sat_srvclsnm = strdup(tmpstr);

	drda_log(0,stderr, "%s object\n",cpname);
	drda_log(0,stderr, "Len of message: %d\n", len);
	drda_log(0,stderr, "Server Class Name: %s\n", tmpstr);

	free(tmpstr);
	return 0;
}

int ddm_read_srvnam(DRDA *drda, unsigned char *buf)
{
    int len, codept;
    char *tmpstr;
    char *funcname = "ddm_read_srvnam";
    char *cpname = "SRVNAM";

	len = drda_get_int2(buf);
	codept = drda_get_int2(&buf[2]);
	if (bad_code_point(DDM_SRVNAM, codept, funcname, cpname)) {
		return -1;
	}

	tmpstr = (char *) malloc(len - 3);
	drda_local_string2remote(drda, (char*)&buf[4], len - 4, tmpstr);

	if (drda->sat_srvnam) { free(drda->sat_srvnam); }
	drda->sat_srvnam = strdup(tmpstr);

	drda_log(0,stderr, "%s object\n",cpname);
	drda_log(0,stderr, "Len of message: %d\n", len);
	drda_log(0,stderr, "Server Name: %s\n", tmpstr);

	free(tmpstr);
	return 0;
}

int ddm_read_srvrlslv(DRDA *drda, unsigned char *buf)
{
    int len, codept;
    char *tmpstr;
    char *funcname = "ddm_read_srvrlslv";
    char *cpname = "SRVRLSLV";

	len = drda_get_int2(buf);
	codept = drda_get_int2(&buf[2]);
	if (bad_code_point(DDM_SRVRLSLV, codept, funcname, cpname)) {
		return -1;
	}

	tmpstr = (char *) malloc(len - 3);
	drda_local_string2remote(drda, (char*)&buf[4], len - 4, tmpstr);

	if (drda->sat_srvrlslv) { free(drda->sat_srvrlslv); }
	drda->sat_srvrlslv = strdup(tmpstr);

	drda_log(0,stderr, "%s object\n",cpname);
	drda_log(0,stderr, "Len of message: %d\n", len);
	drda_log(0,stderr, "Server Release Level: %s\n", tmpstr);

	free(tmpstr);
	return 0;
}

int ddm_read_accsecrd(DRDA *drda, unsigned char *buf)
{
    int len, codept, pos, sublen;
    char *funcname = "ddm_read_accsecrd";
    char *cpname = "ACCSECRD";

	len = drda_get_int2(buf);
	codept = drda_get_int2(&buf[2]);
	if (bad_code_point(DDM_ACCSECRD, codept, funcname, cpname)) {
		return -1;
	}

	drda_log(0,stderr, "%s object\n", cpname);
	drda_log(0,stderr, "Len of message: %d\n", len);

	pos = 4;
	while (pos < len) {
		sublen = drda_get_int2(&buf[pos]);
		codept = drda_get_int2(&buf[pos+2]);
		switch (codept) {
			case DDM_SECMEC:
				ddm_read_secmec(drda, &buf[pos]);
				break;
			default:
				fprintf(stderr,"%s: Unhandled code point %04x\n",funcname, codept);
				break;
		}
		pos += sublen;
	}
	return 0;
}

int ddm_read_secmec(DRDA *drda, unsigned char *buf)
{
    int len, codept;
    char *funcname = "ddm_read_secmec";
    char *cpname = "SECMEC";

	len = drda_get_int2(buf);
	codept = drda_get_int2(&buf[2]);
	if (bad_code_point(DDM_SECMEC, codept, funcname, cpname)) {
		return -1;
	}
	if (bad_length(6, len, funcname)) {
		return -1;
	}

	drda_log(0,stderr, "%s object\n",cpname);
	drda_log(0,stderr, "Len of message: %d\n", len);
	drda_log(0,stderr, "Server Mechanism: %d\n", drda_get_int2(&buf[4]));
	return 0;
}

int ddm_read_secchkrm(DRDA *drda, unsigned char *buf)
{
    char *funcname = "ddm_read_secchkrm";
    char *cpname = "SECCHKRM";
    int len, codept;

	len = drda_get_int2(buf);
	codept = drda_get_int2(&buf[2]);
	if (bad_code_point(DDM_SECCHKRM, codept, funcname, cpname)) {
		return -1;
	}

	drda_log(0,stderr, "%s object\n",cpname);
	drda_log(0,stderr, "Len of message: %d\n", len);

	return ddm_read_rpymsg_subclass(drda, buf);
}

int ddm_read_accrdbrm(DRDA *drda, unsigned char *buf)
{
    char *funcname = "ddm_read_accrdbrm";
    char *cpname = "ACCRDBRM";
    int len, codept;

	len = drda_get_int2(buf);
	codept = drda_get_int2(&buf[2]);
	if (bad_code_point(DDM_ACCRDBRM, codept, funcname, cpname)) {
		return -1;
	}

	drda_log(0,stderr, "%s object\n",cpname);
	drda_log(0,stderr, "Len of message: %d\n", len);

	return ddm_read_rpymsg_subclass(drda, buf);
}

int ddm_read_prdid(DRDA *drda, unsigned char *buf)
{
    char *funcname = "ddm_read_prdid";
    char *cpname = "PRDID";
    int len, codept;
    char *tmpstr;

	len = drda_get_int2(buf);
	codept = drda_get_int2(&buf[2]);
	if (bad_code_point(DDM_PRDID, codept, funcname, cpname)) {
		return -1;
	}

	tmpstr = (char *) malloc(9);
	drda_local_string2remote(drda, (char*)&buf[4], 8, tmpstr);
	drda_log(0,stderr, "%s object\n",cpname);
	drda_log(0,stderr, "Len of message: %d\n", len);
	drda_log(0,stderr, "prdid: %s\n", tmpstr);
	free(tmpstr);
	return 0;
}

int ddm_read_typdefnam(DRDA *drda, unsigned char *buf)
{
    char *funcname = "ddm_read_typdefnam";
    char *cpname = "TYPDEFNAM";
    int len, codept;
    char *tmpstr;

	len = drda_get_int2(buf);
	codept = drda_get_int2(&buf[2]);
	if (bad_code_point(DDM_TYPDEFNAM, codept, funcname, cpname)) {
		return -1;
	}

	tmpstr = (char *) malloc(len - 4);
	drda_local_string2remote(drda, (char*)&buf[4], len - 4, tmpstr);
	drda->typdefnam = strdup(tmpstr);
	drda_log(0,stderr, "%s object\n",cpname);
	drda_log(0,stderr, "Len of message: %d\n", len);
	drda_log(0,stderr, "typedef name: %s\n", tmpstr);
	free(tmpstr);
	return 0;
}

int ddm_read_typdefovr(DRDA *drda, unsigned char *buf)
{
    char *funcname = "ddm_read_typdefovr";
    char *cpname = "TYPDEFOVR";
    int len, codept, pos, sublen;

	len = drda_get_int2(buf);
	codept = drda_get_int2(&buf[2]);
	if (bad_code_point(DDM_TYPDEFOVR, codept, funcname, cpname)) {
		return -1;
	}

	drda_log(0,stderr, "%s object\n",cpname);
	drda_log(0,stderr, "Len of message: %d\n", len);
	pos = 4;
	while (pos < len) {
		sublen = drda_get_int2(&buf[pos]);
		codept = drda_get_int2(&buf[pos+2]);
		switch (codept) {
			case DDM_CCSIDSBC:
				ddm_read_ccsidsbc(drda, &buf[pos]);
				break;
			default:
				fprintf(stderr,"%s: Unhandled code point %04x\n",funcname, codept);
				break;
		}
		pos += sublen;
	}
	return 0;
}

int ddm_read_ccsidsbc(DRDA *drda, unsigned char *buf)
{
    char *funcname = "ddm_read_ccsidsbc";
    char *cpname = "CCSIDSBC";
    int len, codept;

	len = drda_get_int2(buf);
	codept = drda_get_int2(&buf[2]);
	if (bad_code_point(DDM_CCSIDSBC, codept, funcname, cpname)) {
		return -1;
	}

	drda_log(0,stderr, "%s object\n",cpname);
	drda_log(0,stderr, "Len of message: %d\n", len);
	drda_log(0,stderr, "Code Page: %d\n", drda_get_int2(&buf[4]));
	return 0;
}


int ddm_read_prccnvrm(DRDA *drda, unsigned char *buf)
{
    char *funcname = "ddm_read_prccnvrm";
    char *cpname = "PRCCNVRM";
    int len, codept;

	len = drda_get_int2(buf);
	codept = drda_get_int2(&buf[2]);
	if (bad_code_point(DDM_PRCCNVRM, codept, funcname, cpname)) {
		return -1;
	}

	drda_log(0,stderr, "%s object\n",cpname);
	drda_log(0,stderr, "Len of message: %d\n", len);

	return ddm_read_rpymsg_subclass(drda, buf);
}

int ddm_read_sqlcsrhld(DRDA *drda, unsigned char *buf)
{
    char *funcname = "ddm_read_sqlcsrhld";
    char *cpname = "SQLCSRHLD";
    int len, codept;

	len = drda_get_int2(buf);
	codept = drda_get_int2(&buf[2]);
	if (bad_code_point(DDM_SQLCSRHLD, codept, funcname, cpname)) {
		return -1;
	}

	drda_log(0,stderr, "%s object\n",cpname);
	drda_log(0,stderr, "Len of message: %d\n", len);
	drda_log(0,stderr, "Hold Option: %s\n", buf[4]==0xf1 ? "true" : "false");

	return 0;
}

int ddm_read_opnqryrm(DRDA *drda, unsigned char *buf)
{
    char *funcname = "ddm_read_opnqryrm";
    char *cpname = "OPNQRYRM";
    int len, codept;

	len = drda_get_int2(buf);
	codept = drda_get_int2(&buf[2]);
	if (bad_code_point(DDM_OPNQRYRM, codept, funcname, cpname)) {
		return -1;
	}

	drda_log(0,stderr, "%s object\n",cpname);
	drda_log(0,stderr, "Len of message: %d\n", len);

	return ddm_read_rpymsg_subclass(drda, buf);
}

int ddm_read_endqryrm(DRDA *drda, unsigned char *buf)
{
    char *funcname = "ddm_read_endqryrm";
    char *cpname = "ENDQRYRM";
    int len, codept;

	len = drda_get_int2(buf);
	codept = drda_get_int2(&buf[2]);
	if (bad_code_point(DDM_ENDQRYRM, codept, funcname, cpname)) {
		return -1;
	}

	drda_log(0,stderr, "%s object\n",cpname);
	drda_log(0,stderr, "Len of message: %d\n", len);

	return ddm_read_rpymsg_subclass(drda, buf);
}

int ddm_read_qrydsc(DRDA *drda, unsigned char *buf)
{
    char *funcname = "ddm_read_qrydsc";
    char *cpname = "QRYDSC";
    int len, codept;

	len = drda_get_int2(buf);
	codept = drda_get_int2(&buf[2]);
	if (bad_code_point(DDM_QRYDSC, codept, funcname, cpname)) {
		return -1;
	}

	drda_log(0,stderr, "%s object\n",cpname);
	drda_log(0,stderr, "Len of message: %d\n", len);

	return fdoca_read_qrydsc(drda, (char*)&buf[4]);
}

int ddm_read_qrydta(DRDA *drda, unsigned char *buf)
{
char *funcname = "ddm_read_qrydta";
char *cpname = "QRYDTA";
int len, codept;

	len = drda_get_int2(buf);
	codept = drda_get_int2(&buf[2]);
	if (bad_code_point(DDM_QRYDTA, codept, funcname, cpname)) {
		return -1;
	}

	drda_log(0,stderr, "%s object\n",cpname);
	drda_log(0,stderr, "Len of message: %d\n", len);

	return fdoca_read_qrydta(drda, &buf[4], len - 4);
}

int ddm_read_sqldard(DRDA *drda, unsigned char *buf)
{
    char *funcname = "ddm_read_sqldard";
    char *cpname = "SQLDARD";
    int len, codept, ca_sz;

	len = drda_get_int2(buf);
	codept = drda_get_int2(&buf[2]);
	if (bad_code_point(DDM_SQLDARD, codept, funcname, cpname)) {
		return -1;
	}

	drda_log(0,stderr, "%s object\n",cpname);
	drda_log(0,stderr, "Len of message: %d\n", len);

	ca_sz = fdoca_read_sqlca(drda, &buf[4]);
	fdoca_read_sqlda(drda, &buf[4 + ca_sz]);

	return 0;
}

int ddm_read_sqlcard(DRDA *drda, unsigned char *buf)
{
    char *funcname = "ddm_read_sqlcard";
    char *cpname = "SQLCARD";
    int len, codept;

	len = drda_get_int2(buf);
	codept = drda_get_int2(&buf[2]);
	if (bad_code_point(DDM_SQLCARD, codept, funcname, cpname)) {
		return -1;
	}

	drda_log(0,stderr, "%s object\n",cpname);
	drda_log(0,stderr, "Len of message: %d\n", len);

	fdoca_read_sqlca(drda, &buf[4]);

	return 0;
}

int ddm_read_qryprctyp(DRDA *drda, unsigned char *buf)
{
char *funcname = "ddm_read_qryprctyp";
char *cpname = "QRYPRCTYP";
int len, codept;

	len = drda_get_int2(buf);
	codept = drda_get_int2(&buf[2]);
	if (bad_code_point(DDM_QRYPRCTYP, codept, funcname, cpname)) {
		return -1;
	}

	drda_log(0,stderr, "%s object\n",cpname);
	drda_log(0,stderr, "Len of message: %d\n", len);
	drda_log(0,stderr, "Protocol Type: %04x\n", drda_get_int2(&buf[4]));

	return 0;
}

/*
 * ddm_read_dss will handle either rpydss or objdss responses
 */
int ddm_read_dss(DRDA *drda, unsigned char *buf)
{
    char *funcname = "ddm_read_dss";
    char *cpname = "DSS";
    int len, sublen, codept, pos;
    
	len = drda_get_int2(buf);
	if (buf[2] != 0xd0) {
		fprintf(stderr,"%s called on something that is not a %s object!\n", funcname, cpname);
		return -1;
	}
    
	drda_log(0,stderr, "RPYDSS object\n");
	drda_log(0,stderr, "Len of message: %d\n", len);
	drda_log(0,stderr, "DDMID: %d\n", buf[2]);
	drda_log(0,stderr, "Type of Message: ");
	switch(buf[3]) {
		case 0x02: drda_log(0,stderr,"Reply Message\n"); break;
		case 0x03: drda_log(0,stderr,"Object Response\n"); break;
		case 0x42: drda_log(0,stderr,"Chained Reply Message, same RC\n"); break;
		case 0x52: drda_log(0,stderr,"Chained Reply Message, different RC\n"); 
		case 0x53: drda_log(0,stderr,"Chained Object Message, same RC\n"); 
            break;
		default:   drda_log(0,stderr,"Unrecognized Type %02x\n",buf[3]); break;
	}
	drda_log(0,stderr, "Correlation Indentifier: %d\n", drda_get_int2(&buf[4]));
    
	pos = 6;
	while (pos < len) {
		sublen = drda_get_int2(&buf[pos]);
		codept = drda_get_int2(&buf[pos+2]);
		drda_log(0,stderr,"Sublen %04x Code Point %04x\n", sublen, codept);
		switch (codept) {
			case DDM_ACCRDBRM:
				ddm_read_accrdbrm(drda, &buf[pos]);
				break;
			case DDM_EXCSATRD:
				ddm_read_excsatrd(drda, &buf[pos]);
				break;
			case DDM_ACCSECRD:
				ddm_read_accsecrd(drda, &buf[pos]);
				break;
			case DDM_MGRLVLRM:
				ddm_read_mgrlvlrm(drda, &buf[pos]);
				break;
			case DDM_SECCHKRM:
				ddm_read_secchkrm(drda, &buf[pos]);
				break;
			case DDM_SYNTAXRM:
				ddm_read_syntaxrm(drda, &buf[pos]);
				break;
			case DDM_PRMNSPRM:
				ddm_read_prmnsprm(drda, &buf[pos]);
				break;
			case DDM_PRCCNVRM:
				ddm_read_prccnvrm(drda, &buf[pos]);
				break;
			case DDM_OPNQRYRM:
				ddm_read_opnqryrm(drda, &buf[pos]);
				break;
			case DDM_ENDQRYRM:
				ddm_read_endqryrm(drda, &buf[pos]);
				break;
			case DDM_QRYDSC:
				ddm_read_qrydsc(drda, &buf[pos]);
				break;
			case DDM_QRYDTA:
				ddm_read_qrydta(drda, &buf[pos]);
				break;
			case DDM_SQLDARD:
				ddm_read_sqldard(drda, &buf[pos]);
				break;
			case DDM_SQLCARD:
				ddm_read_sqlcard(drda, &buf[pos]);
				break;
			default:
				fprintf(stderr,"Unhandled code point %04x\n",codept);
				break;
		}
		pos += sublen;
	}
	return pos;
	
}