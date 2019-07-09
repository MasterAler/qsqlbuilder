#include "Where.h"
#include "Query.h"

#include <QDateTime>

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

QString Clause::escapeValue(const QVariant& value)
{
    /* Honestly taken from QSqlDriver class */
    const QString NULL_STR = "NULL";

    if (value.isNull())
        return NULL_STR;

    QString result;

    switch(value.type())
    {
    case QVariant::Int:
    case QVariant::UInt:
        result = value.toString();
        break;

#if QT_CONFIG(datestring)
    case QVariant::Date:
        result = value.toDate().isValid()
                        ? value.toDate().toString(Qt::ISODate)
                        : NULL_STR;
        break;
    case QVariant::Time:
        result = value.toTime().isValid()
                        ? value.toTime().toString(Qt::ISODate)
                        : NULL_STR;
        break;
    case QVariant::DateTime:
        result = value.toDateTime().isValid()
                        ? value.toDateTime().toString(Qt::ISODate)
                        : NULL_STR;
        break;
#endif
    case QVariant::String:
    case QVariant::Char:
        result = value.toString().trimmed().replace('\'', "''");
        break;

    case QVariant::Bool:
        result = QString::number(value.toBool());
        break;

    case QVariant::ByteArray:
    {
        static const char hexchars[] = "0123456789abcdef";
        QString res;
        QByteArray ba = value.toByteArray();
        for (int i = 0; i < ba.size(); ++i)
        {
            uchar s = static_cast<uchar>(ba[i]);
            res += QLatin1Char(hexchars[s >> 4]);
            res += QLatin1Char(hexchars[s & 0x0f]);
        }
        result = res;
        break;
    }

    default:
        result = value.toString();
    }

    return QString("'%1'").arg(result);
}

Clause EQ(const QString& fieldName, const QVariant& value)
{
    return Clause{fieldName, "=", Clause::escapeValue(value.toString())};
}

Clause NEQ(const QString& fieldName, const QVariant& value)
{
    return Clause{fieldName, "!=", Clause::escapeValue(value.toString())};
}

Clause LT(const QString& fieldName, const QVariant& value)
{
    return Clause{fieldName, "<", Clause::escapeValue(value.toString())};
}

Clause GT(const QString& fieldName, const QVariant& value)
{
    return Clause{fieldName, ">", Clause::escapeValue(value.toString())};
}

Clause LE(const QString& fieldName, const QVariant& value)
{
    return Clause{fieldName, "<=", Clause::escapeValue(value.toString())};
}

Clause GE(const QString& fieldName, const QVariant& value)
{
    return Clause{fieldName, ">=", Clause::escapeValue(value.toString())};
}

Clause IN(const QString& fieldName, const QVariantList& values)
{
    QStringList stringValues;
    for (const QVariant& value : values)
        stringValues << Clause::escapeValue(value);

    return Clause{fieldName, "IN", QString("(%1)").arg(stringValues.join(','))};
}

Clause IS_NULL(const QString& fieldName)
{
    return Clause{fieldName, "IS", Clause::escapeValue(QVariant())};
}

}
