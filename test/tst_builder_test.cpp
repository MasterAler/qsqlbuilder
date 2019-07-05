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
    {}

    ~builder_test()
    {}

private slots:
    void initTestCase();
    void cleanupTestCase();

    void test_raw_sql();
    void test_select_basic();
    void test_star_selection();
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

//    while(q.next())
//    {
//        qDebug() << q.value("_id") << "\t" << q.value("name");
//    }

    Q_ASSERT(q.numRowsAffected() > 0);
}

void builder_test::test_select_basic()
{
    auto res = Query("object").select(QStringList() << "_id" << "name")
                                ->orderBy("_id", Selector::DESC)
                                ->limit(3)
                                ->offset(20)
                                ->perform();
    Q_ASSERT(res.toList().count() == 3);
//    qInfo() << QJsonDocument::fromVariant(res);
}

void builder_test::test_star_selection()
{
    auto res = Query("object").select()
                                ->orderBy("_id", Selector::ASC)
                                ->limit(5)
                                ->perform();
    Q_ASSERT(res.toList().count() == 5);
//    qInfo() << QJsonDocument::fromVariant(res);
}

QTEST_MAIN(builder_test)

#include "tst_builder_test.moc"
