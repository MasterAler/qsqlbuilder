#pragma once

#include <memory>
#include <QVariant>

#include "Where.h"
QT_FORWARD_DECLARE_CLASS(Query)

//--------------------------- *** helpers go here *** ----------------------------------//

/*!
 * \brief The Join class
 * is a convenient wrapper for JOIN
 * type enum. Most common are supported.
 */
class Join
{
    Q_GADGET
public:
    /*!
     * \brief The JoinType enum
     * (surprise) enums JOIN types
     */
    enum JoinType
    {
        INNER, // INNER JOIN
        LEFT,  // LEFT JOIN
        RIGHT, // RIGHT JOIN
        CROSS  // CROSS JOIN (if supported)
    };
    Q_ENUM(JoinType)
};

//---

/*!
 * \brief The Order class
 * is a convenient wrapper for ORDER BY
 * type enum. Most common are supported.
 */
class Order
{
    Q_GADGET
public:
    /*!
     * \brief The OrderType enum
     * (surprise) enums ORDER BY options
     */
    enum OrderType
    {
        ASC, // ORDER BY *ASC*
        DESC // ORDER BY *DESC*
    };
    Q_ENUM(OrderType)
};

/***************************************************************************************/

/*!
 * \brief The Selector class
 * is the most complex generator class, the SELECT query generator.
 * Supports basics (check the methods' names), tries to resolve disambiguation in
 * JOINs and multiple JOINs (but do not use both at once, it probably would not fail
 * but it would mean that you're doing something terribly wrong in terms of common sense).
 * Unlike in SQL methods' call order does not matter (if you are not trying something strange
 * with JOINs, as mentioned above). Calling same method more than once rewrites the data with
 * the last parameters (but not with join() -- it stores all the parts). Assume this generator
 * as an implicit state machine (all the generators work that way), but don't think too much about it.
 * If you are carefull, it is even possible to use "SELECT smth AS alias ..." and "SELECT SUM(...)"
 * queries. The returned maps of data have same string keys as the column names you've provided and/or
 * aliases, the disamiguation (I need to repeat it) is *attemted* to be resoled, but don't abuse it,
 * the implementation if far from perfect, mostly because it is a rare use-case.
 * NOTICE: "target table" below means the one you've specified in the Query() constructor.
 */
class Selector
{
    Q_DISABLE_COPY(Selector)
public:
    /*!
     * \brief Selector  -- constructor of the generator, don't use it manually
     * \param q         -- ptr to the Query class, that created it
     * \param fields    --list of column names in SELECT ... FROM
     */
    explicit Selector(const Query* q, const QStringList& fields);
    ~Selector();

    Selector(Selector&&);
    Selector& operator=(Selector&&) = default;

    /*!
     * \brief join                      -- adds "JOIN ..." part, use thoughtfully
     * \param otherTable                -- another table name name to be joined with
     * \param joinColumns               -- column names to join by. TARGET table column name goes FIRST,
     * OTHER table column name goes SECOND. That's how it is designed to work.
     * \param joinType                  -- INNER/LEFT/RIGHT/CROSS join type
     * \param resolveDisambigToOther    -- is a tricky flag for rare use cases. By default if you specify a column in
     * your SELECT with the name, that already exists in the other table being joined, generator supposes you've meant *target* table's
     * column. Setting this flag to 'true' result in generator supposing otherwise. That's how it works with TWO tables. If you try it
     * with multiple JOINs and set the flag to 'true', the generator would resolve the problem column **to the first external table, mentioned in join**.
     * Simply put -- you add multiple join() calls, you set this flag to 'true' in any of them, generator resolves everything to the table name
     * **specified in the first join() call**, whatever it be and wherever you set the flag. Contrintuitional? Yes. But WHY would you want to achieve
     * something THAT strange and curious in your real query? Don't see the purpose, really. So, it's been left that way to fit two purposes:
     * 1) make join of TWO tables fully customizable 2) prevent any unhandled failure in case of misuse
     * \return                          -- this generator as rvalue to be reused
     */
    Selector join(const QString& otherTable, const std::pair<QString, QString>& joinColumns
                  , Join::JoinType joinType, bool resolveDisambigToOther = false);

    /*!
     * \brief where     -- "WHERE ..." clause (see OP namespace for details)
     * \param clause    -- some aggregated clause
     * \return          -- this generator as rvalue to be reused
     */
    Selector where(OP::Clause&& clause) &&;

    /*!
     * \brief limit -- "LIMIT x" part
     * \param count -- limit value
     * \return      -- this generator as rvalue to be reused
     */
    Selector limit(int count) &&;

    /*!
     * \brief orderBy       -- "ORDER BY ... ASC/DESC" part
     * \param field         -- column name for ordering by
     * \param selectOrder   -- ASC/DESC order type
     * \return              -- this generator as rvalue to be reused
     */
    Selector orderBy(const QString& field, Order::OrderType selectOrder) &&;

    /*!
     * \brief groupBy   -- "GROUP BY ..." part
     * \param field     -- column name to be grouped by
     * \return          -- this generator as rvalue to be reused
     */
    Selector groupBy(const QString& field) &&;

    /*!
     * \brief having        -- "HAVING ..." part
     * \param havingClause  -- some manually-written clause as string
     * \return              -- this generator as rvalue to be reused
     */
    Selector having(const QString& havingClause) &&;

    /*!
     * \brief offset    -- "... OFFSET x" part
     * \param offset    -- offset valuse
     * \return          -- this generator as rvalue to be reused
     */
    Selector offset(int offset) &&;

    /*!
     * \brief perform   -- executes the query, returning the data
     * \return          -- list of QVariantMaps with keys similar to columns & aliases provided earlier
     */
    QVariantList perform() &&;

private:
    struct SelectorPrivate;
    std::unique_ptr<SelectorPrivate> impl;

    static const QString SELECT_SQL;
};
