#include "akregatorconfig.h"
#include "settings_advanced.h"
#include "storagefactory.h"
#include "storagefactoryregistry.h"

#include <tqpushbutton.h>
#include <tqstringlist.h>
#include <tqwidget.h>

#include <kcombobox.h>

namespace Akregator {

SettingsAdvanced::SettingsAdvanced(TQWidget* parent, const char* name) : SettingsAdvancedBase(parent, name)
{

    TQStringList backends = Backend::StorageFactoryRegistry::self()->list();
    TQString tname;
    int i = 0;
    TQStringList::Iterator end( backends.end() );
    for (TQStringList::Iterator it = backends.begin(); it != end; ++it)
    {
        m_factories[i] = Backend::StorageFactoryRegistry::self()->getFactory(*it);
        m_keyPos[m_factories[i]->key()] = i;
        cbBackend->insertItem(m_factories[i]->name());
        i++;
    }
    connect(pbBackendConfigure, TQT_SIGNAL(clicked()), this, TQT_SLOT(slotConfigureStorage()));
    connect(cbBackend, TQT_SIGNAL(activated(int)), this, TQT_SLOT(slotFactorySelected(int)));
}

TQString SettingsAdvanced::selectedFactory() const
{
    return m_factories[cbBackend->currentItem()]->key();
}

void SettingsAdvanced::selectFactory(const TQString& key)
{
    cbBackend->setCurrentItem(m_keyPos[key]);
    pbBackendConfigure->setEnabled((m_factories[m_keyPos[key]]->isConfigurable()));
}

void SettingsAdvanced::slotConfigureStorage()
{
    m_factories[cbBackend->currentItem()]->configure();
}

void SettingsAdvanced::slotFactorySelected(int pos)
{
    pbBackendConfigure->setEnabled(m_factories[pos]->isConfigurable());
}

} //namespace Akregator
#include "settings_advanced.moc"
