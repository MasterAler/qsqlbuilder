#include "Updater.h"
#include "Query.h"

#include<QSqlQuery>

struct Updater::UpdaterPrivate
{
    UpdaterPrivate(const Query* q, const QVariantMap& updateValues)
        : m_query(q)
        , m_updateValues(updateValues)
    {}

    const Query*        m_query;
    const QVariantMap   m_updateValues;

    QString             m_where;
};

const QString Updater::UPDATE_SQL { "UPDATE %1 SET %2 WHERE %3" };

/***************************************************************************************/

Updater::Updater(const Query* q, const QVariantMap& updateValues)
    : impl(new UpdaterPrivate(q, updateValues))
{ }

Updater::~Updater()
{ }

Updater::Updater(Updater &&) = default;

Updater Updater::where(OP::Clause&& clause) &&
{
    impl->m_where = std::move(clause).getSQl();
    return std::move(*this);
}

bool Updater::perform() &&
{
    QStringList setPart;
    for(const QString& key : impl->m_updateValues.keys())
    {
        setPart << QString("\"%1\"=%2")
                               .arg(key)
                               .arg(OP::Clause::escapeValue(impl->m_updateValues[key]));
    }

    const QString sql = Updater::UPDATE_SQL
                    .arg(impl->m_query->tableName())
                    .arg(setPart.join(','))
                    .arg(impl->m_where.isEmpty() ? "True" : impl->m_where);

    QSqlQuery q = impl->m_query->performSQL(sql);
    return q.numRowsAffected() > 0;
}
