#include "Selector.h"
#include "Query.h"

#include <QSqlQuery>
#include <QSqlRecord>
#include <QSet>

struct Selector::SelectorPrivate
{
    SelectorPrivate(const Query* q, const QStringList& fields)
        : m_query(q)
        , m_fields(!fields.isEmpty() ? fields : q->columnNames())
        , m_join{""}
        , m_joinTable{""}
        , m_joinDisambigToOther(false)
        , m_where{""}
        , m_limit{""}
        , m_order{""}
        , m_having{""}
        , m_groupBy{""}
        , m_offset{""}
    { }

    const Query*        m_query;
    QStringList         m_fields;

    QString             m_join;
    QString             m_joinTable;
    bool                m_joinDisambigToOther;

    QString             m_where;
    QString             m_limit;
    QString             m_order;

    QString             m_having;
    QString             m_groupBy;
    QString             m_offset;

    void resolveColumnDisambiguation()
    {
        // This should resolve disambiduation in column names
        QSet<QString> thisColumnSet  = QSet<QString>::fromList(m_query->columnNames());
        QSet<QString> otherColumnSet = QSet<QString>::fromList(Query::tableColumnNames(m_joinTable));
        thisColumnSet.intersect(otherColumnSet);

        for(int i = 0; i < m_fields.count(); ++i)
        {
            QString fieldName = m_fields[i];
            if (thisColumnSet.contains(fieldName))
            {
                QString resolutionFieldName = QString("%1.%2")
                                                .arg(!m_joinDisambigToOther
                                                            ? m_query->tableName()
                                                            : m_joinTable)
                                                .arg(fieldName);

                m_where.replace(fieldName, resolutionFieldName);
                m_where.replace('.', "\".\"");
                m_fields.replace(i, resolutionFieldName);
            }
        }
    }
};

/***************************************************************************************/

const QString Selector::SELECT_SQL { "SELECT %1 FROM %2 %3 WHERE %4 %5;" };

Selector::Selector(const Query* q, const QStringList& fields)
    : impl(new SelectorPrivate(q, fields))
{ }

Selector::~Selector()
{ }

Selector Selector::join(const QString& otherTable, const std::pair<QString, QString>& joinColumns, Join::JoinType joinType, bool resolveDisambigToOther)
{
    impl->m_joinTable = otherTable;
    impl->m_joinDisambigToOther = resolveDisambigToOther;
    impl->m_join = QString("%1 JOIN %2 on %3")
                        .arg(QVariant::fromValue(joinType).toString())
                        .arg(otherTable)
                        .arg(QString("\"%1\".\"%2\"=\"%3\".\"%4\"")
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
    impl->resolveColumnDisambiguation();

    const QStringList tail = QStringList()
                            << impl->m_groupBy
                            << impl->m_having
                            << impl->m_order
                            << impl->m_limit
                            << impl->m_offset;

    const QString sql = Selector::SELECT_SQL
                    .arg(impl->m_fields.join(", "))
                    .arg(impl->m_query->tableName())
                    .arg(impl->m_join)
                    .arg(impl->m_where.isEmpty() ? "True" : impl->m_where)
                    .arg(tail.join(" "));

    QSqlQuery q = impl->m_query->performSQL(sql);
    QSqlRecord r = q.record();

    while(q.next())
    {
        QVariantMap resultRow;
        for(int i = 0; i < r.count(); ++i)
            resultRow[r.fieldName(i)] = q.value(i);

        result.append(resultRow);
    }

    return result;
}
