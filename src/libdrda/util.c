/* 
 * Copyright (C) 2001 Brian Bruns (camber@ais.org)
 */

#include "drda.h"
#include <iconv.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

// ASCII to EBCDIC tables stolen from http://cprogramminglanguage.net/ascii-ebcdic-conversion-functions.aspx

static unsigned char a2e[256] = {
    0,  1,  2,  3, 55, 45, 46, 47, 22,  5, 37, 11, 12, 13, 14, 15,
    16, 17, 18, 19, 60, 61, 50, 38, 24, 25, 63, 39, 28, 29, 30, 31,
    64, 79,127,123, 91,108, 80,125, 77, 93, 92, 78,107, 96, 75, 97,
    240,241,242,243,244,245,246,247,248,249,122, 94, 76,126,110,111,
    124,193,194,195,196,197,198,199,200,201,209,210,211,212,213,214,
    215,216,217,226,227,228,229,230,231,232,233, 74,224, 90, 95,109,
    121,129,130,131,132,133,134,135,136,137,145,146,147,148,149,150,
    151,152,153,162,163,164,165,166,167,168,169,192,106,208,161,  7,
    32, 33, 34, 35, 36, 21,  6, 23, 40, 41, 42, 43, 44,  9, 10, 27,
    48, 49, 26, 51, 52, 53, 54,  8, 56, 57, 58, 59,  4, 20, 62,225,
    65, 66, 67, 68, 69, 70, 71, 72, 73, 81, 82, 83, 84, 85, 86, 87,
    88, 89, 98, 99,100,101,102,103,104,105,112,113,114,115,116,117,
    118,119,120,128,138,139,140,141,142,143,144,154,155,156,157,158,
    159,160,170,171,172,173,174,175,176,177,178,179,180,181,182,183,
    184,185,186,187,188,189,190,191,202,203,204,205,206,207,218,219,
    220,221,222,223,234,235,236,237,238,239,250,251,252,253,254,255
};

static unsigned char e2a[256] = {
    0,  1,  2,  3,156,  9,134,127,151,141,142, 11, 12, 13, 14, 15,
    16, 17, 18, 19,157,133,  8,135, 24, 25,146,143, 28, 29, 30, 31,
    128,129,130,131,132, 10, 23, 27,136,137,138,139,140,  5,  6,  7,
    144,145, 22,147,148,149,150,  4,152,153,154,155, 20, 21,158, 26,
    32,160,161,162,163,164,165,166,167,168, 91, 46, 60, 40, 43, 33,
    38,169,170,171,172,173,174,175,176,177, 93, 36, 42, 41, 59, 94,
    45, 47,178,179,180,181,182,183,184,185,124, 44, 37, 95, 62, 63,
    186,187,188,189,190,191,192,193,194, 96, 58, 35, 64, 39, 61, 34,
    195, 97, 98, 99,100,101,102,103,104,105,196,197,198,199,200,201,
    202,106,107,108,109,110,111,112,113,114,203,204,205,206,207,208,
    209,126,115,116,117,118,119,120,121,122,210,211,212,213,214,215,
    216,217,218,219,220,221,222,223,224,225,226,227,228,229,230,231,
    123, 65, 66, 67, 68, 69, 70, 71, 72, 73,232,233,234,235,236,237,
    125, 74, 75, 76, 77, 78, 79, 80, 81, 82,238,239,240,241,242,243,
    92,159, 83, 84, 85, 86, 87, 88, 89, 90,244,245,246,247,248,249,
    48, 49, 50, 51, 52, 53, 54, 55, 56, 57,250,251,252,253,254,255
};

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
        // OSX doesn't seem to have EBCDIC compiled in to its version of iconv
        int i;
        for(i=0; i<in_bytes; i++) {
            out_buf[i] = e2a[in_buf[i]];
        }
        out_buf[in_bytes+1] = '\0';
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
        // OSX doesn't seem to have EBCDIC compiled in to its version of iconv
        int i;
        for(i=0; i<in_bytes; i++) {
            out_buf[i] = a2e[in_buf[i]];
        }
        out_buf[in_bytes+1] = '\0';
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
        int i;        
        memset(out_buf, 0x40, out_bytes);
        if(out_bytes<in_bytes) {
            in_bytes = out_bytes;
        }
        for(i=0; i<in_bytes; i++) {
            out_buf[i] = a2e[in_buf[i]];
        }
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
