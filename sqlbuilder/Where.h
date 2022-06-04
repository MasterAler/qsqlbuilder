#pragma once

#include <QString>
#include <QVariant>

namespace OP
{

class Clause
{
public:
    //TODO: escape input
    Clause(const QString& field, const QString& op, const QVariant& value)
        : m_sql(QString("\"%1\" %2 '%3'").arg(field, op).arg(value.toString()/*.replace('\'', "''")*/))
    { }

    Clause operator!() &&;

    Clause operator&&(Clause&& other) &&;

    Clause operator||(Clause&& other) &&;

    QString getSQl() &&;

private:
    QString         m_sql;
};

Clause EQ(const QString& fieldName, const QVariant& value);

Clause NEQ(const QString& fieldName, const QVariant& value);

Clause LT(const QString& fieldName, const QVariant& value);

Clause GT(const QString& fieldName, const QVariant& value);

Clause LE(const QString& fieldName, const QVariant& value);

Clause GE(const QString& fieldName, const QVariant& value);

Clause IN(const QString& fieldName, const QVariantList& value);

} //namespace OP
