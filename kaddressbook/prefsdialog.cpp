#include <qlayout.h>
#include <qframe.h>
#include <qvbox.h>
#include <qcheckbox.h>

#include <kapplication.h>
#include <klocale.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klistview.h>

#include "ldapoptionswidgetimpl.h"
#include "kabprefs.h"

#include "prefsdialog.h"

PrefsDialog::PrefsDialog( QWidget *parent )
    : KDialogBase( IconList, i18n("Preferences"), Apply|Ok|Cancel, Ok, parent, 0,
                   false, true )
{
    setupLdapPage();

    readConfig();
}

void PrefsDialog::setupLdapPage()
{
    ////////////////////////////////////////
    // Views
    QFrame *page = addPage( i18n("Views"), i18n("Views"),
                            KGlobal::iconLoader()->loadIcon( "viewmag",
                                                             KIcon::Desktop ));
    QVBoxLayout *topLayout = new QVBoxLayout( page );
    topLayout->setSpacing(spacingHint());
    topLayout->setMargin(marginHint());
    topLayout->setAutoAdd(true);

    mViewsSingleClickBox = new QCheckBox(i18n("Honor KDE single click"),
                                         page, "mViewsSingleClickBox");
    (void) new QWidget(page);  // spacer

    //////////////////////////////////
    // LDAP
    page = addPage( i18n("LDAP"), i18n("LDAP"),
                    KGlobal::iconLoader()->loadIcon( "find", KIcon::Desktop ));
    topLayout = new QVBoxLayout( page );
    topLayout->setSpacing(0);
    topLayout->setMargin(0);

    mLdapWidget = new LDAPOptionsWidgetImpl( page );
    topLayout->addWidget( mLdapWidget );
}

void PrefsDialog::readConfig()
{
    readLdapConfig();

    mViewsSingleClickBox->setChecked(KABPrefs::instance()->mHonorSingleClick);
}

void PrefsDialog::writeConfig()
{
    writeLdapConfig();

    KABPrefs::instance()->mHonorSingleClick = mViewsSingleClickBox->isChecked();
    KABPrefs::instance()->writeConfig();
}

void PrefsDialog::readLdapConfig()
{
    // Read LDAP information from KConfig object and write to config dialog.
    KConfig *config = kapp->config();
    config->setGroup("LDAP");

    // Michel: Populate the rightListBox and the ServersMap
    uint numSelectedHosts =  config->readUnsignedNumEntry( "NumSelectedHosts");
    if (numSelectedHosts > 0 ) {
        QString selectedHost;
        for ( uint cnt = 0; cnt <  numSelectedHosts; cnt++ ){
            //Set the key for search in the ServersMap
            selectedHost = config->readEntry( QString( "SelectedHost%1").arg(cnt),QString::null).stripWhiteSpace();
            //Populate the rightListBox
            QCheckListItem *item = new QCheckListItem(mLdapWidget->ldapServersLV, selectedHost, QCheckListItem::CheckBox);
            item->setOn(true);

            //Create a host account ( in the ServersMap )
            mLdapWidget->server.setbase( config->readEntry( QString( "SelectedBase%1" ).arg( cnt ), QString::null ).stripWhiteSpace());
            mLdapWidget->server.setport( config->readUnsignedNumEntry( QString( "SelectedPort%1" ).arg( cnt )));
            //Register the host and its data in the map
            mLdapWidget->_ldapservers[selectedHost] = mLdapWidget->server;
        }
    }

    // Populate the leftListBox and the ServersMap
    uint numHosts = config->readUnsignedNumEntry("NumHosts");
    if ( numHosts > 0 ) {
        QString host;
        for ( uint count = 0; count < numHosts; count++ ){
            // Set the key for search in the ServersMap
            host = config->readEntry( QString( "Host%1" ).arg( count ), QString::null).stripWhiteSpace();

            (void) new QCheckListItem( mLdapWidget->ldapServersLV, host, QCheckListItem::CheckBox );

            // Create a host account ( in the ServersMap )
            mLdapWidget->server.setbase( config->readEntry( QString( "Base%1" ).arg( count ), QString::null ).stripWhiteSpace());
            mLdapWidget->server.setport( config->readUnsignedNumEntry( QString( "Port%1" ).arg( count )));
            // Register the host and its data in the map
            mLdapWidget->_ldapservers[host] = mLdapWidget->server;
        }
    }

}

