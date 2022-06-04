#include "Where.h"

namespace OP
{

Clause Clause::operator!() &&
{
    m_sql = QString("NOT (%1)").arg(m_sql);
    return std::move(*this);
}

Clause Clause::operator&&(Clause&& other) &&
{
    m_sql = QString("(%1) AND (%2)").arg(m_sql, other.m_sql);
    return std::move(*this);
}

Clause Clause::operator||(Clause&& other) &&
{
    m_sql = QString("(%1) OR (%2)").arg(m_sql, other.m_sql);
    return std::move(*this);
}

QString Clause::getSQl() &&
{
    return std::move(m_sql);
}

Clause EQ(const QString &fieldName, const QVariant &value)
{
    return Clause{fieldName, "=", value};
}

Clause NEQ(const QString &fieldName, const QVariant &value)
{
    return Clause{fieldName, "!=", value};
}

Clause LT(const QString &fieldName, const QVariant &value)
{
    return Clause{fieldName, "<", value};
}

Clause GT(const QString &fieldName, const QVariant &value)
{
    return Clause{fieldName, ">", value};
}

Clause LE(const QString &fieldName, const QVariant &value)
{
    return Clause{fieldName, "<=", value};
}

Clause GE(const QString &fieldName, const QVariant &value)
{
    return Clause{fieldName, ">=", value};
}

Clause IN(const QString &fieldName, const QVariantList &value)
{
    return Clause{fieldName, "IN", QString("(%1)").arg(QVariant(value).toStringList().join(','))};
}

}
