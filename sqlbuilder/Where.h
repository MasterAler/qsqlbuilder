#pragma once

#include <QString>
#include <QVariant>

/*!
 * Here goes a namespase of helpers that implement "WHERE ..." support
 * in all the generators. Can be extended with other conditions, perhaps.
 */
namespace OP
{

/*!
 * \brief The Clause class
 * is an internal class, that translates C++ boolean logic into "WHERE ..." statements.
 * It optimizes nothing, no syntax tree parsing here, so don't abuse your queries with too much
 * of clauses. Executed clause is guaranteed to have the same interpretation when calculated by
 * database as you've written in your sources. Just using braces here =)
 */
class Clause
{
public:
    /*!
     * \brief Clause    -- is a constructor of smth like " col='val' "
     * \param field     -- column name in the clause
     * \param op        -- clause operation (=,>,<, IN, etc.)
     * \param value     -- value in the clause
     */
    Clause(const QString& field, const QString& op, const QString& value)
        : m_sql(QString("\"%1\" %2 %3").arg(field, op, value))
    { }

    /*!
     * \brief operator ! -- is a SQL negation, uses "NOT (...)"
     * \return           -- this mini-generator as rvalue to be reused
     */
    Clause operator!() &&;

    /*!
     * \brief operator ! -- is a SQL logical conjunction, uses "(...) AND (...)"
     * \return           -- this mini-generator as rvalue to be reused
     */
    Clause operator&&(Clause&& other) &&;

    /*!
     * \brief operator ! -- is a SQL logical union, uses "(...) OR (...)"
     * \return           -- this mini-generator as rvalue to be reused
     */
    Clause operator||(Clause&& other) &&;

    /*!
     * \brief getSQl    -- returns the curently generated SQL
     * \return          -- string with the accumulated clause
     */
    QString getSQl() &&;

    /*!
     * \brief escapeValue   -- escapes values due to database rules. Honestly taken from
     * the QSqlDriver class (same idea, written in a more simple way). Has not been tested fully,
     * the target was PostgreSQL, so binary data may not be supported by other DB engines. It's not
     * only used by the clauses, also internally used in other generators.
     * DB identifiers are just escaped like "id" everywhere through the code.
     * \param value         -- value to be escaped propely
     * \return              -- string represetnation of the value to be used
     */
    static QString escapeValue(const QVariant& value);

private:
    QString     m_sql;
};

/*!
 * \brief EQ        -- helper, that constructs " col='val' " clause part
 * \param fieldName -- column name
 * \param value     -- value to be used
 * \return          -- clause entity (see the above class)
 */
Clause EQ(const QString& fieldName, const QVariant& value);

/*!
 * \brief NEQ       -- helper, that constructs " col!='val' " clause part
 * \param fieldName -- column name
 * \param value     -- value to be used
 * \return          -- clause entity (see the above class)
 */
Clause NEQ(const QString& fieldName, const QVariant& value);

/*!
 * \brief LT        -- helper, that constructs " col<'val' " clause part
 * \param fieldName -- column name
 * \param value     -- value to be used
 * \return          -- clause entity (see the above class)
 */
Clause LT(const QString& fieldName, const QVariant& value);

/*!
 * \brief GT        -- helper, that constructs " col>'val' " clause part
 * \param fieldName -- column name
 * \param value     -- value to be used
 * \return          -- clause entity (see the above class)
 */
Clause GT(const QString& fieldName, const QVariant& value);

/*!
 * \brief LE        -- helper, that constructs " col<='val' " clause part
 * \param fieldName -- column name
 * \param value     -- value to be used
 * \return          -- clause entity (see the above class)
 */
Clause LE(const QString& fieldName, const QVariant& value);

/*!
 * \brief GE        -- helper, that constructs " col>='val' " clause part
 * \param fieldName -- column name
 * \param value     -- value to be used
 * \return          -- clause entity (see the above class)
 */
Clause GE(const QString& fieldName, const QVariant& value);

/*!
 * \brief IN        -- helper, that constructs "col IN ('val1', 'val2', ...)" clause part
 * \param fieldName -- column name
 * \param values    -- values to be used
 * \return          -- clause entity (see the above class)
 */
Clause IN(const QString& fieldName, const QVariantList& values);

/*!
 * \brief EQ        -- helper, that constructs "col IS NULL" clause part
 * \param fieldName -- column name
 * \return          -- clause entity (see the above class)
 */
Clause IS_NULL(const QString& fieldName);

} //namespace OP
