#ifndef KSYNC_DUMMYKONNECTOR_H
#define KSYNC_DUMMYKONNECTOR_H

#include <qiconset.h>
#include <qptrlist.h>

#include <konnectorplugin.h>


namespace KSync {

/**
 * This plugin gets loaded by the KonnectorManager
 * this is the key to the KonnectorWorld
 * we need to implement the interface to fully support it...
 */
class DummyKonnector : public KSync::Konnector
{ 
    Q_OBJECT
  public:
    /**
     * @param parent the Parent Object
     * @param name the name
     * @param strlist a QStringList which is not used but neccessary for KGenericFactory
     */
    DummyKonnector( QObject*, const char*, const QStringList = QStringList() );
    ~DummyKonnector();

    /** return our capabilities() */
    KSync::Kapabilities capabilities();

    /**
     * the user configured this konnector
     * apply his preferecnes
     */
    void setCapabilities( const KSync::Kapabilities& );

    bool startSync();
    bool startBackup(const QString& path );
    bool startRestore( const QString& path );

    bool connectDevice();
    bool disconnectDevice();

    /** the state and some informations */
    KSync::KonnectorInfo info()const;

    /** download a resource/url/foobar */
    void download( const QString& );

    /** configuration widgets */
    KSync::ConfigWidget* configWidget( const KSync::Kapabilities&, QWidget* parent, const char* name );
    KSync::ConfigWidget* configWidget( QWidget* parent, const char* name );

  protected:
    /** write the Syncee back to the device */
    void write( Syncee::PtrList );

  private:
};

}


#endif
