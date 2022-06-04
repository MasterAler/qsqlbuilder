#include <QtTest>

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QJsonDocument>
#include <QDebug>
#include <QUuid>

#include "Config.h"
#include "Query.h"
#include "Selector.h"
#include "Inserter.h"
#include "Deleter.h"
#include "Updater.h"

class builder_test : public QObject
{
    Q_OBJECT

public:
    builder_test()
        : m_showDebug(true)
        , TARGET_TABLE("some_object")
        , SECOND_TABLE("other_table")
    {}

    ~builder_test()
    {}

private slots:
    void initTestCase();
    void cleanupTestCase();

    void test_insert_sql();

    void test_raw_sql();
    void test_select_basic();
    void test_star_selection();

    void test_simple_where();
    void test_comlex_where();

    void test_is_null();

    void test_delete_simple();
    void test_insert_delete();

    void test_error_reporting();
    void test_insert_wrong_usage();

    void test_update_simple();
    void test_full_cycle();

    void test_transcations();

    void test_join();

private:
    bool            m_showDebug;

    const QString   TARGET_TABLE;
    const QString   SECOND_TABLE;
};

void builder_test::initTestCase()
{
    /* setup test tables */

    QSqlDatabase conn = QSqlDatabase::addDatabase("QPSQL", "LOL_TEST");
    conn.setHostName("127.0.0.1");
    conn.setDatabaseName("ksvd4db");
    conn.setUserName("root");
    conn.setPassword("");

    if (!conn.open())
        qCritical() << "DATABASE IS NOT AVAILABLE";
    else
    {
        const QString dbRole{"su"};
        QSqlQuery query(conn);

        if (!conn.tables().contains(TARGET_TABLE))
        {
            const QString createSQL = QString("CREATE TABLE %1 \
                                              ( \
                                                  _id serial NOT NULL, \
                                                  _otype integer NOT NULL, \
                                                  _parent integer, \
                                                  guid text NOT NULL, \
                                                  name text NOT NULL, \
                                                  descr text, \
                                                  CONSTRAINT some_object_pkey PRIMARY KEY (_id) \
                                              );").arg(TARGET_TABLE);

            if (!query.exec(createSQL))
            {
                qCritical() << "table1 creation failed: " << query.lastError().text();
                throw std::runtime_error("TEST TABLE1 CREATION FAILED");
            }

            const QString ownerSQL = QString("ALTER TABLE %1 OWNER to %2;").arg(TARGET_TABLE, dbRole);

            if (!query.exec(ownerSQL))
                qCritical() << "owner change failed";

            const QString grantSQL = QString("GRANT ALL ON TABLE %1 TO %2;").arg(TARGET_TABLE, dbRole);

            if (!query.exec(grantSQL))
                qCritical() << "grant role failed";
        }

        if (!conn.tables().contains(SECOND_TABLE))
        {
            const QString createSecondSql = QString("CREATE TABLE %1 \
                                                    ( \
                                                        _id serial NOT NULL, \
                                                        some_text text NOT NULL, \
                                                        some_fkey integer NOT NULL , \
                                                        CONSTRAINT other_table_pkey PRIMARY KEY (_id), \
                                                        CONSTRAINT other_table__some_fkey_fkey FOREIGN KEY (some_fkey) \
                                                        REFERENCES some_object (_id) MATCH SIMPLE \
                                                        ON UPDATE CASCADE \
                                                        ON DELETE CASCADE \
                                                    )").arg(SECOND_TABLE);
            if (!query.exec(createSecondSql))
            {
                qCritical() << "table2 creation failed: " << query.lastError().text();
                throw std::runtime_error("TEST TABLE2 CREATION FAILED");
            }
        }
    }
    qInfo() << "sample table 1 prepared";

    // configure connection 4 library classes
    Config::setConnectionParams("QPSQL", "127.0.0.1", "ksvd4db", "root", "");
    Query::setQueryLoggingEnabled(true);

    Query().performSQL(QString("TRUNCATE TABLE %1 CASCADE;").arg(TARGET_TABLE));
}

