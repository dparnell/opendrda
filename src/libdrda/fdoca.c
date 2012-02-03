/* 
 * Copyright (C) 2001 Brian Bruns (camber@ais.org)
 */

#include "drda.h"
#include <stdlib.h>

int fdoca_is_nullable(int type)
{
	/* FIX ME - not complete */
	switch (type) {
		/* case 0x30: */
		case 0x21:
		case 0x23:
		case 0x31:
		case 0x32:
		case 0x33:
		case 0x05:
		case 0x03:
		case 0x0f:
			return 1;
		default:
			return 0;
	}
}

int fdoca_sizeof_length(int type)
{
	switch (type) {
		case 0x33:
			return 2;
		case 0x32:
			return 1;
		default:
			return 0; /* fixed length field */
	}
}

int fdoca_read_sqlca(DRDA *drda, unsigned char *buf)
{
DRDA_SQLCA *sqlca;

	sqlca = drda_alloc_sqlca(drda);
	sqlca->sqlcode = drda_get_int4(&buf[1]);
	memcpy(sqlca->sqlstate,&buf[5],5);
	memcpy(sqlca->sqlerrproc,&buf[10],8);
	memcpy(sqlca->sqlrdbnam,&buf[19],18);
	sqlca->sqlerrd1 = drda_get_int4(&buf[37]);
	sqlca->sqlerrd2 = drda_get_int4(&buf[41]);
	sqlca->sqlerrd3 = drda_get_int4(&buf[45]);
	sqlca->sqlerrd4 = drda_get_int4(&buf[49]);
	sqlca->sqlerrd5 = drda_get_int4(&buf[53]);
	sqlca->sqlerrd6 = drda_get_int4(&buf[57]);
	sqlca->sqlwarn0 = buf[61];
	sqlca->sqlwarn1 = buf[62];
	sqlca->sqlwarn2 = buf[63];
	sqlca->sqlwarn3 = buf[64];
	sqlca->sqlwarn4 = buf[65];
	sqlca->sqlwarn5 = buf[66];
	sqlca->sqlwarn6 = buf[67];
	sqlca->sqlwarn7 = buf[68];
	sqlca->sqlwarn8 = buf[69];
	sqlca->sqlwarn9 = buf[70];
	sqlca->sqlwarna = buf[71];

	drda_log(0,stderr,"Err Proc: %s\n", sqlca->sqlerrproc);
	drda_log(0,stderr,"RDB Name: %s\n", sqlca->sqlrdbnam);

	if (drda->sqlca) 
		drda_free_sqlca(drda->sqlca);
	drda->sqlca = sqlca;

	return 76;
}

int fdoca_read_sqlda(DRDA *drda, unsigned char *buf)
{
    DRDA_SQLDA *sqlda;
    int num_cols;
    int pos, i;
    int str_len;

	sqlda = drda_alloc_sqlda(drda);
	/* This is in servers machine order ?! */
	num_cols = drda_get_endian_int2(drda, buf);
	drda_alloc_columns(sqlda, num_cols);
	pos = 2;
	drda_log(0,stderr,"Number of Columns: %d\n", sqlda->num_cols);
	for (i=0;i<num_cols;i++) {
		sqlda->columns[i]->precision = drda_get_endian_int2(drda,&buf[pos]);
		pos += 2;
		sqlda->columns[i]->scale = drda_get_endian_int2(drda,&buf[pos]);
		pos += 2;
		sqlda->columns[i]->length = drda_get_endian_int4(drda,&buf[pos]);
		pos += 4;
		sqlda->columns[i]->type = drda_get_endian_int2(drda,&buf[pos]);
		pos += 2;
		sqlda->columns[i]->ccsid = drda_get_int2(&buf[pos]);
		pos += 2;
		drda_log(0,stderr,"Column length: %d precision: %d scale: %d\n", sqlda->columns[i]->length, sqlda->columns[i]->precision, sqlda->columns[i]->scale);
		/* SQLNAME_m */
		str_len = drda_get_int2(&buf[pos]);
		pos += 2;
		if (str_len) {
			fprintf(stderr,"multibyte characters not supported yet!\n");
			pos += str_len;
		}
		/* SQLNAME_s */
		str_len = drda_get_int2(&buf[pos]);
		pos += 2;
		if (str_len) {
			sqlda->columns[i]->name = (char *) malloc(str_len) + 1;
			memcpy(sqlda->columns[i]->name, &buf[pos], str_len);
			sqlda->columns[i]->name[str_len] = '\0';
			pos += str_len;
		}
		/* SQLLABEL_m */
		str_len = drda_get_int2(&buf[pos]);
		pos += 2;
		if (str_len) {
			fprintf(stderr,"multibyte characters not supported yet!\n");
			pos += str_len;
		}
		/* SQLLABEL_s */
		str_len = drda_get_int2(&buf[pos]);
		pos += 2;
		if (str_len) {
			sqlda->columns[i]->label = (char *) malloc(str_len) + 1;
			memcpy(sqlda->columns[i]->label, &buf[pos], str_len);
			sqlda->columns[i]->label[str_len] = '\0';
			pos += str_len;
		}
		/* SQLCOMMENTS_m */
		str_len = drda_get_int2(&buf[pos]);
		pos += 2;
		if (str_len) {
			fprintf(stderr,"multibyte characters not supported yet!\n");
			pos += str_len;
		}
		/* SQLCOMMENTS_s */
		str_len = drda_get_int2(&buf[pos]);
		pos += 2;
		if (str_len) {
			sqlda->columns[i]->comments = (char *) malloc(str_len) + 1;
			memcpy(sqlda->columns[i]->comments, &buf[pos], str_len);
			sqlda->columns[i]->comments[str_len] = '\0';
			pos += str_len;
		}
		drda_log(0,stderr,"Column Name: %s\n", sqlda->columns[i]->name);
	}
	drda->sqlda = sqlda;

	return 0;
}
/*
** fdoca_read_qrydsc:  DRDA uses a profile of FDOCA that contains a GDA
** containing a SQLCA and then triplets containing type, repeat, and length
** for each column.  DB2 seems not to use the repeat count, but I don't know
** if this is prohibited by the spec or not, need to research it more.
** Anyway, the following code does *not* account for a non-zero repeat count.
*/
int fdoca_read_qrydsc(DRDA *drda, char *buf)
{
DRDA_SQLDA *sqlda;
int num_cols;
int coln;
DRDA_COLUMN *col;
int len, start;

	sqlda = drda->sqlda;

	len = buf[0];
	/* skip the SQLCA definition */
	start = 3;
	coln = 0;
	while (start < len) {
		if (coln < sqlda->num_cols) {
			col = sqlda->columns[coln];
			col->fdoca_type = buf[start];
			col->fdoca_len = buf[start+2];
/*
			fprintf(stderr,"Name: %s Type: %02x Length: %d\n", col->name,
				col->fdoca_type, col->fdoca_len);
*/
		} else {
			fprintf(stderr,"Number of columns in QRYDSC exceeds number in SQLDA!\n");
		}
		start += 3;
		coln++;
	}
}

