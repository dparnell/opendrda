/* 
 * Copyright (C) 2001 Brian Bruns (camber@ais.org)
 */

#include "drda.h"
#include <sys/socket.h>

int ddm_write_opnqry(DRDA *drda);
int ddm_write_qryblksz(DRDA *drda, int sz);
int ddm_write_cntqry(DRDA *drda);

/*
** This file contains the basic framing of the protocol messages, the protocol
** data units themselves are DDM objects and are in ddmwrite.c or ddmread.c
*/

	   static int chain_read(DRDA *drda, unsigned char *buf);

/*
* Exchange Server Attributes
*/
int drda_excsat(DRDA *drda)
{
    int len;
    int rqsdss_len = 0;
    int rqsdss_len_pos;
    int excsat_len = 0;
    int excsat_len_pos;
    int mgrlvlls_len = 0;
    int mgrlvlls_len_pos;
    unsigned char *buf;
    char c[256];

	drda_buffer_init(drda);
	rqsdss_len_pos = drda->out_pos;
	rqsdss_len += ddm_write_rqsdss(drda, 0x01);
	
	buf = &drda->out_buf[drda->out_pos];
	
	excsat_len_pos = drda->out_pos;
	excsat_len = ddm_write_excsat(drda);
	excsat_len += ddm_write_extnam(drda);

	mgrlvlls_len_pos = drda->out_pos;
	mgrlvlls_len = ddm_write_mgrlvlls(drda);
	mgrlvlls_len += ddm_write_mgrlvl(drda, DDM_AGENT, 5);
	mgrlvlls_len += ddm_write_mgrlvl(drda, DDM_SECMGR, 1);
	mgrlvlls_len += ddm_write_mgrlvl(drda, DDM_CMNTCPIP, 5);
	mgrlvlls_len += ddm_write_mgrlvl(drda, DDM_SQLAM, 4);
//	mgrlvlls_len += ddm_write_mgrlvl(drda, DDM_CCSIDMGR, 850);
	mgrlvlls_len += ddm_write_mgrlvl(drda, DDM_RDB, 5);
	drda_put_int2(&drda->out_buf[mgrlvlls_len_pos], mgrlvlls_len);
	excsat_len += mgrlvlls_len;
    
	excsat_len += ddm_write_srvclsnm(drda);
	/* tje, begin insertion, 2001-07-20 */
	gethostname (c, sizeof(c));
	excsat_len += ddm_write_srvnam(drda, c);
	/* tje, end insertion, 2001-07-20 */


	drda_put_int2(&drda->out_buf[excsat_len_pos], excsat_len);
	rqsdss_len += excsat_len;

	drda_put_int2(&drda->out_buf[rqsdss_len_pos], rqsdss_len);

	drda_dump_buf(stderr, drda->out_buf, rqsdss_len);
	write(drda->s,drda->out_buf,rqsdss_len);
	len = chain_read(drda, buf);

	return len;
}

/*
* Negotiate the security mechanism
*/
int drda_accsec(DRDA *drda)
{
    int len;
    int rqsdss_len = 0;
    int rqsdss_len_pos;
    int accsec_len = 0;
    int accsec_len_pos;
    unsigned char *buf;

	drda_buffer_init(drda);
	rqsdss_len_pos = drda->out_pos;
	rqsdss_len += ddm_write_rqsdss(drda, 0x01);

	buf = &drda->out_buf[drda->out_pos];

	accsec_len_pos = drda->out_pos;
	accsec_len = ddm_write_accsec(drda);
	// accsec_len += ddm_write_secmgrnm(drda,NULL);
	accsec_len += ddm_write_secmec(drda, 3); /* 3: userid and passwd */
	rqsdss_len += accsec_len;

	drda_put_int2(&drda->out_buf[accsec_len_pos], accsec_len);
	drda_put_int2(&drda->out_buf[rqsdss_len_pos], rqsdss_len);

	drda_dump_buf(stderr, drda->out_buf, rqsdss_len);
	write(drda->s,drda->out_buf,rqsdss_len);
	len = chain_read(drda, buf);

	return len;
}

