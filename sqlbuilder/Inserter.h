#pragma once

#include <memory>
#include <QVariant>

QT_FORWARD_DECLARE_CLASS(Query)
QT_FORWARD_DECLARE_CLASS(InserterPrivate)

class Inserter
{
    Q_GADGET
    Q_DISABLE_COPY(Inserter)
public:
    Inserter(const Query* q, const QStringList& fields, const QVariantList& data);
    ~Inserter();

    Inserter(Inserter&&) = default;
    Inserter& operator=(Inserter&&) = default;

private:
    static const QString INSERT_SQL;
    std::unique_ptr<InserterPrivate> impl;
};
