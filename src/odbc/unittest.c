/*
 * Copyright (C) 2001 Brian Bruns (camber@ais.org)
 */
/* #include <windows.h> */
#include <sql.h>
#include <sqlext.h>

#include <stdio.h>

static char  software_version[]   = "$Id: unittest.c,v 1.3 2002/03/19 01:01:40 brianb Exp $";
static void *no_unused_var_warn[] = {software_version,
                                     no_unused_var_warn};



#define SALES_PERSON_LEN 2
#define STATUS_LEN 6

SQLSMALLINT       sOrderID;
SQLSMALLINT       sCustID;
DATE_STRUCT dsOpenDate;
SQLCHAR       szSalesPerson[SALES_PERSON_LEN] = "D";
SQLCHAR       szStatus[STATUS_LEN];
SQLINTEGER      cbOrderID = 0, cbCustID = 0, cbOpenDate = 0, cbSalesPerson = SQL_NTS,
            cbStatus = SQL_NTS;
SQLRETURN   retcode;
HENV        henv;
HDBC        hdbc;
SQLHSTMT    hstmt;

static void printStatementError(HSTMT hstmt, char *msg)
{
	UCHAR  szSqlState[6];
	UCHAR  szErrorMsg[SQL_MAX_MESSAGE_LENGTH];
	SQLINTEGER dwNativeError;
	SWORD  wErrorMsg;

	SQLError(SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, 
			szSqlState, &dwNativeError, szErrorMsg,
			SQL_MAX_MESSAGE_LENGTH-1, &wErrorMsg);

	fprintf(stderr,"%s\n%s\n%s\n",msg,
		szSqlState, szErrorMsg);
}


int main()
{
int i;

	retcode = SQLAllocEnv(&henv);
	
	if (SQLAllocConnect(henv, &hdbc) != SQL_SUCCESS)
	{
		UCHAR  szSqlState[6];
		UCHAR  szErrorMsg[SQL_MAX_MESSAGE_LENGTH];
		SQLINTEGER dwNativeError;
		SWORD  wErrorMsg;

		SQLError(henv, SQL_NULL_HDBC, SQL_NULL_HSTMT, 
			szSqlState, &dwNativeError, szErrorMsg,
			SQL_MAX_MESSAGE_LENGTH-1, &wErrorMsg);

		fprintf(stderr,"problem with SQLAllocConnect\n%s\n%s\n", 
			szSqlState, szErrorMsg);
		exit(1);
	}


	retcode = SQLSetConnectOption(hdbc, SQL_ACCESS_MODE, SQL_MODE_READ_ONLY);
	if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO)
	{
		UCHAR  szSqlState[6];
		UCHAR  szErrorMsg[SQL_MAX_MESSAGE_LENGTH];
		SQLINTEGER dwNativeError;
		SWORD  wErrorMsg;

		SQLError(SQL_NULL_HENV, hdbc, SQL_NULL_HSTMT, 
			szSqlState, &dwNativeError, szErrorMsg,
			SQL_MAX_MESSAGE_LENGTH-1, &wErrorMsg);

		fprintf(stderr,"problem with SQLSetConnectOption\n%s\n%s\n",
			szSqlState, szErrorMsg);
		exit(1);
	}

	retcode = SQLSetConnectOption(hdbc, SQL_AUTOCOMMIT, SQL_AUTOCOMMIT_ON);
	if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO)
	{
		UCHAR  szSqlState[6];
		UCHAR  szErrorMsg[SQL_MAX_MESSAGE_LENGTH];
		SQLINTEGER dwNativeError;
		SWORD  wErrorMsg;

		SQLError(SQL_NULL_HENV, hdbc, SQL_NULL_HSTMT, 
			szSqlState, &dwNativeError, szErrorMsg,
			SQL_MAX_MESSAGE_LENGTH-1, &wErrorMsg);

		fprintf(stderr,"problem with SQLSetConnectOption\n%s\n%s\n",
			szSqlState, szErrorMsg);
		exit(1);
	}


	retcode = SQLConnect(hdbc, 
				   (UCHAR *)"sample", SQL_NTS, 
				   (UCHAR *)"", SQL_NTS,
				   (UCHAR *)"", SQL_NTS);
	if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO)
	{
		UCHAR  szSqlState[6];
		UCHAR  szErrorMsg[SQL_MAX_MESSAGE_LENGTH];
		SQLINTEGER dwNativeError;
		SWORD  wErrorMsg;

		SQLError(SQL_NULL_HENV, hdbc, SQL_NULL_HSTMT, 
			szSqlState, &dwNativeError, szErrorMsg,
			SQL_MAX_MESSAGE_LENGTH-1, &wErrorMsg);

		fprintf(stderr,"problem with SQLConnect\n%s\n%s\n",
			szSqlState, szErrorMsg);
		exit(1);
	}


	if (SQLAllocStmt(hdbc, &hstmt)!= SQL_SUCCESS)
	{
		UCHAR  szSqlState[6];
		UCHAR  szErrorMsg[SQL_MAX_MESSAGE_LENGTH];
		SQLINTEGER dwNativeError;
		SWORD  wErrorMsg;

		SQLError(SQL_NULL_HENV, hdbc, SQL_NULL_HSTMT, 
			szSqlState, &dwNativeError, szErrorMsg,
			SQL_MAX_MESSAGE_LENGTH-1, &wErrorMsg);

		fprintf(stderr,"problem with SQLAllocStmt\n%s\n%s\n",
			szSqlState, szErrorMsg);
		exit(1);
	}

	/* Prepare the SQL statement with parameter markers. */

	retcode = SQLPrepare(hstmt,
 	       (unsigned char *)"select deptno,deptname from department for fetch only", 
			SQL_NTS);
			  
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) 
	{
		long   p1;
		long   p1Len = sizeof(p1);
		long   sAge  = 1023;
		long   cbAge = sizeof(long);
		UCHAR  szCol1[60];
		UCHAR  szCol2[60];

		printf("excecuting first statement\n");
		retcode = SQLExecute(hstmt);         
		if (retcode != SQL_SUCCESS) {
			UCHAR  szSqlState[6];
			UCHAR  szErrorMsg[SQL_MAX_MESSAGE_LENGTH];
			SQLINTEGER dwNativeError;
			SWORD  wErrorMsg;

			SQLError(SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, 
				szSqlState, &dwNativeError, szErrorMsg,
				SQL_MAX_MESSAGE_LENGTH-1, &wErrorMsg);

			fprintf(stderr,"problem with SQLExecute\n%s\n%s\n",
				szSqlState, szErrorMsg);
			exit(1);
		}		
		SQLBindCol(hstmt, 1, SQL_CHAR, szCol1, 60, NULL);
		SQLBindCol(hstmt, 2, SQL_CHAR, szCol2, 60, NULL);
		//SQLBindCol(hstmt, 1, SQL_CHAR, szCol1, 60, NULL);
	
		/* Execute statement with first row. */

		i=0;
		while ((retcode = SQLFetch(hstmt)) == SQL_SUCCESS)
		{
			i++;
			printf("%d: szCol1 = %s szCol2 = %s\n",i,szCol1,szCol2);
		}
		if (retcode != SQL_NO_DATA_FOUND)
		{
			printStatementError(hstmt, "problem with SQLFetch");
			exit(1);
		}
	}		
	printf("Done\n");

	return 1;
}