void PrefsDialog::writeLdapConfig()
{
    KConfig *config = kapp->config();

    //
    // Clean config. All the data registered is
    // deleted.
    //
    config->setGroup("LDAP");
    config->deleteGroup("LDAP", true, false );

    //
    // Save the hosts listed in the leftListBox
    // and its associated data registered in the ServersMap
    // to config. This data will then be used to populate
    // the leftListBox and the ServersMap
    //

    config->setGroup("LDAP");

    QString availableHosts;
    int numAvailHosts = 0;
    if ( mLdapWidget->ldapServersLV->childCount() > 0 ) {
        ServersMap::Iterator it;
        QListViewItemIterator itemIt( mLdapWidget->ldapServersLV );
        for ( ; itemIt.current(); ++itemIt ) {
            if (!(static_cast<QCheckListItem *>(itemIt.current()))->isOn()) {
                availableHosts = itemIt.current()->text(0).stripWhiteSpace();
                for ( it = mLdapWidget->_ldapservers.begin(); it != mLdapWidget->_ldapservers.end(); it++ ) {
                    if ( availableHosts == it.key() ) {
                        config->writeEntry( QString( "Host%1" ).arg( numAvailHosts ), it.key());
                        if( it.data().port()) {
                            config->writeEntry( QString( "Port%1" ).arg( numAvailHosts ), it.data().port());
                        } else {
                            config->deleteEntry( QString( "Port%1" ).arg( numAvailHosts ), false, true );
                        }
                        config->writeEntry( QString( "Base%1" ).arg( numAvailHosts ), it.data().baseDN());
                    }
                }
                numAvailHosts++;
            }
        }
    }
    config->writeEntry("NumHosts", numAvailHosts );

    //
    // Save and register the hosts selected (rightListBox)
    // and their associated  data to config. The hosts will be then
    // used to populate the rightListBox.
    //
    QString selectedHosts;
    int numSelHosts = 0;
    if ( mLdapWidget->ldapServersLV->childCount() > 0 ) {
        ServersMap::Iterator it;
        QListViewItemIterator itemIt( mLdapWidget->ldapServersLV );
        for ( ; itemIt.current(); ++itemIt ) {
            if ((static_cast<QCheckListItem *>(itemIt.current()))->isOn()) {
                selectedHosts = itemIt.current()->text(0).stripWhiteSpace();
                for ( it = mLdapWidget->_ldapservers.begin(); it != mLdapWidget->_ldapservers.end(); it++ ) {
                    if ( selectedHosts == it.key() ) {
                        config->writeEntry( QString( "SelectedHost%1" ).arg( numSelHosts ), it.key());
                        if( it.data().port()) {
                            config->writeEntry( QString( "SelectedPort%1" ).arg( numSelHosts ), it.data().port());
                        } else {
                            config->deleteEntry( QString( "SelectedPort%1" ).arg( numSelHosts ), false, true );
                        }
                        config->writeEntry( QString( "SelectedBase%1" ).arg( numSelHosts ), it.data().baseDN());
                    }
                }
                numSelHosts++;
            }
        }
    }
    config->writeEntry("NumSelectedHosts", numSelHosts );
    config->sync();
}

void PrefsDialog::slotApply()
{
    writeConfig();

    emit configChanged();
}

void PrefsDialog::slotOk()
{
    slotApply();
    accept();
}

#include "prefsdialog.moc"
