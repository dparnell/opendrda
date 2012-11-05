/* 
 * Copyright (C) 2001 Brian Bruns (camber@ais.org)
 */



#ifndef _drda_h_
#define _drda_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

#include "drdatypes.h"
#include "ddm.h"

#define DRDA_PRDID "OPD00020"

typedef	short	DRDA_INT2;
typedef	unsigned short	DRDA_UINT2;
typedef	long		DRDA_INT4;
typedef	unsigned long	DRDA_UINT4;

typedef struct _drda_sqlca {
	/* SQLCAGRP */
	int	sqlcode;
	char	sqlstate[6];
	char	sqlerrproc[9];
	/* SQLCAXGRP */
	char	sqlrdbnam[19];
	int	sqlerrd1;
	int	sqlerrd2;
	int	sqlerrd3;
	int	sqlerrd4;
	int	sqlerrd5;
	int	sqlerrd6;
	int	sqlwarn0;
	int	sqlwarn1;
	int	sqlwarn2;
	int	sqlwarn3;
	int	sqlwarn4;
	int	sqlwarn5;
	int	sqlwarn6;
	int	sqlwarn7;
	int	sqlwarn8;
	int	sqlwarn9;
	int	sqlwarna;
	char	*sqlerrmsg;
} DRDA_SQLCA;

typedef struct _drda_column {
	/* from SQLDAGRP */
	int	precision;
	int	scale;
	int 	length;
	int	type;
	int	ccsid;
	char	*name;
	char	*label;
	char	*comments;
	int  fdoca_type;
	int  fdoca_len;
	void *bind_addr;
	int *len_bind_addr;
	/* bookeeping for SQLGetData et al. */
	unsigned char *cur_addr;
	int cur_length;
} DRDA_COLUMN;

typedef struct _drda_sqlda {
	/* SQLNUMROW -> SQLNUMGRP */
	int	num_cols;
	DRDA_COLUMN **columns;
} DRDA_SQLDA;

typedef struct _drda {
	int  s;             /* connection to server */
    char*   local_encoding;
    char*   remote_encoding;
	unsigned char* out_buf;
	int  out_pos;
    int  out_size;
	/* error handling */
	int  err_set;
	int  severity_code;
	int	err_code;
	int  err_code_pt;
	char *err_diag_msg;
	/* server attributes */
	int sat_agent;
	int sat_secmgr;
	int sat_cmntcpip;
	int sat_sqlam;
	int sat_ccsidmgr;
	int sat_rdb;
	char *sat_extnam;
	char *sat_srvclsnm;
	char *sat_srvnam;
	char *sat_srvrlslv;
	char *typdefnam;
	DRDA_INT2 rqscrr;
	/* correlation token, set upon connect */
	/* 8 ip addr, '.', 4 ip port, 6 fixed bytes from time  = 19 */
	char crrtkn[19]; 
	/* */
	char *sql;
	char *database;
	char *collection;
	char *package;
	char *usrid;
	char *passwd;
	char *server;
	unsigned short port;
    char *application_name;
	/* fdoca objects received from the server */
	DRDA_SQLCA	*sqlca;
	DRDA_SQLDA	*sqlda;
	int result_pos;
	int result_len;
	unsigned char *result_buf;
} DRDA;

/* prototypes */

/* ... convert.c */
int drda_set_null_value(DRDA *drda, unsigned char *dest, int desttype);
int drda_convert(DRDA *drda, unsigned char *src, int srctype, int srclen,
                    unsigned char *dest, int desttype, int destlen);
int drda_get_cardinal_type(int fdoca_type);

/* ... ddmread.c */
int  ddm_read_dss             (DRDA *drda, unsigned char *buf);
int  ddm_read_syntaxrm        (DRDA *drda, unsigned char *buf);
int  ddm_read_secchkcd        (DRDA *drda, unsigned char *buf);
int  ddm_read_svrcod          (DRDA *drda, unsigned char *buf);
int  ddm_read_synerrcd        (DRDA *drda, unsigned char *buf);
int  ddm_read_srvdgn          (DRDA *drda, unsigned char *buf);
int  ddm_read_codpnt          (DRDA *drda, unsigned char *buf);
int  ddm_read_prmnsprm        (DRDA *drda, unsigned char *buf);
int  ddm_read_mgrlvlrm        (DRDA *drda, unsigned char *buf);
int  ddm_read_mgrlvlls        (DRDA *drda, unsigned char *buf);
int  ddm_read_rpymsg_subclass (DRDA *drda, unsigned char *buf);
int  ddm_read_excsatrd        (DRDA *drda, unsigned char *buf);
int  ddm_read_extnam          (DRDA *drda, unsigned char *buf);
int  ddm_read_srvclsnm        (DRDA *drda, unsigned char *buf);
int  ddm_read_srvnam          (DRDA *drda, unsigned char *buf);
int  ddm_read_srvrlslv        (DRDA *drda, unsigned char *buf);
int  ddm_read_accsecrd        (DRDA *drda, unsigned char *buf);
int  ddm_read_secmec          (DRDA *drda, unsigned char *buf);
int  ddm_read_secchkrm        (DRDA *drda, unsigned char *buf);
int  ddm_read_accrdbrm        (DRDA *drda, unsigned char *buf);
int  ddm_read_prdid           (DRDA *drda, unsigned char *buf);
int  ddm_read_typdefnam       (DRDA *drda, unsigned char *buf);
int  ddm_read_typdefovr       (DRDA *drda, unsigned char *buf);
int  ddm_read_ccsidsbc        (DRDA *drda, unsigned char *buf);
int  ddm_read_prccnvrm        (DRDA *drda, unsigned char *buf);
int  ddm_read_sqldard         (DRDA *drda, unsigned char *buf);

