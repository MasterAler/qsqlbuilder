#include "Query.h"
#include "Config.h"
#include "Selector.h"
#include "Inserter.h"

#include <QSqlDatabase>
#include <QSqlRecord>
#include <QSqlError>
#include <QSqlIndex>
#include <QUuid>

#include <QDebug>

class QueryPrivate
{
public:
    QueryPrivate(const QString& tableName, const QString& pkey)
        : m_DB(Query::defaultConnection())
        , m_tableName(tableName)
    {
        m_DB.setDatabaseName(Config::DBNAME);
        m_DB.setHostName(Config::HOSTNAME);
        m_DB.setUserName(Config::USERNAME);
        m_DB.setPassword(Config::PASSWORD);

        if (!m_DB.isOpen())
        {
            if (!m_DB.open()) // TODO: error handling on failed connection
                qCritical() << m_DB.lastError();
        }

        m_pkey = pkey.isEmpty() ? m_DB.primaryIndex(m_tableName).fieldName(0) : pkey;

        QSqlRecord columns = m_DB.record(m_tableName);
        for(int i=0; i < columns.count(); ++i)
            m_columnNames << QString("%1.%2").arg(m_tableName, columns.fieldName(i));
    }

    mutable QSqlDatabase    m_DB;
    QString                 m_tableName;

    QString                 m_pkey;
    QStringList             m_columnNames;
};

bool Query::LOG_QUERIES { false };

/**********************************************************************************/

Query::Query(const QString& tableName, const QString& pkey)
    : d_ptr(new QueryPrivate(tableName, pkey))
{ }

Query::~Query()
{
    Q_D(Query);
    d->m_DB.close();

}

void Query::setQueryLoggingEnabled(bool enabled)
{
    Query::LOG_QUERIES = enabled;
}

QSqlQuery Query::performSQL(const QString& sql) const
{
    Q_D(const Query);

    QSqlQuery sqlQuery(d->m_DB);
    sqlQuery.exec(sql);

    if (Query::LOG_QUERIES)
        qDebug() << sqlQuery.lastQuery();

    return sqlQuery;
}

QString Query::tableName() const
{
    Q_D(const Query);
    return d->m_tableName;
}

QString Query::primaryKeyName() const
{
    Q_D(const Query);
    return d->m_pkey;
}

QStringList Query::columnNames() const
{
    Q_D(const Query);
    return d->m_columnNames;
}

Selector Query::select(const QStringList& fields) const
{
    return Selector(this, fields);
}

Inserter Query::insert(const QStringList& fields) const
{
    return Inserter(this, fields);
}

QSqlDatabase& Query::defaultConnection()
{
    static QSqlDatabase dbInstance = QSqlDatabase::addDatabase(Config::DRIVER, QUuid::createUuid().toString());
    return dbInstance;
}

