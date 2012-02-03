/* 
 * Copyright (C) 2001 Brian Bruns (camber@ais.org)
 */

#include "drda.h"
#include <iconv.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

void drda_buffer_init(DRDA *drda)
{
	drda->out_pos = 0;
    if(drda->out_buf == NULL) {
        drda->out_buf = malloc(4096);
        drda->out_size = 4096;
    }
}

DRDA_INT2 drda_get_int2(unsigned char *buf)
{
    DRDA_INT2 val;

	val = buf[0]*256 + buf[1];

	return val;
}

DRDA_INT4 drda_get_int4(unsigned char *buf)
{
    DRDA_INT4 val;

	val = buf[0];
	val <<= 8;
	val |= buf[1];
	val <<= 8;
	val |= buf[2];
	val <<= 8;
	val |= buf[3];

	return val;
}

DRDA_INT2 drda_get_endian_int2(DRDA *drda, unsigned char *buf)
{
    DRDA_INT2 val;

	if (!strcmp(drda->typdefnam, "QTDSQLASC")) {
		val = buf[0]*256 + buf[1];
	} else {
		val = buf[1]*256 + buf[0];
	}

	return val;
}

DRDA_INT4 drda_get_endian_int4(DRDA *drda, unsigned char *buf)
{
    DRDA_INT4 val;

	if (!strcmp(drda->typdefnam, "QTDSQLASC")) {
		val = buf[0];
		val <<= 8;
		val |= buf[1];
		val <<= 8;
		val |= buf[2];
		val <<= 8;
		val |= buf[3];
	} else {
		val = buf[3];
		val <<= 8;
		val |= buf[2];
		val <<= 8;
		val |= buf[1];
		val <<= 8;
		val |= buf[0];
	}
	return val;
}

DRDA_INT2 drda_get_leint2(unsigned char *buf)
{
    DRDA_INT2 val;

	val = buf[1]*256 + buf[0];

	return val;
}

DRDA_INT4 drda_get_leint4(unsigned char *buf)
{
    DRDA_INT4 val;

	val = buf[3];
	val <<= 8;
	val |= buf[2];
	val <<= 8;
	val |= buf[1];
	val <<= 8;
	val |= buf[0];

	return val;
}

void drda_put_int2(unsigned char *buf, DRDA_INT2 val)
{
	buf[0] = (val & 0xFF00) >> 8;
	buf[1] = (val & 0x00FF);
}

void drda_put_int4(unsigned char *buf, DRDA_INT2 val)
{
	buf[0] = (val & 0xFF000000) >> 24;
	buf[1] = (val & 0x00FF0000) >> 16;
	buf[2] = (val & 0x0000FF00) >> 8;
	buf[3] = (val & 0x000000FF);
}

char *drda_remote_string2local(DRDA *drda, char *in_buf, size_t in_bytes, char *out_buf)
{
    iconv_t cd;
    char *in_ptr, *out_ptr;
    size_t  out_bytes, orig_bytes;

	cd = iconv_open (drda->local_encoding, drda->remote_encoding);
    if(cd == (iconv_t)(-1)) {
        return NULL;
    } else {
        out_bytes = in_bytes + 1;
        orig_bytes = in_bytes + 1;
        in_ptr = in_buf;
        out_ptr = out_buf;
        iconv(cd, &in_ptr, &in_bytes, &out_ptr, &out_bytes);
        iconv_close(cd);
        out_buf[orig_bytes-1]='\0';
    }
    
	return out_buf;
}

char *drda_local_string2remote(DRDA *drda, char *in_buf, size_t in_bytes, char *out_buf)
{
    iconv_t cd;
    char *in_ptr, *out_ptr;
    size_t  out_bytes, orig_bytes;

	cd = iconv_open (drda->remote_encoding, drda->local_encoding);
    if(cd == (iconv_t)(-1)) {
        return NULL;
    } else {
        out_bytes = in_bytes + 1;
        orig_bytes = in_bytes + 1;
        in_ptr = in_buf;
        out_ptr = out_buf;
        iconv(cd, &in_ptr, &in_bytes, &out_ptr, &out_bytes);
        iconv_close(cd);
        out_buf[orig_bytes-1]='\0';
    }
    
	return out_buf;
}

void drda_local_string2remote_pad(DRDA *drda, char *in_buf, size_t out_bytes, char *out_buf)
{
    iconv_t cd;
    size_t in_bytes;
    const char  *p_in_buf;
    const char **p_p_in_buf;
    char        *cpy_out_buf;
    size_t cpy_in_bytes, cpy_out_bytes;
    int rc;

    in_bytes = strlen(in_buf);
	cd = iconv_open (drda->remote_encoding, drda->local_encoding);
    if(cd == (iconv_t)(-1)) {
    } else {
        p_in_buf      = in_buf;
        p_p_in_buf    = &p_in_buf;
        /* I (tje, 2001-08-06) do not understand why I cannot do
            this within the call to iconv.) */
        cpy_out_buf   = out_buf;
        cpy_in_bytes  = in_bytes;
        cpy_out_bytes = out_bytes;

        if (in_bytes<out_bytes) {
            rc = iconv (cd, (char**)p_p_in_buf,   &cpy_in_bytes,
                &cpy_out_buf, &cpy_out_bytes);
            // fprintf(stderr,"rc %d, in_bytes %d\n",rc,in_bytes);
            // assert (rc == in_bytes);
            memset (out_buf+in_bytes, 0x40, out_bytes-in_bytes);
        } else {
            size_t cpy_out_bytes_2 = cpy_out_bytes;
            rc = iconv (cd, (char**)p_p_in_buf,   &cpy_out_bytes_2,
                &cpy_out_buf, &cpy_out_bytes);
            // assert (rc == in_bytes);
        };

        iconv_close (cd);
    }
    
	return;
} // drda_local_string2remote_pad