/*
* Send login information
*/
int drda_secchk(DRDA *drda)
{
    int len;
    int rqsdss_len = 0;
    int rqsdss_len_pos;
    int secchk_len = 0;
    int secchk_len_pos;
    unsigned char *buf;

	drda_buffer_init(drda);
	rqsdss_len_pos = drda->out_pos;
	rqsdss_len += ddm_write_rqsdss(drda, 0x01);

	buf = &drda->out_buf[drda->out_pos];

	secchk_len_pos = drda->out_pos;
	secchk_len = ddm_write_secchk(drda);
	secchk_len += ddm_write_secmec(drda, 3); /* 3: userid and passwd */
	secchk_len += ddm_write_rdbnam(drda, drda->database);     
	secchk_len += ddm_write_usrid(drda, drda->usrid); 
	secchk_len += ddm_write_password(drda, drda->passwd);
	rqsdss_len += secchk_len;

	drda_put_int2(&drda->out_buf[secchk_len_pos], secchk_len);
	drda_put_int2(&drda->out_buf[rqsdss_len_pos], rqsdss_len);

	drda_dump_buf(stderr, drda->out_buf, rqsdss_len);
	write(drda->s,drda->out_buf,rqsdss_len);
	len = chain_read(drda, buf);

	return len;
}

/*
* Send info on what database to access
*/
int drda_accrdb(DRDA *drda)
{
    int len;
    int rqsdss_len = 0;
    int rqsdss_len_pos;
    int accrdb_len = 0;
    int accrdb_len_pos;
    unsigned char *buf;

	drda_buffer_init(drda);
	rqsdss_len_pos = drda->out_pos;
	rqsdss_len += ddm_write_rqsdss(drda, 0x01);

	buf = &drda->out_buf[drda->out_pos];

	accrdb_len_pos = drda->out_pos;
	accrdb_len = ddm_write_accrdb(drda);
	accrdb_len += ddm_write_rdbacccl(drda);
	accrdb_len += ddm_write_crrtkn(drda);
	accrdb_len += ddm_write_rdbnam(drda,drda->database); 
	accrdb_len += ddm_write_prdid(drda); 
	accrdb_len += ddm_write_typdefnam(drda);
	accrdb_len += ddm_write_typdefovr(drda);
	rqsdss_len += accrdb_len;

	drda_put_int2(&drda->out_buf[accrdb_len_pos], accrdb_len);
	drda_put_int2(&drda->out_buf[rqsdss_len_pos], rqsdss_len);

	drda_dump_buf(stderr, drda->out_buf, rqsdss_len);
	write(drda->s,drda->out_buf,rqsdss_len);
	len = chain_read(drda, buf);

	return len;
}

/*
* Send a SQL query to the RDB
*/
int drda_excsqlimm(DRDA *drda, char *sql)
{
    int len;
    int tot_len = 0;
    int rqsdss_len = 0;
    int rqsdss_len_pos;
    int excsqlimm_len = 0;
    int excsqlimm_len_pos;
    unsigned char *buf;

	drda_buffer_init(drda);
	rqsdss_len_pos = drda->out_pos;
	rqsdss_len += ddm_write_rqsdss(drda, 0x51);

	buf = &drda->out_buf[drda->out_pos];

	excsqlimm_len_pos = drda->out_pos;
	excsqlimm_len = ddm_write_excsqlimm(drda);
	excsqlimm_len += ddm_write_pkgnamcsn(drda);
	excsqlimm_len += ddm_write_rdbcmtok(drda);
	rqsdss_len += excsqlimm_len;

	drda_put_int2(&drda->out_buf[excsqlimm_len_pos], excsqlimm_len);
	drda_put_int2(&drda->out_buf[rqsdss_len_pos], rqsdss_len);

	tot_len = rqsdss_len;
	
	rqsdss_len = 0;
	rqsdss_len_pos = drda->out_pos;
	rqsdss_len += ddm_write_rqsdss(drda, 0x03);
	rqsdss_len += ddm_write_sqlstt(drda,sql);
	drda_put_int2(&drda->out_buf[rqsdss_len_pos], rqsdss_len);
	tot_len += rqsdss_len;

	drda_dump_buf(stderr, drda->out_buf, tot_len);
	write(drda->s,drda->out_buf,tot_len);
	len = chain_read(drda, buf);

	return len;
}

