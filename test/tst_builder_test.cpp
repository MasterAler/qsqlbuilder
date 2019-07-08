#include <QtTest>

#include <QSqlDatabase>
#include <QSqlRecord>
#include <QSqlError>
#include <QJsonDocument>

#include "Config.h"
#include "Query.h"
#include "Selector.h"

class builder_test : public QObject
{
    Q_OBJECT

public:
    builder_test()
        : m_showDebug(false)
    {}

    ~builder_test()
    {}

private slots:
    void initTestCase();
    void cleanupTestCase();

    void test_raw_sql();
    void test_select_basic();
    void test_star_selection();

    void test_simple_where();
    void test_comlex_where();

private:
    bool m_showDebug;
};

void builder_test::initTestCase()
{
    Config::setConnectionParams("QPSQL", "127.0.0.1", "ksvd4db", "root", "");
}

void builder_test::cleanupTestCase()
{ }

void builder_test::test_raw_sql()
{
    Query query;
    QSqlQuery q = query.performSQL("SELECT _id, name FROM object LIMIT 3;");

    if (m_showDebug)
    {
        while(q.next())
            qInfo() << q.value("_id") << "\t" << q.value("name");
    }

    Q_ASSERT(q.numRowsAffected() > 0);
}

void builder_test::test_select_basic()
{
    auto res = Query("object").select(QStringList() << "_id" << "name")
                                .orderBy("_id", Order::DESC)
                                .limit(3)
                                .offset(20)
                                .perform();
    Q_ASSERT(res.toList().count() == 3);

    if (m_showDebug)
        qInfo() << QJsonDocument::fromVariant(res);
}

void builder_test::test_star_selection()
{
    auto res = Query("object").select()
                                .orderBy("_id", Order::ASC)
                                .limit(5)
                                .perform();
    Q_ASSERT(res.toList().count() == 5);

    if (m_showDebug)
        qInfo() << QJsonDocument::fromVariant(res);
}

void builder_test::test_simple_where()
{
    auto res = Query("object").select(QStringList() << "_id" << "name")
                                .where(OP::EQ("_otype", 2) && (OP::LT("_id", 100) || OP::EQ("guid", "rte")))
                                .perform();
    qInfo() << QJsonDocument::fromVariant(res);
}

void builder_test::test_comlex_where()
{
    auto res = Query("object").select({"_id", "name", "guid"})
            .where(OP::IN("_id", { 2, 31, 25, 10 })).perform();
    qInfo() << QJsonDocument::fromVariant(res);
}

QTEST_MAIN(builder_test)

#include "tst_builder_test.moc"