void builder_test::cleanupTestCase()
{
    /* Uncomment to make real cleanup, choose appropriate */

//    Query(TARGET_TABLE).performSQL(QString("TRUNCATE TABLE %1;").arg(TARGET_TABLE));
//    Query(TARGET_TABLE).performSQL(QString("DROP TABLE %1;").arg(TARGET_TABLE));
//    Query(TARGET_TABLE).performSQL(QString("DROP TABLE %1;").arg(SECOND_TABLE));
}

void builder_test::test_insert_sql()
{
    auto res = Query(TARGET_TABLE)
            .insert({"_otype", "guid", "name"})
            .values({42, "LOL", "TEST"})
            .perform();

    auto resMulti = Query(TARGET_TABLE)
            .insert({"_otype", "guid", "name"})
            .values({42, "LOL", "TEST"})
            .values({66, "LOL66", "TEST66"})
            .values({77, "LOL77", "TEST77"})
            .perform();

    if (m_showDebug)
    {
        qInfo() << "One insert: " << res;
        qInfo() << "Multi insert: " << resMulti;
    }

    const auto query = Query(TARGET_TABLE);
    try
    {
        const auto q_locker = query.createTransactionLock();

        for(int i=0; i < 30; ++i)
        {
            query
                .insert({"_otype", "guid", "name"})
                .values({ 20 * rand() / RAND_MAX, QUuid::createUuid().toString(), QString("RND %1").arg(i + 1)})
                .perform();
        }
    }
    catch (const std::runtime_error& error)
    {
        qCritical() << "Check inserts and/or transactions";
        qFatal(error.what());
    }
}

void builder_test::test_raw_sql()
{
    Query query;
    QSqlQuery q = query.performSQL(QString("SELECT _id, name FROM %1 LIMIT 3;").arg(TARGET_TABLE));

    if (m_showDebug)
    {
        while(q.next())
            qInfo() << q.value("_id") << "\t" << q.value("name");
    }

    Q_ASSERT(q.numRowsAffected() > 0);
}

void builder_test::test_select_basic()
{
    auto res = Query(TARGET_TABLE).select(QStringList() << "_id" << "name")
                                .orderBy("_id", Order::DESC)
                                .limit(3)
                                .offset(20)
                                .perform();
    Q_ASSERT(res.count() == 3);

    if (m_showDebug)
        qInfo() << QJsonDocument::fromVariant(res);
}

void builder_test::test_star_selection()
{
    auto res = Query(TARGET_TABLE).select()
                                .orderBy("_id", Order::ASC)
                                .limit(5)
                                .perform();
    Q_ASSERT(res.count() == 5);

    if (m_showDebug)
        qInfo() << QJsonDocument::fromVariant(res);
}

void builder_test::test_simple_where()
{
    auto res = Query(TARGET_TABLE).select(QStringList() << "_id" << "name")
                                .where(OP::EQ("_otype", 2) && (OP::LT("_id", 100) || OP::EQ("guid", "rte")))
                                .perform();
    if (m_showDebug)
        qInfo() << QJsonDocument::fromVariant(res);
}

void builder_test::test_comlex_where()
{
    auto res = Query(TARGET_TABLE).select({"_id", "name", "guid"})
            .where(OP::IN("_id", { 2, 31, 25, 10 })).perform();
    qInfo() << QJsonDocument::fromVariant(res);
}

void builder_test::test_is_null()
{
    auto res = Query(TARGET_TABLE)
            .insert({"_otype", "guid", "name"})
            .values({999, "SOME_GUID", "EMPTY_PARENTED"})
            .perform();

    int id = res.first();
    auto check = Query(TARGET_TABLE)
            .select()
            .where(OP::EQ("_id", id) && OP::IS_NULL("_parent"))
            .perform();

    if (m_showDebug)
        qInfo() << QJsonDocument::fromVariant(check);
}