/*
* Commit work
*/
int drda_rdbcmm(DRDA *drda)
{
    int len;
    int rqsdss_len = 0;
    int rqsdss_len_pos;
    unsigned char *buf;

	drda_buffer_init(drda);
	rqsdss_len_pos = drda->out_pos;
	rqsdss_len += ddm_write_rqsdss(drda, 0x01);

	buf = &drda->out_buf[drda->out_pos];

	rqsdss_len += ddm_write_rdbcmm(drda);

	drda_put_int2(&drda->out_buf[rqsdss_len_pos], rqsdss_len);

	drda_dump_buf(stderr, drda->out_buf, rqsdss_len);
	write(drda->s,drda->out_buf,rqsdss_len);
	len = chain_read(drda, buf);

	return len;
}

/*
* Bind a package
*/
int drda_bgnbnd(DRDA *drda)
{
    int len;
    int rqsdss_len = 0;
    int rqsdss_len_pos;
    int bgnbnd_len = 0;
    int bgnbnd_len_pos;
    unsigned char *buf;

	drda_buffer_init(drda);
	rqsdss_len_pos = drda->out_pos;
	rqsdss_len += ddm_write_rqsdss(drda, 0x01);

	buf = &drda->out_buf[drda->out_pos];

	bgnbnd_len_pos = drda->out_pos;
	bgnbnd_len = ddm_write_bgnbnd(drda);
	bgnbnd_len += ddm_write_pkgnamct(drda);
	bgnbnd_len += ddm_write_pkgisolvl(drda);

	rqsdss_len += bgnbnd_len;

	drda_put_int2(&drda->out_buf[bgnbnd_len_pos], bgnbnd_len);
	drda_put_int2(&drda->out_buf[rqsdss_len_pos], rqsdss_len);

	drda_dump_buf(stderr, drda->out_buf, rqsdss_len);
	write(drda->s,drda->out_buf,rqsdss_len);

	len = chain_read(drda,buf);
	drda_dump_buf(stderr, buf, len);

	return len;
}
int drda_endbnd(DRDA *drda)
{
    int len;
    int rqsdss_len = 0;
    int rqsdss_len_pos;
    int endbnd_len = 0;
    int endbnd_len_pos;
    unsigned char *buf;

	drda_buffer_init(drda);
	rqsdss_len_pos = drda->out_pos;
	rqsdss_len += ddm_write_rqsdss(drda, 0x01);

	buf = &drda->out_buf[drda->out_pos];

	endbnd_len_pos = drda->out_pos;
	endbnd_len = ddm_write_endbnd(drda);
	endbnd_len += ddm_write_pkgnamct(drda);

	rqsdss_len += endbnd_len;

	drda_put_int2(&drda->out_buf[endbnd_len_pos], endbnd_len);
	drda_put_int2(&drda->out_buf[rqsdss_len_pos], rqsdss_len);

	drda_dump_buf(stderr, drda->out_buf, rqsdss_len);
	write(drda->s,drda->out_buf,rqsdss_len);
	len = chain_read(drda, buf);

	return len;
}

