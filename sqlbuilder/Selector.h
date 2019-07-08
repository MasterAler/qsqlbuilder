#pragma once

#include <tuple>

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

class Selector: public QEnableSharedFromThis<Selector>
{
    Q_GADGET
    Q_DECLARE_PRIVATE(Selector)
    Q_DISABLE_COPY(Selector)
public:
    ~Selector();

    static QSharedPointer<Selector> create(const Query* q, const QStringList& fields);

    QSharedPointer<Selector> where(OP::Clause&& clause);

    QSharedPointer<Selector> limit(int count);

    QSharedPointer<Selector> orderBy(const QString& field, Order::OrderType selectOrder);

    QSharedPointer<Selector> groupBy(const QString& field);

    QSharedPointer<Selector> having(const QString& havingClause);

    QSharedPointer<Selector> offset(int offset);

    QVariant perform();

private:
    friend QSharedPointer<Selector>;
    explicit Selector(const Query* q, const QStringList& fields);

    static const QString SELECT_SQL;

    QScopedPointer<SelectorPrivate> d_ptr;
};
