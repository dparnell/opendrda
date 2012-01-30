/*
 * Copyright (C) 2001 Brian Bruns (camber@ais.org)
 */

#ifndef UNIXODBC
#include <sql.h>
#include <sqlext.h>
#else
#include "isql.h"
#include "isqlext.h"
#endif

#include <odbc.h>

#include <string.h>
#include <stdio.h>

#include "connectparams.h"

static char  software_version[]   = "$Id: odbc.c,v 1.7 2002/12/27 04:43:20 brianb Exp $";
static void *no_unused_var_warn[] = {software_version,
                                     no_unused_var_warn};

static SQLSMALLINT _odbc_get_client_type(int srv_type);
static int _odbc_fix_literals(struct _hstmt *stmt);
static int _odbc_get_server_type(int clt_type);
static int _odbc_get_string_size(int size, char *str);
static SQLRETURN SQL_API _SQLAllocConnect(SQLHENV henv, SQLHDBC FAR *phdbc);
static SQLRETURN SQL_API _SQLAllocEnv(SQLHENV FAR *phenv);
static SQLRETURN SQL_API _SQLAllocStmt(SQLHDBC hdbc, SQLHSTMT FAR *phstmt);
static SQLRETURN SQL_API _SQLFreeConnect(SQLHDBC hdbc);
static SQLRETURN SQL_API _SQLFreeEnv(SQLHENV henv);
static SQLRETURN SQL_API _SQLFreeStmt(SQLHSTMT hstmt, SQLUSMALLINT fOption);

#define MIN(a,b) (a>b ? b : a)
#define _MAX_ERROR_LEN 255
static char lastError[_MAX_ERROR_LEN+1];
unsigned bound_data[256][256];

//#define TRACE(x) fprintf(stderr,"Function %s\n", x);
#define TRACE(x) 

/* The SQL engine is presently non-reenterrant and non-thread safe.  
   See _SQLExecute for details.
*/

static void LogError (const char* error)
{
   /*
    * Someday, I might make this store more than one error.
    */
   strncpy (lastError, error, _MAX_ERROR_LEN);
   lastError[_MAX_ERROR_LEN] = '\0'; /* in case we had a long message */
}

/*
 * Driver specific connectionn information
 */

typedef struct
{
	struct _hdbc hdbc;
	ConnectParams* params;
} ODBCConnection;

static SQLRETURN do_connect (
   SQLHDBC hdbc,
   SQLCHAR FAR *url,
   SQLCHAR FAR *uid,
   SQLCHAR FAR *passwd
)
{
struct _hdbc *dbc = (struct _hdbc *) hdbc;
struct _henv *env = (struct _henv *) dbc->henv;

	TRACE("do_connect");
	drda_set_by_url(env->drda, url);
	if (uid && strlen(uid)) {
		drda_set_user(env->drda, uid);
	}

	if (passwd && strlen(passwd)) {
		drda_set_password(env->drda, passwd);
	}

	if (!drda_connect(env->drda)) {
		LogError ("Failed to connect to server.");
		return SQL_ERROR;
	}
	if (drda_excsat(env->drda)<0) {
		LogError ("Failed to exchange attributes with server.");
		return SQL_ERROR;
     }
     if (drda_accsec(env->drda)<0) {
		LogError ("Failed to negotiate security with server.");
		return SQL_ERROR;
     }
     if (drda_secchk(env->drda)<0) {
		LogError ("Failed to logon to server.");
		return SQL_ERROR;
     }
     if (drda_accrdb(env->drda)<0) {
		LogError ("Failed to access database.");
		return SQL_ERROR;
     }
	return SQL_SUCCESS;
}

SQLRETURN SQL_API SQLDriverConnect(
    SQLHDBC            hdbc,
    SQLHWND            hwnd,
    SQLCHAR FAR       *szConnStrIn,
    SQLSMALLINT        cbConnStrIn,
    SQLCHAR FAR       *szConnStrOut,
    SQLSMALLINT        cbConnStrOutMax,
    SQLSMALLINT FAR   *pcbConnStrOut,
    SQLUSMALLINT       fDriverCompletion)
{
SQLCHAR FAR* dsn = NULL;
ConnectParams* params;
SQLRETURN ret;
SQLCHAR FAR* url = NULL;
SQLCHAR FAR* uid = NULL;
SQLCHAR FAR* passwd = NULL;


	TRACE("SQLDriverConnect");
	strcpy (lastError, "");

	params = ((ODBCConnection*) hdbc)->params;

	if (!(dsn = ExtractDSN (params, szConnStrIn))) {
		LogError ("Could not find DSN in connect string");
		return SQL_ERROR;
	} else if (!LookupDSN (params, dsn)) {
		LogError ("Could not find DSN in odbc.ini");
		return SQL_ERROR;
	} else {
		SetConnectString (params, szConnStrIn);

		if (!(url = GetConnectParam (params, "URL"))) {
			LogError ("Could not find URL parameter");
			return SQL_ERROR;
		}
		if (!(url = GetConnectParam (params, "UID"))) {
			LogError ("Could not find UID parameter");
			return SQL_ERROR;
		}
		if (!(passwd = GetConnectParam (params, "PWD"))) {
			LogError ("Could not find UID parameter");
			return SQL_ERROR;
		}
	}
	ret = do_connect (hdbc, url, uid, passwd);
	return ret;
}

SQLRETURN SQL_API SQLBrowseConnect(
    SQLHDBC            hdbc,
    SQLCHAR FAR       *szConnStrIn,
    SQLSMALLINT        cbConnStrIn,
    SQLCHAR FAR       *szConnStrOut,
    SQLSMALLINT        cbConnStrOutMax,
    SQLSMALLINT FAR   *pcbConnStrOut)
{
	TRACE("SQLBrowseConnect");
	return SQL_SUCCESS;
}

SQLRETURN SQL_API SQLColumnPrivileges(
    SQLHSTMT           hstmt,
    SQLCHAR FAR       *szCatalogName,
    SQLSMALLINT        cbCatalogName,
    SQLCHAR FAR       *szSchemaName,
    SQLSMALLINT        cbSchemaName,
    SQLCHAR FAR       *szTableName,
    SQLSMALLINT        cbTableName,
    SQLCHAR FAR       *szColumnName,
    SQLSMALLINT        cbColumnName)
{
	TRACE("SQLColumnPriviledges");
	return SQL_SUCCESS;
}

SQLRETURN SQL_API SQLDescribeParam(
    SQLHSTMT           hstmt,
    SQLUSMALLINT       ipar,
    SQLSMALLINT FAR   *pfSqlType,
    SQLUINTEGER FAR   *pcbParamDef,
    SQLSMALLINT FAR   *pibScale,
    SQLSMALLINT FAR   *pfNullable)
{
	TRACE("SQLDescribeParam");
	return SQL_SUCCESS;
}

