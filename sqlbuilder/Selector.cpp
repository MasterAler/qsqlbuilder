#include "Selector.h"

#include "Query.h"
#include <QDebug>

class SelectorPrivate
{
public:
    SelectorPrivate(const Query* q, const QStringList& fields)
        : m_query(q)
        , m_fields(!fields.isEmpty() ? fields : q->columnNames())
        , m_where("")
        , m_limit("")
        , m_order("")
        , m_having("")
        , m_groupBy("")
        , m_offset("")
    { }

    const Query*        m_query;
    const QStringList   m_fields;

    QString             m_where;
    QString             m_limit;
    QString             m_order;

    QString             m_having;
    QString             m_groupBy;
    QString             m_offset;
};

/***************************************************************************************/

const QString Selector::SELECT_SQL { "SELECT %1 FROM %2 %3 WHERE %4 %5;" };

Selector::Selector(const Query* q, const QStringList& fields)
    : d_ptr(new SelectorPrivate(q, fields))
{ }

Selector::~Selector()
{ }

QSharedPointer<Selector> Selector::create(const Query* q, const QStringList& fields)
{
    return QSharedPointer<Selector>::create(q, fields);
}

QSharedPointer<Selector> Selector::where(OP::Clause&& clause)
{
    Q_D(Selector);
    d->m_where = std::move(clause).getSQl();
    return sharedFromThis();
}

//QSharedPointer<Selector> Selector::where(WhereConcat::ConcatType concat, const WhereClause& clauses)
//{
//    Q_D(Selector);

//    QStringList whereParts;
//    for(const auto& clause : clauses)
//    {
//        QString key = std::get<0>(clause);
//        QVariant clauseValue = std::get<2>(clause);

//        switch (std::get<1>(clause))
//        {
//            case OP::EQ:
//                whereParts << QString("\"%1\"='%2'").arg(key).arg(clauseValue.toString().replace('\'', "''"));
//                break;

//            case OP::LT:
//                whereParts << QString("\"%1\"<'%2'").arg(key).arg(clauseValue.toString().replace('\'', "''"));
//                break;

//            case OP::GT:
//                whereParts << QString("\"%1\">'%2'").arg(key).arg(clauseValue.toString().replace('\'', "''"));
//                break;

//            case OP::IN:
//            {
//                QStringList vals;
//                for(QString& val: clauseValue.toStringList())
//                    vals << QString("'%1'").arg(val.replace('\'', "''"));

//                whereParts << QString("\"%1\" IN (%2)").arg(key, vals.join(','));
//                break;
//            }

//            case OP::NOT_IN:
//            {
//                QStringList vals;
//                for(QString& val: clauseValue.toStringList())
//                    vals << QString("'%1'").arg(val.replace('\'', "''"));

//                whereParts << QString("\"%1\" NOT IN (%2)").arg(key, vals.join(','));
//                break;
//            }
//        }
//    }

//    const QString cancatStr = QString(" %1 ").arg(QVariant::fromValue(concat).toString());
//    if (!d->m_where.isEmpty())
//        d->m_where.append(cancatStr);

//    d->m_where.append(whereParts.join(cancatStr));

//    return sharedFromThis();
//}

QSharedPointer<Selector> Selector::limit(int count)
{
    Q_D(Selector);
    d->m_limit = count > 0
                    ? QString("LIMIT %1").arg(count)
                    : "";

    return sharedFromThis();
}

QSharedPointer<Selector> Selector::orderBy(const QString& field, Order::OrderType selectOrder)
{
    Q_D(Selector);
    d->m_order = QString("ORDER BY %1 %2").arg(field).arg(QVariant::fromValue(selectOrder).toString());

    return sharedFromThis();
}

QSharedPointer<Selector> Selector::groupBy(const QString& field)
{
    Q_D(Selector);
    d->m_groupBy = QString("GROUP BY %1").arg(field);

    return sharedFromThis();
}

QSharedPointer<Selector> Selector::having(const QString& havingClause)
{
    Q_D(Selector);
    d->m_having = QString("HAVING %1").arg(havingClause);

    return sharedFromThis();
}

QSharedPointer<Selector> Selector::offset(int offset)
{
    Q_D(Selector);
    d->m_offset = QString("OFFSET %1").arg(offset);

    return sharedFromThis();
}

QVariant Selector::perform()
{
    Q_D(Selector);
    QVariantList result;

    QStringList tail = QStringList()
                            << d->m_groupBy
                            << d->m_having
                            << d->m_order
                            << d->m_limit
                            << d->m_offset;

    QString sql = Selector::SELECT_SQL
                    .arg(d->m_fields.join(", "))
                    .arg(d->m_query->tableName())
                    .arg("") //TODO: support join
                    .arg(d->m_where.isEmpty() ? "True" : d->m_where) // TODO: WHERE
                    .arg(tail.join(" "));

    QSqlQuery q = d->m_query->performSQL(sql);
    while(q.next())
    {
        QVariantMap resultRow;
        for(const QString& field:  d->m_fields)
            resultRow[field] = q.value(field);

        result.append(resultRow);
    }

    return result;
}
