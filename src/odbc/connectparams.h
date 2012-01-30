/*
 * Copyright (C) 2001 Brian Bruns (camber@ais.org)
 */
#ifndef _ODBC_INI_LOAD_
#define _ODBC_INI_LOAD_

#include <glib.h>

typedef struct
{
   GString* dsnName;
   GString* iniFileName;
   GHashTable* table;
} ConnectParams;

ConnectParams* NewConnectParams ();
void FreeConnectParams (ConnectParams* params);

gboolean LookupDSN        (ConnectParams* params, const gchar* dsnName);
gchar*   GetConnectParam  (ConnectParams* params, const gchar* paramName);
void     SetConnectString (ConnectParams* params, const gchar* connectString);
void     DumpParams       (ConnectParams* params, FILE* output);

gchar*   ExtractDSN (ConnectParams* params, const gchar* connectString);

#endif
