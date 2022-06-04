#pragma once

#include <memory>
#include <QVariant>

#include "Where.h"
QT_FORWARD_DECLARE_CLASS(Query)

//--------------------------- *** helpers go here *** ----------------------------------//

class Join
{
    Q_GADGET
public:
    enum JoinType
    {
        INNER,
        LEFT,
        RIGHT,
        CROSS
    };
    Q_ENUM(JoinType)
};

//---

class Order
{
    Q_GADGET
public:
    enum OrderType
    {
        ASC,
        DESC
    };
    Q_ENUM(OrderType)
};

/***************************************************************************************/

class Selector
{
    Q_DISABLE_COPY(Selector)
public:
    explicit Selector(const Query* q, const QStringList& fields);
    ~Selector();

    Selector(Selector&&) = default;
    Selector& operator=(Selector&&) = default;

    Selector join(const QString& otherTable, const std::pair<QString, QString>& joinColumns, Join::JoinType joinType);

    Selector where(OP::Clause&& clause) &&;

    Selector limit(int count) &&;

    Selector orderBy(const QString& field, Order::OrderType selectOrder) &&;

    Selector groupBy(const QString& field) &&;

    Selector having(const QString& havingClause) &&;

    Selector offset(int offset) &&;

    QVariantList perform() &&;

private:
    struct SelectorPrivate;
    std::unique_ptr<SelectorPrivate> impl;

    static const QString SELECT_SQL;
};