/*   ... ddmwrite.c */
int  ddm_write_rqsdss         (DRDA *drda, int typ);
int  ddm_write_excsat         (DRDA *drda);
int  ddm_write_extnam         (DRDA *drda);
int  ddm_write_mgrlvlls       (DRDA *drda);
int  ddm_write_mgrlvl         (DRDA *drda, int codept, int level);
int  ddm_write_srvclsnm       (DRDA *drda);
int  ddm_write_srvnam         (DRDA *drda,char *servername);
int  ddm_write_accsec         (DRDA *drda);
int  ddm_write_secmgrnm       (DRDA *drda, char *secmgr);
int  ddm_write_secmec         (DRDA *drda, int mechanism);
int  ddm_write_secchk         (DRDA *drda);
int  ddm_write_accrdb         (DRDA *drda);
int  ddm_write_usrid          (DRDA *drda, char *username);
int  ddm_write_password       (DRDA *drda, char *password);
int  ddm_write_rdbnam         (DRDA *drda, char *database);
int  ddm_write_rdbacccl       (DRDA *drda);
int  ddm_write_typdefnam      (DRDA *drda);
int  ddm_write_prdid          (DRDA *drda);
int  ddm_write_crrtkn         (DRDA *drda);
int  ddm_write_typdefovr      (DRDA *drda);
int  ddm_write_excsqlimm      (DRDA *drda);
int  ddm_write_rdbcmtok       (DRDA *drda);
int  ddm_write_pkgnamcsn      (DRDA *drda);
int  ddm_write_sqlstt         (DRDA *drda, char *sql);
int  ddm_write_rdbcmm         (DRDA *drda);
int  ddm_write_bgnbnd         (DRDA *drda);
int  ddm_write_endbnd         (DRDA *drda);
int  ddm_write_pkgnamct       (DRDA *drda);
int  ddm_write_pkgisolvl      (DRDA *drda);
int  ddm_write_prpsqlstt      (DRDA *drda);
int  ddm_write_rtnsqlda       (DRDA *drda);

/*   ... debug.c */
void drda_dump_buf(FILE *dumpfile, const void *buf, int length);

/*   ... drda.c */
int  drda_excsat              (DRDA *drda);
int  drda_accsec              (DRDA *drda);
int  drda_secchk              (DRDA *drda);
int  drda_accrdb              (DRDA *drda);
int  drda_excsqlimm           (DRDA *drda, char *sql);
int  drda_rdbcmm              (DRDA *drda);
int  drda_bgnbnd              (DRDA *drda);
int  drda_endbnd              (DRDA *drda);
int  drda_prpsqlstt           (DRDA *drda, char *sql);

/*   ... fdoca.c */
int  fdoca_read_sqlca         (DRDA *drda, unsigned char *buf);
int  fdoca_read_sqlda         (DRDA *drda, unsigned char *buf);

/*   ... handle.c */
DRDA *drda_init               ();
int drda_connect              (DRDA *drda);
void drda_clear_error         (DRDA *drda);
void drda_disconnect          (DRDA *drda);
void drda_release             (DRDA *drda);
DRDA_SQLCA *drda_alloc_sqlca  (DRDA *drda);
void drda_free_sqlca          (DRDA_SQLCA *sqlca);
DRDA_SQLDA *drda_alloc_sqlda  (DRDA *drda);
void drda_free_sqlda          (DRDA_SQLDA *sqlda);
void drda_alloc_columns       (DRDA_SQLDA *sqlda, int num_cols);

/*  ... user.c */
void drda_set_server          (DRDA *drda, char *server);
void drda_set_database        (DRDA *drda, char *database);
void drda_set_user            (DRDA *drda, char *user);
void drda_set_password        (DRDA *drda, char *password);
void drda_set_port            (DRDA *drda, unsigned short port);
int drda_set_by_url           (DRDA *drda, const char *url);
int drda_set_by_keyvalue      (DRDA *drda, char *k);
void drda_set_local_encoding  (DRDA *drda, char *encoding);
void drda_set_remote_encoding (DRDA *drda, char *encoding);
void drda_set_application_name (DRDA *drda, char *application);

/*   ... util.c */
void drda_buffer_init         (DRDA *drda);
DRDA_INT2 drda_get_int2       (unsigned char *buf);
DRDA_INT4 drda_get_int4       (unsigned char *buf);
DRDA_INT2 drda_get_leint2     (unsigned char *buf);
DRDA_INT4 drda_get_leint4     (unsigned char *buf);
void drda_put_int2            (unsigned char *buf, DRDA_INT2 val);
void drda_put_int4(unsigned char *buf, DRDA_INT2 val);
DRDA_INT2 drda_get_endian_int2(DRDA *drda, unsigned char *buf);
DRDA_INT4 drda_get_endian_int4(DRDA *drda, unsigned char *buf);

unsigned char *drda_remote_string2local       (DRDA *drda, unsigned char *in_buf, size_t in_bytes,  unsigned char *out_buf);
unsigned char *drda_local_string2remote       (DRDA *drda, unsigned char *in_buf, size_t in_bytes,  unsigned char *out_buf);
void drda_local_string2remote_pad    (DRDA *drda, unsigned char *in_buf, size_t out_bytes, unsigned char *out_buf);

void drda_log(int debug_lvl, FILE *dumpfile, const char *fmt, ...);
    
#ifdef __cplusplus
}
#endif

#endif
