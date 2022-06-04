# QSqlBuilder

A small C++11 library, built over Qt, that provides rapid building & executing of SQL queries, hiding raw query text. 

### What is it? What for?

Well, here is a simple & easy-to-use (I hope!) SQL query builder using Qt's types and database interaction classes.
It uses all the same QSqlDatabase, QSqlQuery, etc., but you'll be saved the effort of writing 100500 strings with SQL (*and checking errors in each of them*) in your sources.
Most of the documentation is provided as Doxxygen comments, unit-test subproject also demonstates the usage.

For some sophisticated purposed you'd probably like to use some ORM, for example an excellent [sqlpp11](https://github.com/rbock/sqlpp11), but sometimes you *just don't need that much*. That could be a fine time-saver in the case.

When a thing like the descibed here could be of use:

* You are writing a small Qt backend (whatever reason there be, ex.: small application service with API, that was my case)
* Your Qt client is small and doesn't need a real ORM (ex.: SQLITE used)
* You need code mocking or *fast implementing* of smth.
* You are going to change your database structure a lot later and that's where a good C++ ORM will bring you pain 

In other languages there exist reflection, LINQ (in C#) and other conveniences. But in C++ a small custom builder can help instead.
Basic CRUD (create + read + update + delete) are implemented, by "create" an INSERT query is meant.

## Usage

There is a `Query` class, which is supposed to be used locally/on demand/once per set of requests. Should not be a global state, anyway it's instances share the connection, opening and closing it on costruction/destruction, other classes are intenal and designed to be rvalue-only. Thread-safety is questionable, it is equal to the thread-safety of QSqlDatabase class. Of course, the generators consist of string manipulations only, that's pretty safe, but if you start a transaction in one thread (thransactions are supported) and perform a SELECT in the ither -- that should cause a failure. Exactly as it does with the QSqlDatabase. You can still have a `Query` lvalue instance (it is move-constructible), no need to reopen connection every time. By the way, despite the methods returning some internall classes all the time, it won't cost you much in terms of performance, the classes are not only lightweight but also heavily use copy elision everywhere.

 The rest is better shown by example.

### Configuration

```cpp
Config::setConnectionParams("QPSQL", "127.0.0.1", "db_name", "root", ""); // specify your connection parameters here
Query::setQueryLoggingEnabled(true); // enable/disable logging via qDebug()
```
Logging is essentially useful to check the generated SQL for better understanding the concept, it's likely that examples do not cover all the caveats.

### Raw SQL (something too complex to be generated)

```cpp
QSqlQuery q = Query().performSQL("SELECT _id, name FROM my_table LIMIT 3;"); 
```
Should be something *really* complex, then use a plin QSqlQuery, as usually, the connection would be already configured.

### Select

```cpp
auto res = Query("my_table")
                .select({"id", "name"})
                .where(OP::EQ("some_field", "some_value") && OP::LE("id", 1234)) // WHERE ("some_field" = 'some_value') AND ("id" <= 1234)
                .orderBy("id", Order::DESC)
                .limit(3)
                .offset(20)
                .perform();

auto res = Query("my_table")
                .select({"id", "name", "guid"})
                .where(OP::IN("id", {23, 55, 66, 77}) || !OP::IS_NULL("some_key")) // WHERE ("id" IN ('23', '55', '66', '77')) OR NOT("some_key" IS NULL)
                .orderBy("id", Order::ASC)
                .perform();
``` 
You'll get a `QVariantList<QVariantMap>`, each map contains same column names as specified in `select()`, aliases ("col AS smth") are also supported. Actually, aliases should save you from trying to figure out, how the `join()` is working in details.

NOTE: the WHERE part is implemented cpp-style, it generates lots of braces, but that is how your natural cpp logic is being translated into SQL without surprising permutations. Order of the calls does not matter, except for joins. That WHERE clauses are used by all the generators internally.

### Delete

```cpp
const auto query = Query("my_table"); // yeah, you can have a variable
bool ok = query.delete_(OP::IN("id", {11, 22, 33}).perform();
```
Pretty simple, result means "affected rows > 0";
 
### Update

```cpp
bool ok = query
            .update({{"descr", "OLOLOLO"}})
            .where(OP::LE("id", 100500))
            .perform();
```
Same as DELETE, result means "affected rows > 0";

### Insert

```cpp
auto ids = query
            .insert({"some_number", "guid", "date"})
            .values({42, QUuid::createUuid().toString(), QDate::currentDate()})
            .values({42, QUuid::createUuid().toString(), QDate::currentDate().addDays(3)})
            .values({42, QUuid::createUuid().toString(), QDate::currentDate().addDays(-3)})
            .perform();
```
Returns a list of newly inserted ids. 
NOTE: this functional relies on "... RETURNINF id;" feature support, my target was PostgreSQL. The `Query` class will try to determine the primary key, but if you *really mean something strange* another column can be specified instead, like `Query("my_table", "some_col")`. It is just a string, you can pass there whatever `RETURNING` supports, but a have not tested that option thorougly.

### Transactions

Here is a sample from the project's self-test:

```cpp
auto query = Query(TARGET_TABLE);

const QString GOOD_NAME {"GOOD_TRANSACTION"};
const QString BAD_NAME {"BAD_TRANSACTION"};

bool ok = query.transact([&]{ // don't use default capture in real project
    auto ids = query
            .insert({"_otype", "guid", "name"})
            .values({33, QUuid::createUuid().toString(), GOOD_NAME})
            .values({33, QUuid::createUuid().toString(), GOOD_NAME})
            .perform();

    Q_ASSERT(ids.count() == 2);
    Q_ASSERT(!query.hasError()); // yaeh, error checking works

    bool d_ok = query.delete_(OP::EQ("_id", ids.first())).perform();
    Q_ASSERT(d_ok);
    Q_ASSERT(!query.hasError());
});
Q_ASSERT(ok);

// The transaction has been commited, we can check that there's only one record on the table
auto check1 = query.select().where(OP::EQ("name", GOOD_NAME)).perform();
Q_ASSERT(!query.hasError());
Q_ASSERT(check1.count() == 1);
 ```

 ... and here goes a "bad" transaction, that's been rollbacked:

```cpp
ok = query.transact([&]{
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
});
Q_ASSERT(!ok);

// Nothing's been inserted due to the error, after rollback we can check it
auto check2 = query.select().where(OP::EQ("name", BAD_NAME)).perform();
Q_ASSERT(!query.hasError());
Q_ASSERT(check2.isEmpty()); 
```

You can even write a *nested* transaction, but the behaviour would much depend on the database engine. Once again, it would be similar to the one of QSqlDatabase.
On PostgreSQL, for example, you'll get a warning, but the code would not fail.

Note that the returned success/failure indicated transaction's success/failure. Qt's logic differs a bit, but I cannot imagine how would anyone handle the failure of rollback operation.

### Select with JOINs

```cpp
auto res = query
            .select({"id", "some_text", "name", "guid"})
            .join("other_table_name", {"some_fkey", "id"}, Join::INNER)
            .orderBy("id", Order::ASC)
            .perform();

auto res = query
            .select()
            .join("other_table_name", {"some_fkey", "id"}, Join::INNER, true) // overriding disambiguation resolution
            .orderBy("id", Order::DESC)
            .perform();
```

Well, that is where the tricky part is. While you are having only two joined tables everything is pretty simple -- you specify the parameters and still don't nave to care about methods' call order.
If you are not carefull and column's name disambiguation occurs, it's gonna be resolved in favour of the table, specified in the `Query`'s  constructor by deafult. Another behaviour can be obtained via boolean flag.

But things are different with multiple joins. Actually, `join()` is the only method that doesn't overwrite the state being called more than once, accumulating JOIN parts instead. I cannot imagine from the top of my head WHY would anyone write SQL with multiple JOINs and column disambiguation at the same time, so here works a simple rule -- if you do it's either resolved by default or you *may* set a magic flag again... but wherever you do it, resolution's gonna work only in favour of the table in the first `join()` call, whatever it be.

```cpp
auto res = complex_query
            .select({"_id", "some_text", "name", "guid", "some_date"})
            .join("second_table", {"some_fkey", "_id"}, Join::INNER, true)  // disambig can ONLY be resolved to this table, no matter where is the flag
            .join("third_table", {"date_fkey", "_id"}, Join::INNER, true)
            .where(OP::IN("_id", joinIds))
            .orderBy("_id", Order::ASC)
            .perform();
```
Honestly you should not write queries like that even manually.

### Extras 

```cpp
const auto query = Query("my_table");

auto res = query.select({"*"})   // this means "SELECT * FROM ..." 
            .orderBy("_id", Order::ASC)
            .limit(5)
            .perform();

res = query.select({"*"})   // this means "SELECT id, name, ... FROM ...", all column names are inserted  
            .orderBy("_id", Order::ASC)
            .limit(5)
            .perform();

res = query.select({"COUNT(*) as count"}).perform(); // yeah, you can use functions
Q_ASSERT(!res.isEmpty());
Q_ASSERT(res.first().toMap().contains("count"));

res = query.select({"MAX(_id) as max_id"}).perform(); // or even like that
```

There possible exist some other hacky ways to use the builder, I just did my best to make sure that the worst you'd get is a query error, reported through the `Query` class (exactly as Qt reports it, receiving from the database itself), all the rest builder classes and helpers are hopefully lightweight enought not to bring any unexpected trouble.

## Tests

The tests are numerous but far from being full. QtTest project contains a creation of three tables and performing some queries upon them. You'll need to set your own connection parameters, of course. Uncommenting the cleanup code there can vary usage from debugging to real smoke-test of the functional.


## Requirements 

Honestly, the library has been tested on PostgreSQL only, but most of the SQL being built is simple and should be easily ported to other DB engine
And as mentioned in the title, C++11 suppor is required.

Tested on:
* PostgreSQL 9.4+
* MSVC 2015 / gcc 5.4+
* Qt 5.6+ (actually Qt's version should not be important, mine was Qt 5.10)

## Credits

Extra thanks to Igor Oferkin for the convenient idea of WHERE clauses.

**GL;HF**
