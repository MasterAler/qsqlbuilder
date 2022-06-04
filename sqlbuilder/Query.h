#pragma once

#include <memory>
#include <QStringList>

#include "Where.h"

QT_FORWARD_DECLARE_CLASS(QSqlDatabase)
QT_FORWARD_DECLARE_CLASS(QSqlQuery)
QT_FORWARD_DECLARE_CLASS(QSqlError)

QT_FORWARD_DECLARE_CLASS(Selector)
QT_FORWARD_DECLARE_CLASS(Inserter)
QT_FORWARD_DECLARE_CLASS(Deleter)
QT_FORWARD_DECLARE_CLASS(Updater)
QT_FORWARD_DECLARE_CLASS(TransactionLocker)

class Query
{
    Q_DISABLE_COPY(Query)
public:
     Query(const QString& tableName = QString(), const QString& pkey = QString());
    ~Query();

    Query(Query&&) = default;
    Query& operator=(Query&&) = default;

    static void setQueryLoggingEnabled(bool enabled);

    Selector select(const QStringList& fields = QStringList()) const;

    Inserter insert(const QStringList& fields) const;

    Deleter  delete_(OP::Clause&& whereClause) const;

    Updater  update(const QVariantMap& updateValues) const;

    TransactionLocker createTransactionLock() const;

public:
    QSqlQuery performSQL(const QString& sql) const;

    QSqlError lastError() const;

    bool hasError() const;

    QString tableName() const;

    QString primaryKeyName() const;

    QStringList columnNames() const;

private:
    static QSqlDatabase& defaultConnection();
    static bool LOG_QUERIES;

private:
    struct QueryPrivate;
    std::unique_ptr<QueryPrivate> impl;

    friend class TransactionLocker;
};

/*******************************************************************************************/

class TransactionLocker
{
    Q_DISABLE_COPY(TransactionLocker)
public:
    TransactionLocker(const Query* q);
    ~TransactionLocker();

    TransactionLocker(TransactionLocker&&) = default;
    TransactionLocker& operator=(TransactionLocker&&) = default;

private:
    const Query*        m_query;
};
