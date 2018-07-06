#include "stdafx.h"
#include "tdb.h"


static int connection_failure = 0;

DB_CON CreateDBConnect()
{
	return create_db_connect();
}

void ReleaseDBConnect(DB_CON con)
{
	release_db_connect(con);
}

void	DBclose(DB_CON con)
{
	t_db_close(con);
}

void	DBinit(void)
{
	t_db_init(db_dbname);
}

int	DBconnect(DB_CON con, int flag)
{
	const char	*__function_name = "DBconnect";
	int		err;

	zlog_debug(z_cate, "In %s() flag:%d", __function_name, flag);
	while (T_DB_OK != (err = t_db_connect(con, db_host, db_user, db_password,
			db_dbname, db_schema, db_socket, db_port)))
	{
		if (T_DB_CONNECT_ONCE == flag)
			break;

		if (T_DB_FAIL == err || T_DB_CONNECT_EXIT == flag)
		{
			zlog_fatal(z_cate, "Cannot connect to the database. Exiting...");
			return T_DB_FAIL;
		}

		zlog_error(z_cate, "database is down: reconnecting in %d seconds", T_DB_WAIT_DOWN);
		connection_failure = 1;
		Sleep(T_DB_WAIT_DOWN*1000);
	}

	if (0 != connection_failure)
	{
		zlog_error(z_cate, "database connection re-established");
		connection_failure = 0;
	}

	zlog_debug(z_cate, "End of %s():%d", __function_name, err);

	return err;
}

/*事务操作*/
static void DBtxn_operation(int (*txn_operation)(DB_CON), DB_CON con)
{
	int	rc;

	rc = txn_operation(con);

	while (T_DB_DOWN == rc)
	{
		DBclose(con);
		DBconnect(con, T_DB_CONNECT_NORMAL);

		if (T_DB_DOWN == (rc = txn_operation(con)))
		{
			zlog_error(z_cate, "database is down: retrying in %d seconds", T_DB_WAIT_DOWN);
			connection_failure = 1;
			Sleep(T_DB_WAIT_DOWN*1000);
		}
	}
}

void	DBbegin(DB_CON con)
{
	DBtxn_operation(t_db_begin, con);
}

void	DBcommit(DB_CON con)
{
	DBtxn_operation(t_db_commit, con);
}

void	DBrollback(DB_CON con)
{
	DBtxn_operation(t_db_rollback, con);
}

void	DBend(DB_CON con, int ret)
{
	if (SUCCEED == ret)
		DBtxn_operation(t_db_commit, con);
	else
		DBtxn_operation(t_db_rollback, con);
}

int	DBexecute(DB_CON con, const char *fmt, ...)
{
	va_list	args;
	int	rc;

	va_start(args, fmt);

	rc = t_db_vexecute(con, fmt, args);

	while (T_DB_DOWN == rc)
	{
		DBclose(con);
		DBconnect(con, T_DB_CONNECT_NORMAL);

		if (T_DB_DOWN == (rc = t_db_vexecute(con, fmt, args)))
		{
			zlog_error(z_cate, "database is down: retrying in %d seconds", T_DB_WAIT_DOWN);
			connection_failure = 1;
			Sleep(T_DB_WAIT_DOWN*1000);
		}
	}

	va_end(args);

	return rc;
}

DB_ROW	DBfetch(DB_RESULT result)
{
	return t_db_fetch(result);
}

DB_RESULT DBselect_once(DB_CON con, const char *fmt, ...)
{
	va_list		args;
	DB_RESULT	rc;

	va_start(args, fmt);

	rc = t_db_vselect(con, fmt, args);

	va_end(args);

	return rc;
}

DB_RESULT DBselect(DB_CON con, const char *fmt, ...)
{
	va_list		args;
	DB_RESULT	rc;

	va_start(args, fmt);

	rc = t_db_vselect(con, fmt, args);

	while ((DB_RESULT)T_DB_DOWN == rc)
	{
		DBclose(con);
		DBconnect(con, T_DB_CONNECT_NORMAL);

		if ((DB_RESULT)T_DB_DOWN == (rc = t_db_vselect(con, fmt, args)))
		{
			zlog_error(z_cate, "database is down: retrying in %d seconds", T_DB_WAIT_DOWN);
			connection_failure = 1;
			Sleep(T_DB_WAIT_DOWN*1000);
		}
	}

	va_end(args);

	return rc;
}

DB_PARAMBIND DB_Parambind_init(DB_CON con, const char *sql)
{
	return t_param_bind_init(con, sql);
}

int DB_setParambind(DB_PARAMBIND bind, int index, void *value, int length, int type)
{
	return t_set_param_bind(bind, index, value, length, type);
}

int DB_Parambind_execute(DB_PARAMBIND bind)
{
	return t_param_bind_execute(bind);
}