SQLRETURN SQL_API SQLExtendedFetch(
    SQLHSTMT           hstmt,
    SQLUSMALLINT       fFetchType,
    SQLINTEGER         irow,
    SQLUINTEGER FAR   *pcrow,
    SQLUSMALLINT FAR  *rgfRowStatus)
{
	TRACE("SQLExtendedFetch");
	return SQL_SUCCESS;
}

SQLRETURN SQL_API SQLForeignKeys(
    SQLHSTMT           hstmt,
    SQLCHAR FAR       *szPkCatalogName,
    SQLSMALLINT        cbPkCatalogName,
    SQLCHAR FAR       *szPkSchemaName,
    SQLSMALLINT        cbPkSchemaName,
    SQLCHAR FAR       *szPkTableName,
    SQLSMALLINT        cbPkTableName,
    SQLCHAR FAR       *szFkCatalogName,
    SQLSMALLINT        cbFkCatalogName,
    SQLCHAR FAR       *szFkSchemaName,
    SQLSMALLINT        cbFkSchemaName,
    SQLCHAR FAR       *szFkTableName,
    SQLSMALLINT        cbFkTableName)
{
	TRACE("SQLForeignKeys");
	return SQL_SUCCESS;
}

SQLRETURN SQL_API SQLMoreResults(
    SQLHSTMT           hstmt)
{
	TRACE("SQLMoreResults");
	return SQL_SUCCESS;
}

SQLRETURN SQL_API SQLNativeSql(
    SQLHDBC            hdbc,
    SQLCHAR FAR       *szSqlStrIn,
    SQLINTEGER         cbSqlStrIn,
    SQLCHAR FAR       *szSqlStr,
    SQLINTEGER         cbSqlStrMax,
    SQLINTEGER FAR    *pcbSqlStr)
{
	TRACE("SQLNativeSql");
	return SQL_SUCCESS;
}

SQLRETURN SQL_API SQLNumParams(
    SQLHSTMT           hstmt,
    SQLSMALLINT FAR   *pcpar)
{
	TRACE("SQLNumParams");
	return SQL_SUCCESS;
}

SQLRETURN SQL_API SQLParamOptions(
    SQLHSTMT           hstmt,
    SQLUINTEGER        crow,
    SQLUINTEGER FAR   *pirow)
{
	TRACE("SQLParamOptions");
	return SQL_SUCCESS;
}

SQLRETURN SQL_API SQLPrimaryKeys(
    SQLHSTMT           hstmt,
    SQLCHAR FAR       *szCatalogName,
    SQLSMALLINT        cbCatalogName,
    SQLCHAR FAR       *szSchemaName,
    SQLSMALLINT        cbSchemaName,
    SQLCHAR FAR       *szTableName,
    SQLSMALLINT        cbTableName)
{
	TRACE("SQLPrimaryKeys");
	return SQL_SUCCESS;
}

SQLRETURN SQL_API SQLProcedureColumns(
    SQLHSTMT           hstmt,
    SQLCHAR FAR       *szCatalogName,
    SQLSMALLINT        cbCatalogName,
    SQLCHAR FAR       *szSchemaName,
    SQLSMALLINT        cbSchemaName,
    SQLCHAR FAR       *szProcName,
    SQLSMALLINT        cbProcName,
    SQLCHAR FAR       *szColumnName,
    SQLSMALLINT        cbColumnName)
{
	TRACE("SQLProcedureColumns");
	return SQL_SUCCESS;
}

SQLRETURN SQL_API SQLProcedures(
    SQLHSTMT           hstmt,
    SQLCHAR FAR       *szCatalogName,
    SQLSMALLINT        cbCatalogName,
    SQLCHAR FAR       *szSchemaName,
    SQLSMALLINT        cbSchemaName,
    SQLCHAR FAR       *szProcName,
    SQLSMALLINT        cbProcName)
{
	TRACE("SQLProcedures");
	return SQL_SUCCESS;
}

SQLRETURN SQL_API SQLSetPos(
    SQLHSTMT           hstmt,
    SQLUSMALLINT       irow,
    SQLUSMALLINT       fOption,
    SQLUSMALLINT       fLock)
{
	TRACE("SQLSetPos");
	return SQL_SUCCESS;
}

SQLRETURN SQL_API SQLTablePrivileges(
    SQLHSTMT           hstmt,
    SQLCHAR FAR       *szCatalogName,
    SQLSMALLINT        cbCatalogName,
    SQLCHAR FAR       *szSchemaName,
    SQLSMALLINT        cbSchemaName,
    SQLCHAR FAR       *szTableName,
    SQLSMALLINT        cbTableName)
{
	TRACE("SQLTablePrivleges");
	return SQL_SUCCESS;
}

SQLRETURN SQL_API SQLDrivers(
    SQLHENV            henv,
    SQLUSMALLINT       fDirection,
    SQLCHAR FAR       *szDriverDesc,
    SQLSMALLINT        cbDriverDescMax,
    SQLSMALLINT FAR   *pcbDriverDesc,
    SQLCHAR FAR       *szDriverAttributes,
    SQLSMALLINT        cbDrvrAttrMax,
    SQLSMALLINT FAR   *pcbDrvrAttr)
{
	TRACE("SQLDrivers");
	return SQL_SUCCESS;
}
SQLRETURN SQL_API SQLSetEnvAttr (
    SQLHENV EnvironmentHandle,
    SQLINTEGER Attribute,
    SQLPOINTER Value,
    SQLINTEGER StringLength)
{
	TRACE("SQLSetEnvAttr");
	return SQL_SUCCESS;
}


SQLRETURN SQL_API SQLBindParameter(
    SQLHSTMT           hstmt,
    SQLUSMALLINT       ipar,
    SQLSMALLINT        fParamType,
    SQLSMALLINT        fCType,
    SQLSMALLINT        fSqlType,
    SQLUINTEGER        cbColDef,
    SQLSMALLINT        ibScale,
    SQLPOINTER         rgbValue,
    SQLINTEGER         cbValueMax,
    SQLINTEGER FAR    *pcbValue)
{
struct _hstmt *stmt;

	TRACE("SQLBindParameter");
	stmt = (struct _hstmt *) hstmt;
	return SQL_SUCCESS;
}

