#pragma once

#include <memory>
#include <QString>

#include "Where.h"

QT_FORWARD_DECLARE_CLASS(Query)
QT_FORWARD_DECLARE_CLASS(DeleterPrivate)

class Deleter
{
    Q_DISABLE_COPY(Deleter)
public:
    Deleter(const Query* q, OP::Clause&& whereClause);
    ~Deleter();

    Deleter(Deleter&&) = default;
    Deleter& operator=(Deleter&&) = default;

    bool perform() &&;

private:
    static const QString DELETE_SQL;
    std::unique_ptr<DeleterPrivate> impl;
};
