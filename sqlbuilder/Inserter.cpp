#include "Inserter.h"



class InserterPrivate
{
public:
    InserterPrivate(const Query* q, const QStringList& fields, const QVariantList& data)
        : m_query(q)
        , m_fields(fields)
        , m_data(data)
    {}

    const Query*        m_query;
    const QStringList   m_fields;

    QVariantList        m_data;
};

/***************************************************************************************/

const QString Inserter::INSERT_SQL {"INSERT INTO %s %s VALUES %s"};

Inserter::Inserter(const Query* q, const QStringList& fields, const QVariantList &data)
    : impl(new InserterPrivate(q, fields, data))
{ }

Inserter::~Inserter()
{ }
