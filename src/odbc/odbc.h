/*
 * Copyright (C) 2001 Brian Bruns (camber@ais.org)
 */
#ifndef _odbc_h_
#define _odbc_h_

#include <drda.h>
#include <ddm.h>

#ifdef __cplusplus
extern "C" {
#endif

static char  rcsid_sql_h [ ] =
         "$Id: odbc.h,v 1.3 2002/03/20 12:38:03 brianb Exp $";
static void *no_unused_sql_h_warn[]={rcsid_sql_h, no_unused_sql_h_warn};

struct _henv {
	DRDA *drda;
};
struct _hdbc {
	struct _henv *henv;
};
struct _hstmt {
	struct _hdbc *hdbc;
	/* reminder to self: the following is here for testing purposes.
	 * please make dynamic before checking in 
	 */
	char query[4096];
	struct _sql_bind_info *bind_head;
	int rowcount;
};

struct _sql_bind_info {
	int column_number;
	int column_bindtype;
	int *column_lenbind;
	char *varaddr;
	int column_maxbind;
	struct _sql_bind_info *next;
};

#ifdef __cplusplus
}
#endif
#endif
