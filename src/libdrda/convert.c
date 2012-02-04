/* 
 * Copyright (C) 2001 Brian Bruns (camber@ais.org)
 */

/*
** This file contains code to support the binding and retrieval of data from 
** one data type to another.
*/
#include "drda.h"
#include <stdlib.h>
#include <string.h>

static int drda_convert_char(DRDA *drda, 
					unsigned char *src, int srctype, int srclen,
					unsigned char *dest, int desttype, int destlen);
static int drda_convert_smallint(DRDA *drda, 
					unsigned char *src, int srctype, int srclen,
					unsigned char *dest, int desttype, int destlen);
static int drda_convert_int(DRDA *drda, 
					unsigned char *src, int srctype, int srclen,
					unsigned char *dest, int desttype, int destlen);

char 
drda_get_sign(unsigned char nybble)
{
	if (nybble == 0x0b || nybble == 0x0d)
		return '-';
	if (nybble == 0x0c ||  /* 0x0c seems to be preferred so list first */
		nybble == 0x0a ||
		nybble == 0x0e ||
		nybble == 0x0f) 
		return '+';
	return '?'; /* invalid */
}
int 
drda_get_cardinal_type(int fdoca_type)
{
	switch (fdoca_type) {
		case 0x023:
			return DRDA_TIME;
			break;
		case 0x021:
			return DRDA_DATE;
			break;
		case 0x00f:	
			return DRDA_DECIMAL; 
			break;
		case 0x030:	
		case 0x031:	
		case 0x032:	
		case 0x033:	
			return DRDA_CHAR;
			break;
		case 0x005:	
		case 0x004:	
			return DRDA_SMALLINT;
			break;
		case 0x003:	
		case 0x002:	
			return DRDA_INT;
			break;
		default:
			fprintf(stderr,"Type 0x%02x (%d) not supported, aborting\n",fdoca_type, fdoca_type);
			return 0;
	}
	return 0;
}

int 
drda_set_null_value(DRDA *drda, unsigned char *dest, int desttype)
{
	if (!dest) return 0;

	switch (desttype) {
		case DRDA_CHAR:
		case DRDA_VARCHAR:
			/* for character types, set empty string */
			((char *)dest)[0]='\0';
			return 1;
		break;
	}
	return 0;
}

int drda_convert(DRDA *drda, unsigned char *src, int srctype, int srclen, 
					unsigned char *dest, int desttype, int destlen)
{
	//fprintf(stderr, "Converting from %d to %d\n", srctype, desttype);
	switch (srctype) {
		case DRDA_TIME:
			return drda_convert_char(drda, src, srctype, 8, dest, desttype, destlen);
		case DRDA_DATE:
			return drda_convert_char(drda, src, srctype, 10, dest, desttype, destlen);
		case DRDA_CHAR:
		case DRDA_VARCHAR:
			return drda_convert_char(drda, src, srctype, srclen, dest, desttype, destlen);
		case DRDA_SMALLINT:
			return drda_convert_smallint(drda, src, srctype, srclen, dest, desttype, destlen);
		case DRDA_INT:
			return drda_convert_int(drda, src, srctype, srclen, dest, desttype, destlen);
	}
    
    return -1;
}

int drda_convert_precision(DRDA *drda, unsigned char *src, int srctype, 
					int srcprec, int srcscale,
					unsigned char *dest, int desttype, 
					int destlen)
{
    int i, j;
    unsigned char *buf;

	switch (desttype) {
		case DRDA_CHAR:
		case DRDA_VARCHAR:
			buf = (unsigned char *) malloc(srcprec + 2);
			j=0;
			for (i=0;i<srcprec;i++) {
				buf[j++] = i%2 ? 
					(src[i/2] & 0x0F) + 0x30 :
					((src[i/2] & 0xF0) >> 4) + 0x30; 
				if ((srcprec - srcscale - 1) == i)
					buf[j++] = '.';
			}
			buf[srcprec+1]='\0';
			/* clean off the zeros */
			for (j=0;buf[j]=='0';j++);
			strncpy((char*)dest, (char*)&buf[j], destlen);
			dest[destlen]='\0';
			free(buf);
		break;
	}
	return strlen((char*)dest);
}

static int drda_convert_char(DRDA *drda, 
	unsigned char *src, int srctype, int srclen, 
	unsigned char *dest, int desttype, int destlen)
{
	switch (desttype) {
		case DRDA_CHAR:
		case DRDA_VARCHAR:
			strncpy((char*)dest, (char*)src, destlen);
			dest[destlen]='\0';
		break;
	}
	return strlen((char*)dest);
}
static int drda_convert_smallint(DRDA *drda, 
	unsigned char *src, int srctype, int srclen, 
	unsigned char *dest, int desttype, int destlen)
{
    DRDA_INT2 val;

	val = drda_get_endian_int2(drda, src);

	switch(desttype) {
		case DRDA_CHAR:
		case DRDA_VARCHAR:
			sprintf((char*)dest, "%d", val); 
			// fprintf(stderr, "Converted %d %s\n", val, dest);
			return strlen((char*)dest);
		break;
	}
	return 0;
}
static int drda_convert_int(DRDA *drda, 
	unsigned char *src, int srctype, int srclen, 
	unsigned char *dest, int desttype, int destlen)
{
    DRDA_INT4 val;

	val = drda_get_endian_int2(drda, src);

	switch(desttype) {
		case DRDA_CHAR:
		case DRDA_VARCHAR:
			sprintf((char*)dest, "%ld", val); 
			// fprintf(stderr, "Converted %d %s\n", val, dest);
			return strlen((char*)dest);
		break;
	}
	return 0;
}