SQLRETURN SQL_API SQLAllocHandle(    
	SQLSMALLINT HandleType,
    	SQLHANDLE InputHandle,
    	SQLHANDLE * OutputHandle)
{
	TRACE("SQLAllocHandle");
	switch(HandleType) {
		case SQL_HANDLE_STMT:
			return _SQLAllocStmt(InputHandle,OutputHandle);
			break;
		case SQL_HANDLE_DBC:
			return _SQLAllocConnect(InputHandle,OutputHandle);
			break;
		case SQL_HANDLE_ENV:
			return _SQLAllocEnv(OutputHandle);
			break;
	}
}
static SQLRETURN SQL_API _SQLAllocConnect(
    SQLHENV            henv,
    SQLHDBC FAR       *phdbc)
{
struct _henv *env;
ODBCConnection* dbc;

	TRACE("_SQLAllocConnect");
	env = (struct _henv *) henv;
        dbc = (SQLHDBC) malloc (sizeof (ODBCConnection));
        memset(dbc,'\0',sizeof (ODBCConnection));
	dbc->hdbc.henv=env;
	
     dbc->params = NewConnectParams ();
	*phdbc=dbc;

	return SQL_SUCCESS;
}
SQLRETURN SQL_API SQLAllocConnect(
    SQLHENV            henv,
    SQLHDBC FAR       *phdbc)
{
	TRACE("SQLAllocConnect");
	return _SQLAllocConnect(henv, phdbc);
}

static SQLRETURN SQL_API _SQLAllocEnv(
    SQLHENV FAR       *phenv)
{
struct _henv *env;

	TRACE("_SQLAllocEnv");
     env = (SQLHENV) malloc(sizeof(struct _henv));
     memset(env,'\0',sizeof(struct _henv));
	*phenv=env;
	env->drda = drda_init();
	return SQL_SUCCESS;
}
SQLRETURN SQL_API SQLAllocEnv(
    SQLHENV FAR       *phenv)
{
	TRACE("SQLAllocEnv");
	return _SQLAllocEnv(phenv);
}

static SQLRETURN SQL_API _SQLAllocStmt(
    SQLHDBC            hdbc,
    SQLHSTMT FAR      *phstmt)
{
struct _hdbc *dbc;
struct _hstmt *stmt;

	TRACE("_SQLAllocStmt");
	dbc = (struct _hdbc *) hdbc;

     stmt = (SQLHSTMT) malloc(sizeof(struct _hstmt));
     memset(stmt,'\0',sizeof(struct _hstmt));
	stmt->hdbc=hdbc;
	*phstmt = stmt;

	return SQL_SUCCESS;
}
SQLRETURN SQL_API SQLAllocStmt(
    SQLHDBC            hdbc,
    SQLHSTMT FAR      *phstmt)
{
	TRACE("SQLAllocStmt");
	return _SQLAllocStmt(hdbc,phstmt);
}

SQLRETURN SQL_API SQLBindCol(
    SQLHSTMT           hstmt,
    SQLUSMALLINT       icol,
    SQLSMALLINT        fCType,
    SQLPOINTER         rgbValue,
    SQLINTEGER         cbValueMax,
    SQLINTEGER FAR    *pcbValue)
{
struct _hstmt *stmt = (struct _hstmt *) hstmt;
struct _hdbc *dbc = (struct _hdbc *) stmt->hdbc;
struct _henv *env = (struct _henv *) dbc->henv;
struct _sql_bind_info *cur;

	TRACE("SQLBindCol");
	if (stmt->bind_head) {
		cur = stmt->bind_head;
		/* advance to end of bind list */
		while (cur->next) 
			cur = cur->next;
			
		/* alloc struct and assign to next */
		cur->next = (struct _sql_bind_info *) 
			malloc(sizeof(struct _sql_bind_info));
		cur = cur->next;
	} else {
		stmt->bind_head = (struct _sql_bind_info *) 
			malloc(sizeof(struct _sql_bind_info));
		cur = stmt->bind_head;
	}
	memset(cur,0,sizeof(struct _sql_bind_info)); 
	cur->column_number = icol;
	cur->column_bindtype = fCType;
	cur->varaddr = rgbValue;
	cur->column_maxbind = cbValueMax;
	cur->column_lenbind = (SQLINTEGER FAR *) pcbValue;

	return SQL_SUCCESS;
}

SQLRETURN SQL_API SQLCancel(
    SQLHSTMT           hstmt)
{
	TRACE("SQLCancel");
	return SQL_SUCCESS;
}

SQLRETURN SQL_API SQLConnect(
    SQLHDBC            hdbc,
    SQLCHAR FAR       *szDSN,
    SQLSMALLINT        cbDSN,
    SQLCHAR FAR       *szUID,
    SQLSMALLINT        cbUID,
    SQLCHAR FAR       *szAuthStr,
    SQLSMALLINT        cbAuthStr)
{
SQLCHAR FAR* database = NULL;
ConnectParams* params;
SQLRETURN ret;
SQLCHAR FAR *url;
SQLCHAR FAR *uid;
SQLCHAR FAR *passwd;

	TRACE("SQLConnect");
	strcpy (lastError, "");

	params = ((ODBCConnection*) hdbc)->params;

	if (!LookupDSN (params, szDSN)) {
		LogError ("Could not find DSN in odbc.ini");
		return SQL_ERROR;
	}
	else if (!(url = GetConnectParam (params, "URL"))) {
		LogError ("Could not find URL parameter");
		return SQL_ERROR;
	}
	/* These may be code in the url as well, so these are optional */
	uid = GetConnectParam (params, "UID");
	passwd = GetConnectParam (params, "PWD");

	ret = do_connect (hdbc, url, uid, passwd);
	return ret;
}

SQLRETURN SQL_API SQLDescribeCol(
    SQLHSTMT           hstmt,
    SQLUSMALLINT       icol,
    SQLCHAR FAR       *szColName,
    SQLSMALLINT        cbColNameMax,
    SQLSMALLINT FAR   *pcbColName,
    SQLSMALLINT FAR   *pfSqlType,
    SQLUINTEGER FAR   *pcbColDef, /* precision */
    SQLSMALLINT FAR   *pibScale,
    SQLSMALLINT FAR   *pfNullable)
{
int cplen, namelen, i;
struct _hstmt *stmt = (struct _hstmt *) hstmt;
struct _hdbc *dbc = (struct _hdbc *) stmt->hdbc;
struct _henv *env = (struct _henv *) dbc->henv;
DRDA_COLUMN *col;

	TRACE("SQLDescribeCol");
	if (icol<1 || icol>env->drda->sqlda->num_cols) {
		return SQL_ERROR;
	}

	col = env->drda->sqlda->columns[icol-1];

    if (szColName) {
    	if (strlen(col->name) > cbColNameMax) {
			strncpy(szColName, col->name, cbColNameMax-1);
			((char *)szColName)[cbColNameMax]='\0';
		} else {
			strcpy(szColName, col->name);
		}
	}
	if (pcbColName) {
		*pcbColName = strlen(col->name);
	}
	if (pfSqlType) {
		*pfSqlType = _odbc_get_client_type_fdoca(col->fdoca_type);
	}
	if (pcbColDef) {
		*pcbColDef = col->fdoca_len;
	}
	if (pibScale) {
		/* FIX ME */
		*pibScale = 0;
	}
	if (pfNullable) {
		// *pfNullable = !col->is_fixed;
	}

	return SQL_SUCCESS;
}

