#pragma once

#include <tuple>
#include <memory>

#include <QEnableSharedFromThis>
#include <QVariant>

#include "Where.h"

QT_FORWARD_DECLARE_CLASS(Query)
QT_FORWARD_DECLARE_CLASS(SelectorPrivate)

//---------------------------------- *** here go helpers *** -------------------------------//

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


/*******************************************************************************************/

class Selector
{
    Q_GADGET
    Q_DECLARE_PRIVATE(Selector)
    Q_DISABLE_COPY(Selector)

public:
    explicit Selector(const Query* q, const QStringList& fields);
    Selector(Selector&&) = default;
    ~Selector();

    Selector& operator=(Selector&&) = default;

    Selector* where(OP::Clause&& clause);

    Selector* limit(int count);

    Selector* orderBy(const QString& field, Order::OrderType selectOrder);

    Selector* groupBy(const QString& field);

    Selector* having(const QString& havingClause);

    Selector* offset(int offset);

    QVariant perform();

private:
    static const QString SELECT_SQL;
    QScopedPointer<SelectorPrivate> d_ptr;
};
