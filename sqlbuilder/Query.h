#pragma once

#include <memory>
#include <functional>
#include <QStringList>

#include "Where.h"

QT_FORWARD_DECLARE_CLASS(QSqlDatabase)
QT_FORWARD_DECLARE_CLASS(QSqlQuery)
QT_FORWARD_DECLARE_CLASS(QSqlError)

QT_FORWARD_DECLARE_CLASS(Selector)
QT_FORWARD_DECLARE_CLASS(Inserter)
QT_FORWARD_DECLARE_CLASS(Deleter)
QT_FORWARD_DECLARE_CLASS(Updater)

/*!
 * \brief The Query class
 * is the prime class, that manages everything and creates query classes.
 * Naming is not best perhaps, it shares a connection, performs SQL and 
 * provides access to all the other rvalue-only generators. They are rvalue-only,
 * because reusing them is undefined in terms of common sense, but you can reuse this Query class.
 * It is designed to be used somewhere locally when needed, opens/closes it's shared connection
 * on construction/destruction. Provides all the basics, CRUD + transactions. It is supposed
 * that all tables have primary key, not that it won't work without those, but the classes were
 * tesed on the data where they exist, use-case was the similar.
 */
class Query
{
    Q_DISABLE_COPY(Query)
public:
    /*!
     * \brief Query     -- constructor, opens db connection, throws std::runtime_error if database was not opened
     * \param tableName -- name of the table to be used in the current set of queries 
     * \param pkey      -- primary key name, in case Qt will not be able to determine it
     */
    Query(const QString& tableName = QString(), const QString& pkey = QString());

    /*!
     * \brief ~Query    -- note: the destructors closes the shared connection
     */
    ~Query();
    
    Query(Query&&);
    Query& operator=(Query&&) = default;

    /*!
     * \brief setQueryLoggingEnabled -- globally enables debug logging of SQL queries via qDebug()
     * \param enabled                -- logging enabled/disabled
     */
    static void setQueryLoggingEnabled(bool enabled);

    /*!
     * \brief select    -- creates SELECT query generator
     * \param fields    -- column names in SELECT ... FROM
     * \return          -- select generator
     */
    Selector select(const QStringList& fields = QStringList()) const;

    /*!
     * \brief insert    -- creates INSERT query generator
     * \param fields    -- column names in INSERT INTO tbl (...)
     * \return          -- insert query generator
     */
    Inserter insert(const QStringList& fields) const;

    /*!
     * \brief delete_       -- creates DELETE query generator
     * \param whereClause   -- "WHERE ..." clause (see OP namespace for details)
     * \return              -- delete query generator
     */
    Deleter  delete_(OP::Clause&& whereClause) const;

    /*!
     * \brief update        -- creates UPDATE query generator
     * \param updateValues  -- map of [column : value] to be set
     * \return              -- update query generator
     */
    Updater  update(const QVariantMap& updateValues) const;

    /*!
     * \brief transact      -- executes the given commands in a trancation
     * \param operations    -- some callable, containing queries' execution
     * \return              -- success/failure of the transaction
     */
    bool     transact(std::function<void()>&& operations);

public:
    /*!
     * \brief performSQL -- performs *raw* SQL, because not all use-cases can be covered
     *  in the current library implementation in principle. Also used internally by the generators.
     * \param sql        -- string with SQL query to be executed
     * \return           -- Qt's query object with the state of the query
     */
    QSqlQuery performSQL(const QString& sql) const;

    /*!
     * \brief lastError -- wrapper method for obtaining last error of the last query
     * \return          -- last QSqlQuery's lastError()
     */
    QSqlError lastError() const;

    /*!
     * \brief hasError -- convenience method th check if last query had any errors while executed
     * \return         -- ok/failure
     */
    bool hasError() const;

    /*!
     * \brief tableName -- name of the table, chosen as primary queries' target
     * \return          -- name of the table, of course
     */
    QString tableName() const;

    /*!
     * \brief primaryKeyName -- name of the primary key column, set for the chosen table
     * \return               -- primary key column name
     */
    QString primaryKeyName() const;

    /*!
     * \brief columnNames   -- list of columns' names of the the chosen table
     * \return              -- returns as described above
     */
    QStringList columnNames() const;

    /*!
     * \brief tableColumnNames  -- helper method, can get a list of column's names for arbitrary existing table
     * \param tableName         -- name of the table to be examined
     * \return                  -- returns as supposed
     */
    QStringList tableColumnNames(const QString& tableName) const;

private:
    static QSqlDatabase& defaultConnection();
    static bool LOG_QUERIES;

private:
    struct QueryPrivate;
    std::unique_ptr<QueryPrivate> impl;
};
