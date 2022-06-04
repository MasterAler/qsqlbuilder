#pragma once

#include <memory>
#include <QString>

#include "Where.h"
QT_FORWARD_DECLARE_CLASS(Query)

/*!
 * \brief The Deleter class
 * is a DELETE SQL query generator. Supports WHERE clauses,
 * should *never* be used as lvalue or constructed manually.
 * Don't clean tables with it, it is possible, but not efficient.
 */
class Deleter
{
    Q_DISABLE_COPY(Deleter)
public:
    /*!
     * \brief Deleter       -- constructs the generator
     * \param q             -- ptr to the Query class, that created it
     * \param whereClause   -- "WHERE ..." clause (see OP namespace for details)
     */
    Deleter(const Query* q, OP::Clause&& whereClause);
    ~Deleter();

    Deleter(Deleter&&);
    Deleter& operator=(Deleter&&) = default;

    /*!
     * \brief perform   -- executes the generated query
     * \return          -- success/failure of the query (affected rows > 0)
     */
    bool perform() &&;

private:
    struct DeleterPrivate;
    std::unique_ptr<DeleterPrivate> impl;

    static const QString DELETE_SQL;
};
