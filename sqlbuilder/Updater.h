#pragma once

#include <memory>
#include <QString>

#include "Where.h"
QT_FORWARD_DECLARE_CLASS(Query)

class Updater
{
    Q_DISABLE_COPY(Updater)
public:
    Updater(const Query* q, const QVariantMap& updateValues);
    ~Updater();

private:
    struct UpdaterPrivate;
    std::unique_ptr<UpdaterPrivate> impl;

    static const QString UPDATE_SQL;
};

