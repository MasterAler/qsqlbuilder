#include "Query.h"
#include "Config.h"
#include "Selector.h"

#include <QSqlDatabase>
#include <QSqlRecord>
#include <QSqlError>
#include <QUuid>

#include <QDebug>

class QueryPrivate
{
public:
    QueryPrivate(const QString& tableName)
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
                qCritical() << m_DB.lastError();
        }

        QSqlRecord columns = m_DB.record(m_tableName);
        for(int i=0; i < columns.count(); ++i)
            m_columnNames << QString("%1.%2").arg(m_tableName, columns.fieldName(i));
    }

    mutable QSqlDatabase    m_DB;
    QString                 m_tableName;
    QStringList             m_columnNames;
};

/**********************************************************************************/

Query::Query(const QString& tableName)
    : d_ptr(new QueryPrivate(tableName))
{ }

Query::~Query()
{
    Q_D(Query);
    d->m_DB.close();

}

QSqlQuery Query::performSQL(const QString& sql) const
{
    Q_D(const Query);

    QSqlQuery sqlQuery(d->m_DB);
    sqlQuery.exec(sql);

    qDebug() << sqlQuery.lastQuery();

    return sqlQuery;
}

QString Query::tableName() const
{
    Q_D(const Query);
    return d->m_tableName;
}

QStringList Query::columnNames() const
{
    Q_D(const Query);
    return d->m_columnNames;
}

QSharedPointer<Selector> Query::select(const QStringList& fields)
{
    return Selector::create(this, fields);
}

QSqlDatabase& Query::defaultConnection()
{
    static QSqlDatabase dbInstance = QSqlDatabase::addDatabase(Config::DRIVER, QUuid::createUuid().toString());
    return dbInstance;
}