void builder_test::test_delete_simple()
{
    auto target = Query(TARGET_TABLE)
            .select({"_id"})
            .limit(5)
            .orderBy("_id", Order::DESC)
            .perform();
    Q_ASSERT(target.count() == 5);

    QVariantList ids;
    for (const auto& item : target)
        ids << item.toMap()["_id"];

    bool ok = Query(TARGET_TABLE).delete_(OP::IN("_id", ids)).perform();
    Q_ASSERT(ok);
}

void builder_test::test_insert_delete()
{
    auto query = Query(TARGET_TABLE);
    auto id = query
            .insert({"_otype", "name", "guid"})
            .values({123, "GUID_&&%$%$#$_GUID_", "TO_DELETE"})
            .perform();
    Q_ASSERT(id.count() > 0);
    Q_ASSERT(!query.lastError().isValid());

    bool ok = query.delete_(OP::EQ("_id", id.first())).perform();
    Q_ASSERT(ok);
    Q_ASSERT(!query.lastError().isValid());
}

void builder_test::test_error_reporting()
{
    auto no_query = Query("no_table");

    no_query.select().perform();
    Q_ASSERT(no_query.lastError().isValid());

    no_query.insert({"lol"}).values({112, 3333}).perform();
    Q_ASSERT(no_query.lastError().isValid());

    no_query.delete_(OP::GE("dfdf", 33333)).perform();
    Q_ASSERT(no_query.lastError().isValid());

    no_query.update({{"asdf", "asfasf"}}).perform();
    Q_ASSERT(no_query.lastError().isValid());

    // -----------------------------------------------------

    auto query = Query(TARGET_TABLE);

    query.select().where(OP::EQ("fuubar", "sOmEcRaZy_sTuFf")).perform();
    Q_ASSERT(query.lastError().isValid());

    query.select({"_id"}).limit(1).perform();
    Q_ASSERT(!query.lastError().isValid());

    query.delete_(OP::EQ("sdfgsdg", 3333)).perform();
    Q_ASSERT(query.lastError().isValid());

    bool ok = query.delete_(OP::EQ("_id", -11)).perform();
    Q_ASSERT(!ok);
    Q_ASSERT(!query.lastError().isValid());

    query.insert({"olol"}).values({"FUUUU"}).perform();
    Q_ASSERT(query.lastError().isValid());

    query.update({{"sdsdg", 33} , {"hdfdfh", "asfasf"}}).perform();
    Q_ASSERT(query.lastError().isValid());

    query.update({{"sdsdg", 33} , {"hdfdfh", "asfasf"}}).where(OP::GT("_id", 100)).perform();
    Q_ASSERT(query.lastError().isValid());
}

void builder_test::test_insert_wrong_usage()
{
    auto query = Query(TARGET_TABLE);
    query
        .insert({"_otype", "name", "guid"})
        .values({ 42, "HELLO", "WORLD", "OOPS", "EXTRA", "DATA"})
        .perform();
    Q_ASSERT(query.lastError().isValid());

    query
        .insert({"_otype", "name", "guid"})
        .values({ 42, "OH_I_FORGOT_TO_ADD_MORE_DATA"})
        .perform();
    Q_ASSERT(query.lastError().isValid());
}

void builder_test::test_update_simple()
{
    const auto query = Query(TARGET_TABLE);

    auto res = query.select({"_id"}).orderBy("_id", Order::ASC).offset(5).perform();
    int id5 = res[5].toMap()["_id"].toInt();

    bool ok = query
                .update({{"descr", "OLOLOLO"}})
                .where(OP::LE("_id", id5))
                .perform();
    Q_ASSERT(ok);
    Q_ASSERT(!query.lastError().isValid());
}