SQLRETURN SQL_API SQLColAttributes(
    SQLHSTMT           hstmt,
    SQLUSMALLINT       icol,
    SQLUSMALLINT       fDescType,
    SQLPOINTER         rgbDesc,
    SQLSMALLINT        cbDescMax,
    SQLSMALLINT FAR   *pcbDesc,
    SQLINTEGER FAR    *pfDesc)
{
int cplen, len = 0;
struct _hstmt *stmt;
struct _hdbc *dbc;
struct _henv *env;
DRDA_COLUMN *col;

	TRACE("SQLColAttributes");
	stmt = (struct _hstmt *) hstmt;
	dbc = (struct _hdbc *) stmt->hdbc;
	env = (struct _henv *) dbc->henv;


	/* dont check column index for these */
	switch(fDescType) {
		case SQL_COLUMN_COUNT:
			return SQL_SUCCESS;
			break;
	}
	if (icol<1 || icol>env->drda->sqlda->num_cols) {
		return SQL_ERROR;
	}

	col = env->drda->sqlda->columns[icol-1];

	switch(fDescType) {
		case SQL_COLUMN_NAME:
			TRACE("SQLColAttributes SQL_COLUMN_NAME");
			if (strlen(col->name) > cbDescMax) {
				strncpy(rgbDesc, col->name, cbDescMax-1);
				((char *)rgbDesc)[cbDescMax]='\0';
			} else {
				strcpy(rgbDesc, col->name);
			}
			break;
		case SQL_COLUMN_TYPE:
			TRACE("SQLColAttributes SQL_COLUMN_TYPE");
			*pfDesc = _odbc_get_client_type_fdoca(col->fdoca_type);
			break;
		case SQL_COLUMN_LENGTH:
			TRACE("SQLColAttributes SQL_COLUMN_LENGTH");
    		*pfDesc = col->length;
			break;
		case SQL_COLUMN_DISPLAY_SIZE:
			TRACE("SQLColAttributes SQL_COLUMN_DISPLAY_SIZE");
			switch(_odbc_get_client_type_fdoca(col->fdoca_type)) {
				case SQL_CHAR:
				case SQL_VARCHAR:
					*pfDesc = col->length;
					break;
				case SQL_INTEGER:
					*pfDesc = 8;
					break;
				case SQL_SMALLINT:
					*pfDesc = 6;
					break;
				case SQL_TINYINT:
					*pfDesc = 4;
					break;
				case SQL_NUMERIC:
					if (col->scale) 
						*pfDesc = col->precision + 1;
					else 
						*pfDesc = col->precision;
					break;
			}
			if (*pfDesc < strlen(col->name)) {
				*pfDesc = strlen(col->name);
			}
			break;
		case SQL_COLUMN_LABEL:
			TRACE("SQLColAttributes SQL_COLUMN_LABEL");
			if (strlen(col->name) > cbDescMax) {
				strncpy(rgbDesc, col->name, cbDescMax-1);
				((char *)rgbDesc)[cbDescMax]='\0';
			} else {
				strcpy(rgbDesc, col->name);
			}
			break;
		default:
			fprintf(stderr,"Unknown type %d\n",fDescType);
			break;
	}
	return SQL_SUCCESS;
}


SQLRETURN SQL_API SQLDisconnect(
    SQLHDBC            hdbc)
{
	TRACE("SQLDisconnect");
	return SQL_SUCCESS;
}

SQLRETURN SQL_API SQLError(
    SQLHENV            henv,
    SQLHDBC            hdbc,
    SQLHSTMT           hstmt,
    SQLCHAR FAR       *szSqlState,
    SQLINTEGER FAR    *pfNativeError,
    SQLCHAR FAR       *szErrorMsg,
    SQLSMALLINT        cbErrorMsgMax,
    SQLSMALLINT FAR   *pcbErrorMsg)
{
   SQLRETURN result = SQL_NO_DATA_FOUND;
   
	TRACE("SQLError");
   if (strlen (lastError) > 0)
   {
      strcpy (szSqlState, "08001");
      strcpy (szErrorMsg, lastError);
      if (pcbErrorMsg)
	 *pcbErrorMsg = strlen (lastError);
      if (pfNativeError)
	 *pfNativeError = 1;

      result = SQL_SUCCESS;
      strcpy (lastError, "");
   }

   return result;
}

static SQLRETURN SQL_API _SQLExecute( SQLHSTMT hstmt)
{
struct _hstmt *stmt = (struct _hstmt *) hstmt;
struct _hdbc *dbc = (struct _hdbc *) stmt->hdbc;
struct _henv *env = (struct _henv *) dbc->henv;
int ret;
DRDA *drda;

	TRACE("_SQLExecute");
	drda = env->drda;
   
	fprintf(stderr,"query = %s\n",stmt->query);
	_odbc_fix_literals(stmt);

	if (drda_prpsqlstt(drda,stmt->query)<0) {
	//if (drda_prpsqlstt(drda,"select deptno, deptname from department for fetch only")<0) {
		LogError ("SQL Failed on drda_prpsqlstt.\n");
		fprintf(stderr,"failing\n");
		return SQL_ERROR;
	}
	drda->sqlda->columns[0]->bind_addr = bound_data[0];
	drda->sqlda->columns[1]->bind_addr = bound_data[1];
	
	if (drda_opnqry(drda)<0) {
		LogError ("SQL Failed on drda_opnqry.\n");
		return SQL_ERROR;
	}
    drda->result_pos = 0;
    //fprintf(stderr,"len %d\n",drda->result_len);
    stmt->rowcount = 0;

	return SQL_SUCCESS;
}

SQLRETURN SQL_API SQLExecDirect(
    SQLHSTMT           hstmt,
    SQLCHAR FAR       *szSqlStr,
    SQLINTEGER         cbSqlStr)
{
struct _hstmt *stmt = (struct _hstmt *) hstmt;
int ret;

	TRACE("_SQLExecDirect");
   strcpy(stmt->query, szSqlStr);

   return _SQLExecute(hstmt);
}

SQLRETURN SQL_API SQLExecute(
    SQLHSTMT           hstmt)
{
	TRACE("SQLExecute");
   return _SQLExecute(hstmt);
}