int fdoca_read_qrydta(DRDA *drda, unsigned char *buf, int len)
{
DRDA_SQLDA *sqlda;
int pos;

	pos = 0;

	if (drda->result_buf)
		free(drda->result_buf);
	drda->result_len = len;
	drda->result_buf = (unsigned char *) malloc(len);
	memcpy(drda->result_buf, buf, len);
	return len;
/*
	while (pos<len) {
		pos += fdoca_read_qrydta_row(drda, buf, len, pos);
	}
*/
}
int fdoca_read_qrydta_row(DRDA *drda, unsigned char *buf, int len, int start)
{
int pos = start;
DRDA_SQLDA *sqlda;
int num_cols;
int coln, dlen;
DRDA_COLUMN *col;
int szbytes, flen;
unsigned char *tmpbuf;

	sqlda = drda->sqlda;

	if (buf[pos]!=0xff) {
		fprintf(stderr,"Non-NULL SQLCA encountered, code does not yet support this!\n");
		return 0;
	}
	/* second byte should be 0x00 */
	pos+=2;

	for (coln=0;coln<sqlda->num_cols;coln++) {
		// fprintf(stderr,"pos %d\n", pos); 
		col = sqlda->columns[coln];
		//fprintf(stderr,"fdoca type is %d %d\n", col->fdoca_type, fdoca_is_nullable(col->fdoca_type)); 
		if (fdoca_is_nullable(col->fdoca_type)) {
			if (buf[pos]==0xff) {
				// fprintf(stderr,"NULL!!!!!!!!!\n");
				col->cur_length=0;
				/* set bound address to empty string */
				/* FIX ME - should be bind type */
				if (col->bind_addr) {
					/* if unsupported type, error and stop */
					if (!drda_get_cardinal_type(col->fdoca_type)) {
						fprintf(stderr,"Error - Column was %s\n",col->name);
						exit(1);
					}
					drda_set_null_value(drda, col->bind_addr, DRDA_VARCHAR);
				}	
				/* ((char *)col->bind_addr)[0]='\0'; */
				pos++;
				continue;
			}
			pos++;
		}

		szbytes = fdoca_sizeof_length(col->fdoca_type);
		if (szbytes==1) {
			flen = buf[pos];
		} else if (szbytes==2) {
			flen = drda_get_int2(&buf[pos]);
		} else if (szbytes==0) {
			if (col->precision) {
				flen = (col->precision + 1) / 2;
			} else {
				flen = col->fdoca_len;
			}
		} else {
			fprintf(stderr,"code does not yet support length size of %d!\n",flen);
			return;
		}
		pos += szbytes;
		// fprintf(stderr,"flen = %d\n", col->fdoca_len);

		col->cur_length=flen;
		col->cur_addr=&buf[pos];

		if (col->bind_addr) {
			/* if unsupported type, error and stop */
			if (!drda_get_cardinal_type(col->fdoca_type)) {
				fprintf(stderr,"Error - Column was %s\n",col->name);
				exit(1);
			}
			if (col->precision) {
				dlen = drda_convert_precision(drda, &buf[pos], drda_get_cardinal_type(col->fdoca_type), col->precision, col->scale, col->bind_addr, DRDA_VARCHAR, col->precision+1);
			} else {
				dlen = drda_convert(drda, &buf[pos], drda_get_cardinal_type(col->fdoca_type), flen, col->bind_addr, DRDA_VARCHAR, flen);
			}
			if (col->len_bind_addr) {
				*col->len_bind_addr = dlen;
			}
/*
			tmpbuf = (char *) col->bind_addr;
			strncpy(tmpbuf, &buf[pos], flen);
			tmpbuf[flen]='\0';
*/
			/* fprintf(stderr,"%s\t",tmpbuf); */
		}
		pos += flen;
	}
	/* fprintf(stderr,"\n"); */

	return pos - start;
}
