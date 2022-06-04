#pragma once

#include <memory>
#include <QVariant>

QT_FORWARD_DECLARE_CLASS(Query)
QT_FORWARD_DECLARE_CLASS(InserterPerformer)

class Inserter
{
    Q_DISABLE_COPY(Inserter)
public:
    Inserter(const Query* q, const QStringList& fields);
    ~Inserter();

    Inserter(Inserter&&) = default;
    Inserter& operator=(Inserter&&) = default;

    InserterPerformer values(const QVariantList& data) &&;

protected:
    struct InserterPrivate;
    std::unique_ptr<InserterPrivate> impl;

    static const QString INSERT_SQL;
};

/*******************************************************************************************/

class InserterPerformer: public Inserter
{
    Q_DISABLE_COPY(InserterPerformer)
public:
    InserterPerformer(const Query* q, const QStringList& fields);
    ~InserterPerformer();

    InserterPerformer(InserterPerformer&&) = default;
    InserterPerformer& operator=(InserterPerformer&&) = default;

    InserterPerformer values(const QVariantList& data) &&;

    QList<int> perform() &&;
};
