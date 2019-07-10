#include "Selector.h"
#include "Query.h"

#include <QSqlQuery>

struct Selector::SelectorPrivate
{
    SelectorPrivate(const Query* q, const QStringList& fields)
        : m_query(q)
        , m_fields(!fields.isEmpty() ? fields : q->columnNames())
        , m_join{""}
        , m_where{""}
        , m_limit{""}
        , m_order{""}
        , m_having{""}
        , m_groupBy{""}
        , m_offset{""}
    { }

    const Query*        m_query;
    const QStringList   m_fields;

    QString             m_join;
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
    : impl(new SelectorPrivate(q, fields))
{ }

Selector::~Selector()
{ }

Selector Selector::join(const QString& otherTable, const std::pair<QString, QString>& joinColumns, Join::JoinType joinType)
{
    impl->m_join = QString("%1 JOIN %2 on %3")
                        .arg(QVariant::fromValue(joinType).toString())
                        .arg(otherTable)
                        .arg(QString("%1.\"%2\"=%3.\"%4\"")
                                .arg(impl->m_query->tableName(), joinColumns.first, otherTable, joinColumns.second));
    return std::move(*this);
}

Selector Selector::where(OP::Clause&& clause) &&
{
    impl->m_where = std::move(clause).getSQl();
    return std::move(*this);
}

Selector Selector::limit(int count) &&
{
    impl->m_limit = count > 0
                    ? QString("LIMIT %1").arg(count)
                    : "";
    return std::move(*this);
}

Selector Selector::orderBy(const QString& field, Order::OrderType selectOrder) &&
{
    impl->m_order = QString("ORDER BY %1 %2").arg(field).arg(QVariant::fromValue(selectOrder).toString());
    return std::move(*this);
}

Selector Selector::groupBy(const QString& field) &&
{
    impl->m_groupBy = QString("GROUP BY %1").arg(field);
    return std::move(*this);
}

Selector Selector::having(const QString& havingClause) &&
{
    impl->m_having = QString("HAVING %1").arg(havingClause);
    return std::move(*this);
}

Selector Selector::offset(int offset) &&
{
    impl->m_offset = QString("OFFSET %1").arg(offset);
    return std::move(*this);
}

QVariantList Selector::perform() &&
{
    QVariantList result;

    const QStringList tail = QStringList()
                            << impl->m_groupBy
                            << impl->m_having
                            << impl->m_order
                            << impl->m_limit
                            << impl->m_offset;

    const QString sql = Selector::SELECT_SQL
                    .arg(impl->m_fields.join(", "))
                    .arg(impl->m_query->tableName())
                    .arg(impl->m_join) //TODO: support join
                    .arg(impl->m_where.isEmpty() ? "True" : impl->m_where)
                    .arg(tail.join(" "));

    QSqlQuery q = impl->m_query->performSQL(sql);
    while(q.next())
    {
        QVariantMap resultRow;
        for(const QString& field : impl->m_fields)
            resultRow[field] = q.value(field);

        result.append(resultRow);
    }

    return result;
}
