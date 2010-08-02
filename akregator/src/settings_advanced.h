#ifndef AKREGATOR_SETTINGS_ADVANCED_H
#define AKREGATOR_SETTINGS_ADVANCED_H

#include "settings_advancedbase.h"

#include <tqmap.h>

class TQString;
class TQWidget;

namespace Akregator {

namespace Backend
{
    class StorageFactory;
}

class SettingsAdvanced : public SettingsAdvancedBase
{
    Q_OBJECT	

    public:
        SettingsAdvanced(TQWidget* parent=0, const char* name=0);

        /** returns the key of the currently selected factory */
        TQString selectedFactory() const;
        
        void selectFactory(const TQString& key);
        
    public slots:
        
        void slotConfigureStorage();
        void slotFactorySelected(int);
        
    private:
        TQMap<int,Backend::StorageFactory*> m_factories;
        TQMap<TQString, int> m_keyPos;
};

} // namespace Akregator

#endif //AKREGATOR_SETTINGS_ADVANCED_H