SQLRETURN SQL_API SQLFetch(
    SQLHSTMT           hstmt)
{
struct _hstmt *stmt = (struct _hstmt *) hstmt;
struct _hdbc *dbc = (struct _hdbc *) stmt->hdbc;
struct _henv *env = (struct _henv *) dbc->henv;
DRDA *drda;
struct _sql_bind_info *cur;

	TRACE("SQLFetch");
	drda = env->drda;

	/* First call? bind those columns! */
	if (drda->result_pos == 0) {
		cur = stmt->bind_head;
		while (cur) {
			//fprintf(stderr, "Binding %d to %ld\n",cur->column_number, cur->varaddr);
			if (cur->column_number <= drda->sqlda->num_cols) {
				drda->sqlda->columns[cur->column_number-1]->bind_addr = 
					cur->varaddr;
				drda->sqlda->columns[cur->column_number-1]->len_bind_addr = 
					cur->column_lenbind;
			} else {
				// LogError("
			}
			cur = cur->next;
		}
	}

	// fprintf(stderr,"pos %d %d\n", drda->result_pos, drda->result_len);
    if (drda->result_pos<drda->result_len) {
		//fprintf(stderr,"pos %d\n", drda->result_pos);
        drda->result_pos += fdoca_read_qrydta_row(drda, drda->result_buf, drda->result_len, drda->result_pos);
		stmt->rowcount++;
		return SQL_SUCCESS;
	} else {
		/* clean up shop and return */
        	//drda_rdbcmm(drda);
		return SQL_NO_DATA_FOUND;
	}
}

SQLRETURN SQL_API SQLFreeHandle(    
	SQLSMALLINT HandleType,
    	SQLHANDLE Handle)
{
	TRACE("SQLFreeHandle");
	switch(HandleType) {
		case SQL_HANDLE_STMT:
			_SQLFreeStmt(Handle,SQL_DROP);
			break;
		case SQL_HANDLE_DBC:
			_SQLFreeConnect(Handle);
			break;
		case SQL_HANDLE_ENV:
			_SQLFreeEnv(Handle);
			break;
	}
   return SQL_SUCCESS;
}

static SQLRETURN SQL_API _SQLFreeConnect(
    SQLHDBC            hdbc)
{
   ODBCConnection* dbc = (ODBCConnection*) hdbc;

	TRACE("_SQLFreeConnect");

   FreeConnectParams (dbc->params);
   free (dbc);

   return SQL_SUCCESS;
}
SQLRETURN SQL_API SQLFreeConnect(
    SQLHDBC            hdbc)
{
	TRACE("SQLFreeConnect");
	return _SQLFreeConnect(hdbc);
}

static SQLRETURN SQL_API _SQLFreeEnv(
    SQLHENV            henv)
{
	TRACE("_SQLFreeEnv");
	return SQL_SUCCESS;
}
SQLRETURN SQL_API SQLFreeEnv(
    SQLHENV            henv)
{
	TRACE("SQLFreeEnv");
	return _SQLFreeEnv(henv);
}

static SQLRETURN SQL_API _SQLFreeStmt(
    SQLHSTMT           hstmt,
    SQLUSMALLINT       fOption)
{
struct _hstmt *stmt=(struct _hstmt *)hstmt;
struct _sql_bind_info *cur, *next;

	TRACE("_SQLFreeStmt");
   if (fOption==SQL_DROP) {
	cur = stmt->bind_head;	
	while (cur) {
		next = cur->next;
		free(cur);
		cur = next;
	}
   	free (hstmt);
   } else if (fOption==SQL_CLOSE) {
   } else {
   }
   return SQL_SUCCESS;
}
SQLRETURN SQL_API SQLFreeStmt(
    SQLHSTMT           hstmt,
    SQLUSMALLINT       fOption)
{
	TRACE("SQLFreeStmt");
	return _SQLFreeStmt(hstmt, fOption);
}

SQLRETURN SQL_API SQLGetStmtAttr (
    SQLHSTMT StatementHandle,
    SQLINTEGER Attribute,
    SQLPOINTER Value,
    SQLINTEGER BufferLength,
    SQLINTEGER * StringLength)
{
	TRACE("SQLGetStmtAttr");
   return SQL_SUCCESS;
}

SQLRETURN SQL_API SQLGetCursorName(
    SQLHSTMT           hstmt,
    SQLCHAR FAR       *szCursor,
    SQLSMALLINT        cbCursorMax,
    SQLSMALLINT FAR   *pcbCursor)
{
	TRACE("SQLGetCursorName");
	return SQL_SUCCESS;
}

SQLRETURN SQL_API SQLNumResultCols(
    SQLHSTMT           hstmt,
    SQLSMALLINT FAR   *pccol)
{
struct _hstmt *stmt = (struct _hstmt *) hstmt;
struct _hdbc *dbc = (struct _hdbc *) stmt->hdbc;
struct _henv *env = (struct _henv *) dbc->henv;
	
	TRACE("SQLNumResultCols");
	*pccol = env->drda->sqlda->num_cols;
	return SQL_SUCCESS;
}

SQLRETURN SQL_API SQLPrepare(
    SQLHSTMT           hstmt,
    SQLCHAR FAR       *szSqlStr,
    SQLINTEGER         cbSqlStr)
{
   struct _hstmt *stmt=(struct _hstmt *)hstmt;

	TRACE("SQLPrepare");

   if (cbSqlStr!=SQL_NTS) {
	strncpy(stmt->query, szSqlStr, cbSqlStr);
	stmt->query[cbSqlStr]='\0';
   } else {
   	strcpy(stmt->query, szSqlStr);
   }

   return SQL_SUCCESS;
}

SQLRETURN SQL_API SQLRowCount(
    SQLHSTMT           hstmt,
    SQLINTEGER FAR    *pcrow)
{
struct _hstmt *stmt=(struct _hstmt *)hstmt;

	TRACE("SQLRowCount");
	*pcrow = stmt->rowcount;
	return SQL_SUCCESS;
}

SQLRETURN SQL_API SQLSetCursorName(
    SQLHSTMT           hstmt,
    SQLCHAR FAR       *szCursor,
    SQLSMALLINT        cbCursor)
{
	TRACE("SQLSetCursorName");
	return SQL_SUCCESS;
}

SQLRETURN SQL_API SQLTransact(
    SQLHENV            henv,
    SQLHDBC            hdbc,
    SQLUSMALLINT       fType)
{
	TRACE("SQLTransact");
	return SQL_SUCCESS;
}


SQLRETURN SQL_API SQLSetParam(            /*      Use SQLBindParameter */
    SQLHSTMT           hstmt,
    SQLUSMALLINT       ipar,
    SQLSMALLINT        fCType,
    SQLSMALLINT        fSqlType,
    SQLUINTEGER        cbParamDef,
    SQLSMALLINT        ibScale,
    SQLPOINTER         rgbValue,
    SQLINTEGER FAR     *pcbValue)
{
	TRACE("SQLSetParam");
	return SQL_SUCCESS;
}

SQLRETURN SQL_API SQLColumns(
    SQLHSTMT           hstmt,
    SQLCHAR FAR       *szCatalogName,
    SQLSMALLINT        cbCatalogName,
    SQLCHAR FAR       *szSchemaName,
    SQLSMALLINT        cbSchemaName,
    SQLCHAR FAR       *szTableName,
    SQLSMALLINT        cbTableName,
    SQLCHAR FAR       *szColumnName,
    SQLSMALLINT        cbColumnName)
{
	TRACE("SQLColumns");
	return SQL_SUCCESS;
}

