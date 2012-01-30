/* 
 * Copyright (C) 2001 Brian Bruns (camber@ais.org)
 */



#ifndef _drdatypes_h_
#define _drdatypes_h_

#ifdef __cplusplus
extern "C" {
#endif

/* list of cardinal types */
enum {
	DRDA_CHAR = 1,
	DRDA_VARCHAR,
	DRDA_SMALLINT,
	DRDA_INT,
	DRDA_TIME,
	DRDA_DATE,
	DRDA_TIMESTAMP,
	DRDA_BLOB,
	DRDA_CLOB,
	DRDA_DECIMAL
};

#ifdef __cplusplus
}
#endif

#endif
