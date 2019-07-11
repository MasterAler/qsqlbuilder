#include "Deleter.h"
#include "Query.h"

#include <QSqlQuery>
#include <QSqlError>

struct Deleter::DeleterPrivate
{
    DeleterPrivate(const Query *q, OP::Clause&& whereClause)
        : m_query(q)
        , m_where{std::move(whereClause).getSQl()}
    {}

    const Query*        m_query;
    QString             m_where;
};

const QString Deleter::DELETE_SQL { "DELETE FROM %1 WHERE %2;" };

/***************************************************************************************/

Deleter::Deleter(const Query *q, OP::Clause&& whereClause)
    : impl(new DeleterPrivate(q, std::forward<OP::Clause>(whereClause)))
{ }

Deleter::~Deleter()
{ }

Deleter::Deleter(Deleter &&) = default;

bool Deleter::perform() &&
{
    const QString sql = Deleter::DELETE_SQL
                    .arg(impl->m_query->tableName())
                    .arg(impl->m_where.isEmpty() ? "True" : impl->m_where);

    QSqlQuery q = impl->m_query->performSQL(sql);
    return q.numRowsAffected() > 0;
}