SQLRETURN SQL_API SQLGetConnectOption(
    SQLHDBC            hdbc,
    SQLUSMALLINT       fOption,
    SQLPOINTER         pvParam)
{
	TRACE("SQLGetConnectOption");
	return SQL_SUCCESS;
}

SQLRETURN SQL_API SQLGetData(
    SQLHSTMT           hstmt,
    SQLUSMALLINT       icol,
    SQLSMALLINT        fCType,
    SQLPOINTER         rgbValue,
    SQLINTEGER         cbValueMax,
    SQLINTEGER FAR    *pcbValue)
{
struct _hstmt *stmt;
struct _hdbc *dbc;
struct _henv *env;
unsigned char *src;
int srclen;
DRDA_COLUMN *col;

	TRACE("SQLGetData");
	stmt = (struct _hstmt *) hstmt;
	dbc = (struct _hdbc *) stmt->hdbc;
	env = (struct _henv *) dbc->henv;

	if (icol<1 || icol>env->drda->sqlda->num_cols) {
		return SQL_ERROR;
	}

	col = env->drda->sqlda->columns[icol-1];

	/* FIX ME - SQL_CHAR only */
	switch (fCType) {
		case SQL_CHAR:
		case SQL_VARCHAR:
			if (col->cur_length > cbValueMax) {
				/* if unsupported type, error and stop */
				if (!drda_get_cardinal_type(col->fdoca_type)) {
					fprintf(stderr,"Error - Column was %s\n",col->name);
					exit(1);
				}
				if (col->precision) {
					drda_convert_precision(env->drda, col->cur_addr, drda_get_cardinal_type(col->fdoca_type), col->precision, col->scale, rgbValue, DRDA_VARCHAR, cbValueMax);
            	} else {
					drda_convert(env->drda, col->cur_addr, drda_get_cardinal_type(col->fdoca_type), 0, rgbValue, DRDA_VARCHAR, cbValueMax);
				}
/*
				strncpy(rgbValue,col->cur_addr, cbValueMax-1);
				((char *)rgbValue)[cbValueMax]='\0';
*/
				if (pcbValue) *pcbValue = cbValueMax;
			} else if (col->cur_length > 0) {
				/* if unsupported type, error and stop */
				if (!drda_get_cardinal_type(col->fdoca_type)) {
					fprintf(stderr,"Error - Column was %s\n",col->name);
					exit(1);
				}
				if (col->precision) {
					drda_convert_precision(env->drda, col->cur_addr, drda_get_cardinal_type(col->fdoca_type), col->precision, col->scale, rgbValue, DRDA_VARCHAR, col->precision+1);
            	} else {
					drda_convert(env->drda, col->cur_addr, drda_get_cardinal_type(col->fdoca_type), 0, rgbValue, DRDA_VARCHAR, col->cur_length);
				}
/*
				strncpy(rgbValue,col->cur_addr, col->cur_length);
				((char *)rgbValue)[col->cur_length]='\0';
*/
				if (pcbValue) *pcbValue = col->cur_length;
			} else {
				drda_set_null_value(env->drda, rgbValue, DRDA_VARCHAR);
			}
		break;
		default:
			fprintf(stderr,"Unknown bind type %d\n",fCType);
		break;
	}
	return SQL_SUCCESS;
}

