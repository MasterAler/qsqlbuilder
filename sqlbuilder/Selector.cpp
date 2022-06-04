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
        , m_where{""}
        , m_limit{""}
        , m_order{""}
        , m_having{""}
        , m_groupBy{""}
        , m_offset{""}
    { }

    struct JoinPart
    {
        QString     m_sql;
        QString     m_joinTable;
        bool        m_joinDisambigToOther;
    };

    const Query*        m_query;
    QStringList         m_fields;

    QString             m_where;
    QString             m_limit;
    QString             m_order;

    QString             m_having;
    QString             m_groupBy;
    QString             m_offset;

    QList<JoinPart>     m_joinParts;

    // This should resolve disambiduation in column names
    void resolveColumnDisambiguation()
    {
        QSet<QString> thisColumnSet  = QSet<QString>::fromList(m_query->columnNames());
        for (const auto& part: m_joinParts)
        {
            QSet<QString> otherColumnSet = QSet<QString>::fromList(m_query->tableColumnNames(part.m_joinTable));
            otherColumnSet.intersect(thisColumnSet);

            for(int i = 0; i < m_fields.count(); ++i)
            {
                QString fieldName = m_fields[i];
                if (otherColumnSet.contains(fieldName))
                {
                    QString resolutionFieldName = QString("%1.%2")
                                                    .arg(!part.m_joinDisambigToOther
                                                                ? m_query->tableName()
                                                                : part.m_joinTable)
                                                    .arg(fieldName);
                    // hacky a bit, but IT WORKS
                    m_where.replace(fieldName, resolutionFieldName);
                    m_where.replace('.', "\".\"");
                    m_fields.replace(i, resolutionFieldName);
                }
            }
        }
    }

    //-------

    QString getJoinTail() const
    {
        QString result;

        if (!m_joinParts.isEmpty())
        {
            QStringList joiner;
            for (const auto& part: m_joinParts)
                joiner << part.m_sql;
            result = joiner.join(' ');
        }

        return result;
    }
};

/***************************************************************************************/

const QString Selector::SELECT_SQL { "SELECT %1 FROM %2 %3 WHERE %4 %5;" };

Selector::Selector(const Query* q, const QStringList& fields)
    : impl(new SelectorPrivate(q, fields))
{ }

Selector::~Selector()
{ }

Selector::Selector(Selector &&) = default;

Selector Selector::join(const QString& otherTable, const std::pair<QString, QString>& joinColumns, Join::JoinType joinType, bool resolveDisambigToOther)
{
    SelectorPrivate::JoinPart part;

    part.m_joinTable = otherTable;
    part.m_joinDisambigToOther = resolveDisambigToOther;
    part.m_sql = QString("%1 JOIN %2 on %3")
                        .arg(QVariant::fromValue(joinType).toString())
                        .arg(otherTable)
                        .arg(QString("\"%1\".\"%2\"=\"%3\".\"%4\"")
                                .arg(impl->m_query->tableName(), joinColumns.first, otherTable, joinColumns.second));

    impl->m_joinParts.append(part);
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
                    .arg(impl->getJoinTail())
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
