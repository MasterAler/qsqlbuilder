#pragma once

#include <QString>

struct Config
{
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