static void _set_func_exists(SQLUSMALLINT FAR *pfExists, SQLUSMALLINT fFunction)
{
SQLUSMALLINT FAR *mod;

	mod = pfExists + (fFunction >> 4);
	*mod |= (1 << (fFunction & 0x0f));
}
SQLRETURN SQL_API SQLGetFunctions(
    SQLHDBC            hdbc,
    SQLUSMALLINT       fFunction,
    SQLUSMALLINT FAR  *pfExists)
{
int i;

	TRACE("SQLGetFunctions");
	switch (fFunction) {
#if ODBCVER >= 0x0300
		case SQL_API_ODBC3_ALL_FUNCTIONS:
 
/*			for (i=0;i<SQL_API_ODBC3_ALL_FUNCTIONS_SIZE;i++) {
				pfExists[i] = 0xFFFF;
			}
*/
			_set_func_exists(pfExists,SQL_API_SQLALLOCCONNECT);
			_set_func_exists(pfExists,SQL_API_SQLALLOCENV);
			_set_func_exists(pfExists,SQL_API_SQLALLOCHANDLE);
			_set_func_exists(pfExists,SQL_API_SQLALLOCSTMT);
			_set_func_exists(pfExists,SQL_API_SQLBINDCOL);
			_set_func_exists(pfExists,SQL_API_SQLBINDPARAMETER);
			_set_func_exists(pfExists,SQL_API_SQLCANCEL);
			_set_func_exists(pfExists,SQL_API_SQLCLOSECURSOR);
			_set_func_exists(pfExists,SQL_API_SQLCOLATTRIBUTE);
			_set_func_exists(pfExists,SQL_API_SQLCOLUMNS);
			_set_func_exists(pfExists,SQL_API_SQLCONNECT);
			_set_func_exists(pfExists,SQL_API_SQLCOPYDESC);
			_set_func_exists(pfExists,SQL_API_SQLDATASOURCES);
			_set_func_exists(pfExists,SQL_API_SQLDESCRIBECOL);
			_set_func_exists(pfExists,SQL_API_SQLDISCONNECT);
			_set_func_exists(pfExists,SQL_API_SQLENDTRAN);
			_set_func_exists(pfExists,SQL_API_SQLERROR);
			_set_func_exists(pfExists,SQL_API_SQLEXECDIRECT);
			_set_func_exists(pfExists,SQL_API_SQLEXECUTE);
			_set_func_exists(pfExists,SQL_API_SQLFETCH);
			_set_func_exists(pfExists,SQL_API_SQLFETCHSCROLL);
			_set_func_exists(pfExists,SQL_API_SQLFREECONNECT);
			_set_func_exists(pfExists,SQL_API_SQLFREEENV);
			_set_func_exists(pfExists,SQL_API_SQLFREEHANDLE);
			_set_func_exists(pfExists,SQL_API_SQLFREESTMT);
			_set_func_exists(pfExists,SQL_API_SQLGETCONNECTATTR);
			_set_func_exists(pfExists,SQL_API_SQLGETCONNECTOPTION);
			_set_func_exists(pfExists,SQL_API_SQLGETCURSORNAME);
			_set_func_exists(pfExists,SQL_API_SQLGETDATA);
			_set_func_exists(pfExists,SQL_API_SQLGETDESCFIELD);
			_set_func_exists(pfExists,SQL_API_SQLGETDESCREC);
			_set_func_exists(pfExists,SQL_API_SQLGETDIAGFIELD);
			_set_func_exists(pfExists,SQL_API_SQLGETDIAGREC);
			_set_func_exists(pfExists,SQL_API_SQLGETENVATTR);
			_set_func_exists(pfExists,SQL_API_SQLGETFUNCTIONS);
			_set_func_exists(pfExists,SQL_API_SQLGETINFO);
			_set_func_exists(pfExists,SQL_API_SQLGETSTMTATTR);
			_set_func_exists(pfExists,SQL_API_SQLGETSTMTOPTION);
			_set_func_exists(pfExists,SQL_API_SQLGETTYPEINFO);
			_set_func_exists(pfExists,SQL_API_SQLNUMRESULTCOLS);
			_set_func_exists(pfExists,SQL_API_SQLPARAMDATA);
			_set_func_exists(pfExists,SQL_API_SQLPREPARE);
			_set_func_exists(pfExists,SQL_API_SQLPUTDATA);
			_set_func_exists(pfExists,SQL_API_SQLROWCOUNT);
			_set_func_exists(pfExists,SQL_API_SQLSETCONNECTATTR);
			_set_func_exists(pfExists,SQL_API_SQLSETCONNECTOPTION);
			_set_func_exists(pfExists,SQL_API_SQLSETCURSORNAME);
			_set_func_exists(pfExists,SQL_API_SQLSETDESCFIELD);
			_set_func_exists(pfExists,SQL_API_SQLSETDESCREC);
			_set_func_exists(pfExists,SQL_API_SQLSETENVATTR);
			_set_func_exists(pfExists,SQL_API_SQLSETPARAM);
			_set_func_exists(pfExists,SQL_API_SQLSETSTMTATTR);
			_set_func_exists(pfExists,SQL_API_SQLSETSTMTOPTION);
			_set_func_exists(pfExists,SQL_API_SQLSPECIALCOLUMNS);
			_set_func_exists(pfExists,SQL_API_SQLSTATISTICS);
			_set_func_exists(pfExists,SQL_API_SQLTABLES);
			_set_func_exists(pfExists,SQL_API_SQLTRANSACT);

			return SQL_SUCCESS;
			break;
#endif
		case SQL_API_ALL_FUNCTIONS:
			_set_func_exists(pfExists,SQL_API_SQLALLOCCONNECT);
			_set_func_exists(pfExists,SQL_API_SQLALLOCENV);
			_set_func_exists(pfExists,SQL_API_SQLALLOCSTMT);
			_set_func_exists(pfExists,SQL_API_SQLBINDCOL);
			_set_func_exists(pfExists,SQL_API_SQLCANCEL);
			_set_func_exists(pfExists,SQL_API_SQLCOLATTRIBUTES);
			_set_func_exists(pfExists,SQL_API_SQLCOLUMNS);
			_set_func_exists(pfExists,SQL_API_SQLCONNECT);
			_set_func_exists(pfExists,SQL_API_SQLDATASOURCES);
			_set_func_exists(pfExists,SQL_API_SQLDESCRIBECOL);
			_set_func_exists(pfExists,SQL_API_SQLDISCONNECT);
			_set_func_exists(pfExists,SQL_API_SQLERROR);
			_set_func_exists(pfExists,SQL_API_SQLEXECDIRECT);
			_set_func_exists(pfExists,SQL_API_SQLEXECUTE);
			_set_func_exists(pfExists,SQL_API_SQLFETCH);
			_set_func_exists(pfExists,SQL_API_SQLFREECONNECT);
			_set_func_exists(pfExists,SQL_API_SQLFREEENV);
			_set_func_exists(pfExists,SQL_API_SQLFREESTMT);
			_set_func_exists(pfExists,SQL_API_SQLGETCONNECTOPTION);
			_set_func_exists(pfExists,SQL_API_SQLGETCURSORNAME);
			_set_func_exists(pfExists,SQL_API_SQLGETDATA);
			_set_func_exists(pfExists,SQL_API_SQLGETFUNCTIONS);
			_set_func_exists(pfExists,SQL_API_SQLGETINFO);
			_set_func_exists(pfExists,SQL_API_SQLGETSTMTOPTION);
			_set_func_exists(pfExists,SQL_API_SQLGETTYPEINFO);
			_set_func_exists(pfExists,SQL_API_SQLNUMRESULTCOLS);
			_set_func_exists(pfExists,SQL_API_SQLPARAMDATA);
			_set_func_exists(pfExists,SQL_API_SQLPREPARE);
			_set_func_exists(pfExists,SQL_API_SQLPUTDATA);
			_set_func_exists(pfExists,SQL_API_SQLROWCOUNT);
			_set_func_exists(pfExists,SQL_API_SQLSETCONNECTOPTION);
			_set_func_exists(pfExists,SQL_API_SQLSETCURSORNAME);
			_set_func_exists(pfExists,SQL_API_SQLSETPARAM);
			_set_func_exists(pfExists,SQL_API_SQLSETSTMTOPTION);
			_set_func_exists(pfExists,SQL_API_SQLSPECIALCOLUMNS);
			_set_func_exists(pfExists,SQL_API_SQLSTATISTICS);
			_set_func_exists(pfExists,SQL_API_SQLTABLES);
			_set_func_exists(pfExists,SQL_API_SQLTRANSACT);
			return SQL_SUCCESS;
			break;
		default:
			*pfExists = 1; /* SQL_TRUE */
			return SQL_SUCCESS;
			break;
	}
	return SQL_SUCCESS;
}

SQLRETURN SQL_API SQLGetInfo(
    SQLHDBC            hdbc,
    SQLUSMALLINT       fInfoType,
    SQLPOINTER         rgbInfoValue,
    SQLSMALLINT        cbInfoValueMax,
    SQLSMALLINT FAR   *pcbInfoValue)
{
	TRACE("SQLGetInfo");
	return SQL_SUCCESS;
}

SQLRETURN SQL_API SQLGetStmtOption(
    SQLHSTMT           hstmt,
    SQLUSMALLINT       fOption,
    SQLPOINTER         pvParam)
{
	TRACE("SQLGetStmtOption");
	return SQL_SUCCESS;
}

SQLRETURN SQL_API SQLGetTypeInfo(
    SQLHSTMT           hstmt,
    SQLSMALLINT        fSqlType)
{
struct _hstmt *stmt;

	TRACE("SQLGetTypeInfo");
	stmt = (struct _hstmt *) hstmt;
	if (!fSqlType) {
   		strcpy(stmt->query, "SELECT * FROM tds_typeinfo");
	} else {
			sprintf(stmt->query, "SELECT * FROM tds_typeinfo WHERE SQL_DATA_TYPE = %d", fSqlType);
		}
		
		return _SQLExecute(hstmt);
	}

SQLRETURN SQL_API SQLParamData(
    SQLHSTMT           hstmt,
    SQLPOINTER FAR    *prgbValue)
{
	TRACE("SQLParamData");
	return SQL_SUCCESS;
}

SQLRETURN SQL_API SQLPutData(
    SQLHSTMT           hstmt,
    SQLPOINTER         rgbValue,
    SQLINTEGER         cbValue)
{
	TRACE("SQLPutData");
	return SQL_SUCCESS;
}

