#pragma once

#include <memory>

#include <QObject>
#include <QScopedPointer>
#include <QSqlQuery>

QT_FORWARD_DECLARE_CLASS(QueryPrivate)
QT_FORWARD_DECLARE_CLASS(Selector)

class Query
{
    Q_GADGET
    Q_DECLARE_PRIVATE(Query)
    Q_DISABLE_COPY(Query)
public:
    explicit Query(const QString& tableName = QString());
    ~Query();

    std::unique_ptr<Selector> select(const QStringList& fields = QStringList()) const;

public:
    QSqlQuery performSQL(const QString& sql) const;

    QString tableName() const;

    QStringList columnNames() const;

private:
    static QSqlDatabase& defaultConnection();

private:
    QScopedPointer<QueryPrivate> d_ptr;
};
