#pragma once

#include <memory>
#include <QString>

#include "Where.h"
QT_FORWARD_DECLARE_CLASS(Query)

/*!
 * \brief The Updater class
 * is an UPDATE query generator. Works as all the
 * other generators, supports clauses.
 * IMPORTANT: if you forget to call "where()" and specify some correct update
 * values, you'll get ALL OF YOU TABLE updated. That's how SQL works, but
 * here it is a bit less obvious way to shoot your foot badly.
 */
class Updater
{
    Q_DISABLE_COPY(Updater)
public:
    /*!
     * \brief Updater       -- constructor of the generator, don't use it manually
     * \param q             -- ptr to the Query class, that created it
     * \param updateValues  -- map of values like in "...SET column_name = 'value',...", maps
     * (obviously) column names on values being set
     */
    Updater(const Query* q, const QVariantMap& updateValues);
    ~Updater();

    Updater(Updater&&);
    Updater& operator=(Updater&&) = default;

    /*!
     * \brief where     -- "WHERE ..." clause (see OP namespace for details)
     * \param clause    -- some aggregated clause
     * \return          -- this generator as rvalue to be reused
     */
    Updater where(OP::Clause&& clause) &&;

    /*!
     * \brief perform   -- executes the generated query
     * \return          -- success/failure of the query (affected rows > 0)
     */
    bool perform() &&;

private:
    struct UpdaterPrivate;
    std::unique_ptr<UpdaterPrivate> impl;

    static const QString UPDATE_SQL;
};