/*
* Prepare a SQL query 
*/
int drda_prpsqlstt(DRDA *drda, char *sql)
{
    int tot_len = 0;
    int rqsdss_len = 0;
    int rqsdss_len_pos;
    int prpsqlstt_len = 0;
    int prpsqlstt_len_pos;
    unsigned char *buf;

	drda_buffer_init(drda);
	rqsdss_len_pos = drda->out_pos;
	rqsdss_len += ddm_write_rqsdss(drda, 0x51);

	buf = &drda->out_buf[drda->out_pos];

	prpsqlstt_len_pos = drda->out_pos;
	prpsqlstt_len = ddm_write_prpsqlstt(drda);
	prpsqlstt_len += ddm_write_pkgnamcsn(drda);
	prpsqlstt_len += ddm_write_rtnsqlda(drda);
	rqsdss_len += prpsqlstt_len;

	drda_put_int2(&drda->out_buf[prpsqlstt_len_pos], prpsqlstt_len);
	drda_put_int2(&drda->out_buf[rqsdss_len_pos], rqsdss_len);

	tot_len = rqsdss_len;
	
	rqsdss_len = 0;
	rqsdss_len_pos = drda->out_pos;
	rqsdss_len += ddm_write_rqsdss(drda, 0x03);
	rqsdss_len += ddm_write_sqlstt(drda,sql);
	drda_put_int2(&drda->out_buf[rqsdss_len_pos], rqsdss_len);
	tot_len += rqsdss_len;

	drda_dump_buf(stderr, drda->out_buf, tot_len);
	write(drda->s,drda->out_buf,tot_len);
	chain_read(drda, buf);
	
	if (drda->sqlca && drda->sqlca->sqlcode) {
		drda_log (0,stderr,"no sqlca or sqlcode.\n");
		return -1;
	}
	return tot_len;
}
/*
* Open a prepared SQL query 
*/
int drda_opnqry(DRDA *drda)
{
    int len;
//    int read_len;
//    int tot_len = 0;
    int rqsdss_len = 0;
    int rqsdss_len_pos;
    int opnqry_len = 0;
    int opnqry_len_pos;
    unsigned char *buf;
    /* move this somewhere else */
//    DRDA_COLUMN *col;

	drda_buffer_init(drda);
	rqsdss_len_pos = drda->out_pos;
	rqsdss_len += ddm_write_rqsdss(drda, 0x01);

	buf = &drda->out_buf[drda->out_pos];

	opnqry_len_pos = drda->out_pos;
	opnqry_len = ddm_write_opnqry(drda);
	opnqry_len += ddm_write_pkgnamcsn(drda);
	opnqry_len += ddm_write_qryblksz(drda,BUFSIZ);
	rqsdss_len += opnqry_len;

	drda_put_int2(&drda->out_buf[opnqry_len_pos], opnqry_len);
	drda_put_int2(&drda->out_buf[rqsdss_len_pos], rqsdss_len);

	drda_dump_buf(stderr, drda->out_buf, rqsdss_len);
	write(drda->s,drda->out_buf,rqsdss_len);
	
	len = chain_read(drda, buf);
	fprintf(stderr,"len = %d\n",len);

	return len;
}

/*
* Continue a SQL query 
*/
int drda_cntqry(DRDA *drda)
{
    int len;
//    int read_len;
//    int tot_len = 0;
    int rqsdss_len = 0;
    int rqsdss_len_pos;
    int cntqry_len = 0;
    int cntqry_len_pos;
    unsigned char *buf;
    /* move this somewhere else */
//    DRDA_COLUMN *col;

	drda_buffer_init(drda);
	rqsdss_len_pos = drda->out_pos;
	rqsdss_len += ddm_write_rqsdss(drda, 0x01);

	buf = &drda->out_buf[drda->out_pos];

	cntqry_len_pos = drda->out_pos;
	cntqry_len = ddm_write_cntqry(drda);
	cntqry_len += ddm_write_pkgnamcsn(drda);
	cntqry_len += ddm_write_qryblksz(drda,BUFSIZ);
	rqsdss_len += cntqry_len;

	drda_put_int2(&drda->out_buf[cntqry_len_pos], cntqry_len);
	drda_put_int2(&drda->out_buf[rqsdss_len_pos], rqsdss_len);

	drda_dump_buf(stderr, drda->out_buf, rqsdss_len);
	write(drda->s,drda->out_buf,rqsdss_len);
	
	len = chain_read(drda, buf);

	return len;
}

static int chain_read(DRDA *drda, unsigned char *buf)
{
    int len, dss_len;

	len = 0;
	while (1) {
		int  segment_len;
		char dssfmt;
		segment_len = read(drda->s,buf,BUFSIZ);
		len += segment_len;
		drda_log (0,stderr,"RPYDSS...\n");
		drda_dump_buf(stderr, buf, segment_len);
		dss_len = 0;
		do {
			dssfmt = buf[dss_len + 3];
			drda_log (0, stderr,"dssfmt %02x\n",dssfmt);
			dss_len += ddm_read_dss(drda, &buf[dss_len]);
			drda_log (0,stderr,"dss_len %d\n",dss_len);
		} while (dss_len < segment_len);

		if (dssfmt != 0x42 && dssfmt != 0x52)
			break;
	}
	return len;
}
