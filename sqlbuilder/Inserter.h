#pragma once

#include <memory>
#include <QVariant>

QT_FORWARD_DECLARE_CLASS(Query)
QT_FORWARD_DECLARE_CLASS(InserterPrivate)
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
    static const QString INSERT_SQL;
    std::unique_ptr<InserterPrivate> impl;
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
