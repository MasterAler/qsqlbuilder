#include "Updater.h"

struct Updater::UpdaterPrivate
{
    UpdaterPrivate()
    {}
};

const QString Updater::UPDATE_SQL { "UPDATE %1 SET %2 WHERE %3" };

/***************************************************************************************/

Updater::Updater(const Query* q, const QVariantMap& updateValues)
    : impl(new UpdaterPrivate)
{

}

Updater::~Updater()
{ }
