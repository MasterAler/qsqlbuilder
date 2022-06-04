#pragma once

#include <memory>

#include <QObject>
#include <QScopedPointer>
#include <QSqlQuery>

QT_FORWARD_DECLARE_CLASS(QueryPrivate)
QT_FORWARD_DECLARE_CLASS(Selector)
QT_FORWARD_DECLARE_CLASS(Inserter)

class Query
{
    Q_GADGET
    Q_DECLARE_PRIVATE(Query)
    Q_DISABLE_COPY(Query)
public:
    explicit Query(const QString& tableName = QString(), const QString& pkey = QString());
    ~Query();

    static void setQueryLoggingEnabled(bool enabled);

    Selector select(const QStringList& fields = QStringList()) const;

    Inserter insert(const QStringList& fields) const;

public:
    QSqlQuery performSQL(const QString& sql) const;

    QString tableName() const;

    QString primaryKeyName() const;

    QStringList columnNames() const;

private:
    static QSqlDatabase& defaultConnection();
    static bool LOG_QUERIES;

private:
    QScopedPointer<QueryPrivate> d_ptr;
};