void builder_test::test_full_cycle()
{
    const auto query = Query(TARGET_TABLE);

    auto ids = query
            .insert({"_otype", "guid", "name"})
            .values({42, QUuid::createUuid().toString(), "CYCLE_TEST1"})
            .values({42, QUuid::createUuid().toString(), "CYCLE_TEST2"})
            .values({42, QUuid::createUuid().toString(), "CYCLE_TEST3"})
            .perform();
    Q_ASSERT(ids.count() == 3);
    Q_ASSERT(!query.hasError());

    QVariantList idData;
    for(const int& id: ids)
        idData << id;

    const QString descr = "THIS_IS_A_DESCRIPTION";
    bool ok = query.update({{"descr", descr}}).where(OP::IN("_id", idData)).perform();
    Q_ASSERT(ok);
    Q_ASSERT(!query.hasError());

    auto data = query.select({"descr"}).where(OP::IN("_id", idData)).perform();
    Q_ASSERT(data.count() == 3);
    Q_ASSERT(!query.hasError());

    for(const auto& val: data)
        Q_ASSERT(val.toMap()["descr"].toString() == descr);

    ok = query.delete_(OP::IN("_id", idData)).perform();
    Q_ASSERT(ok);
    Q_ASSERT(!query.hasError());

    data = query.select({"descr"}).where(OP::IN("_id", idData)).perform();
    Q_ASSERT(data.isEmpty());
    Q_ASSERT(!query.hasError());
}

void builder_test::test_transcations()
{
    const auto query = Query(TARGET_TABLE);

//    const auto errorCode = query.do([]{
//        auto ids = query
//                .insert({"_otype", "guid", "name"})
//                .values({33, QUuid::createUuid().toString(), GOOD_NAME})
//                .values({33, QUuid::createUuid().toString(), GOOD_NAME})
//                .perform();

//        Q_ASSERT(ids.count() == 2);
//        Q_ASSERT(!query.hasError());

//        bool d_ok = query.delete_(OP::EQ("_id", ids.first())).perform();
//        Q_ASSERT(d_ok);
//        Q_ASSERT(!query.hasError());
//    });

    const QString GOOD_NAME {"GOOD_TRANSACTION"};
    {
        auto q_locker = query.createTransactionLock();

        auto ids = query
                .insert({"_otype", "guid", "name"})
                .values({33, QUuid::createUuid().toString(), GOOD_NAME})
                .values({33, QUuid::createUuid().toString(), GOOD_NAME})
                .perform();
        Q_ASSERT(ids.count() == 2);
        Q_ASSERT(!query.hasError());

        bool d_ok = query.delete_(OP::EQ("_id", ids.first())).perform();
        Q_ASSERT(d_ok);
        Q_ASSERT(!query.hasError());
    }
     Q_ASSERT(!query.hasError());

    auto check1 = query.select().where(OP::EQ("name", GOOD_NAME)).perform();
    Q_ASSERT(!query.hasError());
    Q_ASSERT(check1.count() == 1);

    const QString BAD_NAME {"BAD_TRANSACTION"};
    {
        auto q_locker = query.createTransactionLock();

        auto ids = query
                .insert({"_otype", "guid", "name"})
                .values({33, QUuid::createUuid().toString(), BAD_NAME})
                .values({33, QUuid::createUuid().toString(), BAD_NAME})
                .perform();
        Q_ASSERT(ids.count() == 2);
        Q_ASSERT(!query.hasError());

        bool d_ok = query.delete_(OP::EQ("_id_OOPS", ids.first())).perform();
        Q_ASSERT(!d_ok);
        Q_ASSERT(query.hasError());
    }
     Q_ASSERT(query.hasError());

    auto check2 = query.select().where(OP::EQ("name", BAD_NAME)).perform();
    Q_ASSERT(!query.hasError());
    Q_ASSERT(check2.isEmpty());
}

void builder_test::test_join()
{
    const auto query = Query(TARGET_TABLE);

    query.select().join(SECOND_TABLE, {"_id", "some_fkey"}, Join::INNER).perform();
    qDebug() << query.lastError().text();
}

QTEST_MAIN(builder_test)

#include "tst_builder_test.moc"
