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

private:
    struct InserterPrivate;
    std::unique_ptr<InserterPrivate> impl;

    friend class InserterPerformer;
    static const QString INSERT_SQL;
};

/*******************************************************************************************/

class InserterPerformer
{
    Q_DISABLE_COPY(InserterPerformer)
public:
    ~InserterPerformer();

    InserterPerformer(Inserter&& inserter);

    InserterPerformer(InserterPerformer&&) = default;
    InserterPerformer& operator=(InserterPerformer&&) = default;

    InserterPerformer values(const QVariantList& data) &&;

    QList<int> perform() &&;

private:
    std::unique_ptr<Inserter::InserterPrivate> impl;
};
