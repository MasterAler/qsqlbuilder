#include "Inserter.h"
#include "Query.h"
#include "Where.h"

#include <QSqlQuery>
#include <QDebug>

class InserterPrivate
{
public:
    InserterPrivate(const Query* q, const QStringList& fields)
        : m_query(q)
        , m_fields(fields)
    {}

    const Query*        m_query;
    const QStringList   m_fields;

    QList<QVariantList> m_data;
};

/***************************************************************************************/

const QString Inserter::INSERT_SQL {"INSERT INTO %1 %2 VALUES %3 RETURNING %4; "};

Inserter::Inserter(const Query* q, const QStringList& fields)
    : impl(new InserterPrivate(q, fields))
{ }

Inserter::~Inserter()
{ }

InserterPerformer Inserter::values(const QVariantList& data) &&
{
    return InserterPerformer(impl->m_query, impl->m_fields).values(data);
}

/***************************************************************************************/

InserterPerformer::InserterPerformer(const Query* q, const QStringList& fields)
    : Inserter(q, fields)
{ }

InserterPerformer::~InserterPerformer()
{ }

InserterPerformer InserterPerformer::values(const QVariantList& data) &&
{
    impl->m_data.append(data);
    return std::move(*this);
}

QList<int> InserterPerformer::perform() &&
{
    QList<int> result;

    QStringList valueTail;
    for(const QVariantList& dataTuple : impl->m_data)
    {
        QStringList vBlock;
        for(const QVariant& value : dataTuple)
            vBlock << OP::Clause::escapeValue(value);

        valueTail << QString("(%1)").arg(vBlock.join(','));
    }

    QString sql = Inserter::INSERT_SQL
                    .arg(impl->m_query->tableName())
                    .arg(QString("(%1)").arg(impl->m_fields.join(',')))
                    .arg(valueTail.join(','))
                    .arg(impl->m_query->primaryKeyName());

    QSqlQuery q = impl->m_query->performSQL(sql);
    while(q.next())
        result.append(q.value(0).toInt());

    return result;
}
