#pragma once

#include <QString>
#include <QVariant>

namespace OP
{

class Clause
{
public:
    Clause(const QString& field, const QString& op, const QString& value)
        : m_sql(QString("\"%1\" %2 %3").arg(field, op, value))
    { }

    Clause operator!() &&;

    Clause operator&&(Clause&& other) &&;

    Clause operator||(Clause&& other) &&;

    QString getSQl() &&;

    static QString escapeValue(const QVariant& value);

private:
    QString     m_sql;
};

Clause EQ(const QString& fieldName, const QVariant& value);

Clause NEQ(const QString& fieldName, const QVariant& value);

Clause LT(const QString& fieldName, const QVariant& value);

Clause GT(const QString& fieldName, const QVariant& value);

Clause LE(const QString& fieldName, const QVariant& value);

Clause GE(const QString& fieldName, const QVariant& value);

Clause IN(const QString& fieldName, const QVariantList& values);

} //namespace OP
