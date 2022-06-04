#pragma once

#include <QEnableSharedFromThis>
#include <QVariant>

QT_FORWARD_DECLARE_CLASS(Query)
QT_FORWARD_DECLARE_CLASS(SelectorPrivate)

class Selector: public QEnableSharedFromThis<Selector>
{
    Q_GADGET
    Q_DECLARE_PRIVATE(Selector)
    Q_DISABLE_COPY(Selector)
public:
    enum Order
    {
        ASC,
        DESC
    };
    Q_ENUM(Order)

    ~Selector();

    static QSharedPointer<Selector> create(const Query* q, const QStringList& fields);

    QSharedPointer<Selector> limit(int count);

    QSharedPointer<Selector> orderBy(const QString& field, Selector::Order selectOrder);

    QSharedPointer<Selector> having(const QString& havingClause);

    QSharedPointer<Selector> offset(int offset);

    QVariant perform();

private:
    friend QSharedPointer<Selector>;
    explicit Selector(const Query* q, const QStringList& fields);

    static const QString SELECT_SQL;

    QScopedPointer<SelectorPrivate> d_ptr;
};
