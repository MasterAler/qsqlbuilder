#pragma once

#include <QString>

/*!
 * \brief The Config struct
 * is a primitive wrapper for connection params.
 * It could be somehow more sophisticated, with connection
 * managing and all, but it's not the purpose of the library,
 * so several static variables just store params, so that
 * other classes (Query class to be exact) are not garbaged with them.
 */
struct Config
{
    /*!
     * \brief setConnectionParams -- convenience method, sets all connection params at once
     * \param driver              -- Qt driver choosing string, like "QPSQL"
     * \param hostname            -- obviously, hostname
     * \param dbname              -- obviously, database name
     * \param username            -- obviously, db user name
     * \param password            -- obviously, db user password
     */
    static void setConnectionParams(const QString& driver, const QString& hostname, const QString& dbname
                                    , const QString& username, const QString& password)
    {
        Config::DRIVER   = driver;
        Config::DBNAME   = dbname;
        Config::HOSTNAME = hostname;
        Config::USERNAME = username;
        Config::PASSWORD = password;
    }

    static QString DRIVER;
    static QString DBNAME;
    static QString HOSTNAME;
    static QString USERNAME;
    static QString PASSWORD;
};