SQLRETURN SQL_API SQLSetConnectOption(
    SQLHDBC            hdbc,
    SQLUSMALLINT       fOption,
    SQLUINTEGER        vParam)
{
	TRACE("SQLSetConnectOption");
	return SQL_SUCCESS;
}

SQLRETURN SQL_API SQLSetStmtOption(
    SQLHSTMT           hstmt,
    SQLUSMALLINT       fOption,
    SQLUINTEGER        vParam)
{
	TRACE("SQLSetStmtOption");
	return SQL_SUCCESS;
}

SQLRETURN SQL_API SQLSpecialColumns(
    SQLHSTMT           hstmt,
    SQLUSMALLINT       fColType,
    SQLCHAR FAR       *szCatalogName,
    SQLSMALLINT        cbCatalogName,
    SQLCHAR FAR       *szSchemaName,
    SQLSMALLINT        cbSchemaName,
    SQLCHAR FAR       *szTableName,
    SQLSMALLINT        cbTableName,
    SQLUSMALLINT       fScope,
    SQLUSMALLINT       fNullable)
{
	TRACE("SQLSpecialColumns");
	return SQL_SUCCESS;
}

SQLRETURN SQL_API SQLStatistics(
    SQLHSTMT           hstmt,
    SQLCHAR FAR       *szCatalogName,
    SQLSMALLINT        cbCatalogName,
    SQLCHAR FAR       *szSchemaName,
    SQLSMALLINT        cbSchemaName,
    SQLCHAR FAR       *szTableName,
    SQLSMALLINT        cbTableName,
    SQLUSMALLINT       fUnique,
    SQLUSMALLINT       fAccuracy)
{
	TRACE("SQLStatistics");
	return SQL_SUCCESS;
}

SQLRETURN SQL_API SQLTables(
    SQLHSTMT           hstmt,
    SQLCHAR FAR       *szCatalogName,
    SQLSMALLINT        cbCatalogName,
    SQLCHAR FAR       *szSchemaName,
    SQLSMALLINT        cbSchemaName,
    SQLCHAR FAR       *szTableName,
    SQLSMALLINT        cbTableName,
    SQLCHAR FAR       *szTableType,
    SQLSMALLINT        cbTableType)
{
char *query, *p;
char *sptables = "exec sp_tables ";
int querylen, clen, slen, tlen, ttlen;
int first = 1;
struct _hstmt *stmt;

	TRACE("SQLTables");
	stmt = (struct _hstmt *) hstmt;

	clen = _odbc_get_string_size(cbCatalogName, szCatalogName);
	slen = _odbc_get_string_size(cbSchemaName, szSchemaName);
	tlen = _odbc_get_string_size(cbTableName, szTableName);
	ttlen = _odbc_get_string_size(cbTableType, szTableType);

	querylen = strlen(sptables) + clen + slen + tlen + ttlen + 40; /* a little padding for quotes and commas */
	query = (char *) malloc(querylen);
	p = query;

	strcpy(p, sptables);
	p += strlen(sptables);

	if (tlen) {
		*p++ = '"';
		strncpy(p, szTableName, tlen); *p+=tlen;
		*p++ = '"';
		first = 0;
	}
	if (slen) {
		if (!first) *p++ = ',';
		*p++ = '"';
		strncpy(p, szSchemaName, slen); *p+=slen;
		*p++ = '"';
		first = 0;
	}
	if (clen) {
		if (!first) *p++ = ',';
		*p++ = '"';
		strncpy(p, szCatalogName, clen); *p+=clen;
		*p++ = '"';
		first = 0;
	}
	if (ttlen) {
		if (!first) *p++ = ',';
		*p++ = '"';
		strncpy(p, szTableType, ttlen); *p+=ttlen;
		*p++ = '"';
		first = 0;
	}
	*p++ = '\0';
	// fprintf(stderr,"\nquery = %s\n",query);

	strcpy(stmt->query, query);
	return _SQLExecute(hstmt);
}


SQLRETURN SQL_API SQLDataSources(
    SQLHENV            henv,
    SQLUSMALLINT       fDirection,
    SQLCHAR FAR       *szDSN,
    SQLSMALLINT        cbDSNMax,
    SQLSMALLINT FAR   *pcbDSN,
    SQLCHAR FAR       *szDescription,
    SQLSMALLINT        cbDescriptionMax,
    SQLSMALLINT FAR   *pcbDescription)
{
	TRACE("SQLDataSources");
	return SQL_SUCCESS;
}

static int _odbc_fix_literals(struct _hstmt *stmt)
{
char tmp[4096],begin_tag[11];
char *s, *d, *p;
int i, quoted = 0, find_end = 0;
char quote_char;

	TRACE("_odbc_fix_literals");
    s=stmt->query;
    d=tmp;
    while (*s) {
	if (!quoted && (*s=='"' || *s=='\'')) {
		quoted = 1;
		quote_char = *s;
	} else if (quoted && *s==quote_char) {
		quoted = 0;
	}
	if (!quoted && find_end && *s=='}') {
		s++; /* ignore the end of tag */
	} else if (!quoted && *s=='{') {
		for (p=s,i=0;*p && *p!=' ';p++) i++;
		if (i>10) {
			/* garbage */
			*d++=*s++;
		} else {
			strncpy(begin_tag, s, i);
			begin_tag[i] = '\0';
			/* printf("begin tag %s\n", begin_tag); */
			s += i;
			find_end = 1;
		}
	} else {
		*d++=*s++;	
	}
	}
	*d='\0';
	strcpy(stmt->query,tmp);
}

static int _odbc_get_string_size(int size, char *str)
{
	TRACE("_odbc_get_string_size");
	if (!str) {
		return 0;
	}
	if (size==SQL_NTS) {
		return strlen(str);
	} else {
		return size;
	}
}
static int _odbc_get_server_type(int clt_type)
{
	TRACE("_odbc_get_server_type");
	switch (clt_type) {
	case SQL_CHAR:
	case SQL_VARCHAR:
	case SQL_BIT:
	case SQL_TINYINT:
	case SQL_SMALLINT:
	case SQL_INTEGER:
	case SQL_DOUBLE:
	case SQL_DECIMAL:
	case SQL_NUMERIC:
	case SQL_FLOAT:
	}
}
static SQLSMALLINT _odbc_get_client_type_fdoca(int fdoca_type)
{
	return _odbc_get_client_type(drda_get_cardinal_type(fdoca_type));
}
static SQLSMALLINT _odbc_get_client_type(int srv_type)
{
	TRACE("_odbc_get_client_type");
	switch (srv_type) {
		case DRDA_DECIMAL:
			return SQL_NUMERIC;
			break;
		case DRDA_SMALLINT:
			return SQL_SMALLINT;
			break;
	}
	return SQL_VARCHAR;
}

