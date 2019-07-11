#include "Query.h"

#include "Config.h"
#include "Selector.h"
#include "Inserter.h"
#include "Deleter.h"
#include "Updater.h"

#include <QSqlDatabase>
#include <QSqlRecord>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlIndex>
#include <QUuid>

#include <QDebug>

struct Query::QueryPrivate
{
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
            if (!m_DB.open())
                throw std::runtime_error("Database was not opened! =(");
        }

        m_pkey = pkey.isEmpty() ? m_DB.primaryIndex(m_tableName).fieldName(0) : pkey;

        QSqlRecord columns = m_DB.record(m_tableName);
        for(int i = 0; i < columns.count(); ++i)
            m_columnNames << columns.fieldName(i);
    }

    QSqlDatabase            m_DB;
    QString                 m_tableName;

    QString                 m_pkey;
    QStringList             m_columnNames;

    QSqlError               m_lastError;
};

bool Query::LOG_QUERIES { false };

/**********************************************************************************/

Query::Query(const QString& tableName, const QString& pkey)
    : impl(new QueryPrivate(tableName, pkey))
{ }

Query::~Query()
{
    impl->m_DB.close();
}

void Query::setQueryLoggingEnabled(bool enabled)
{
    Query::LOG_QUERIES = enabled;
}

QSqlQuery Query::performSQL(const QString& sql) const
{
    QSqlQuery sqlQuery(impl->m_DB);
    sqlQuery.exec(sql);

    if (Query::LOG_QUERIES)
        qDebug() << sqlQuery.lastQuery();

    impl->m_lastError = sqlQuery.lastError();
    return sqlQuery;
}

QSqlError Query::lastError() const
{
    return impl->m_lastError;
}

bool Query::hasError() const
{
    return impl->m_lastError.isValid();
}

QString Query::tableName() const
{
    return impl->m_tableName;
}

QString Query::primaryKeyName() const
{
    return impl->m_pkey;
}

QStringList Query::columnNames() const
{
    return impl->m_columnNames;
}

QStringList Query::tableColumnNames(const QString& tableName) const
{
    QStringList result;

    QSqlRecord columns = impl->m_DB.record(tableName);
    for(int i=0; i < columns.count(); ++i)
        result << columns.fieldName(i);

    return result;
}

Selector Query::select(const QStringList& fields) const
{
    return Selector(this, fields);
}

Inserter Query::insert(const QStringList& fields) const
{
    return Inserter(this, fields);
}

Deleter Query::delete_(OP::Clause&& whereClause) const
{
    return Deleter(this, std::forward<OP::Clause>(whereClause));
}

Updater Query::update(const QVariantMap& updateValues) const
{
    return Updater(this, updateValues);
}

bool Query::transact(std::function<void ()>&& operations)
{
    bool result;

    if (impl->m_DB.transaction())
    {
        std::move(operations)();

        if (hasError())
        {
            impl->m_DB.rollback();
            result = false;
        }
        else
            result = impl->m_DB.commit();
    }
    else
        return false;

    return result;
}

QSqlDatabase& Query::defaultConnection()
{
    static QSqlDatabase dbInstance = QSqlDatabase::addDatabase(Config::DRIVER, QUuid::createUuid().toString());
    return dbInstance;
}
