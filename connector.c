#include "connector.h"

MYSQL *lpSqlGetConnector(const char *lpcServer, const char *lpcUser, const char *lpcPassword, const char *lpcDatabase)
{
	MYSQL *connector;

	connector = mysql_init(NULL);

	if (!mysql_real_connect(connector, lpcServer, lpcUser, lpcPassword, lpcDatabase, 0, NULL, 0))
	{
		connector = NULL;
	}

	return connector;
};

void vDestroyConnector(MYSQL **connector)
{
	mysql_close(*connector);
}
