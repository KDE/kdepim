#include "detailsviewcontainer.h"
#include "detailsviewcontainer.moc"

#include <qcombobox.h>
#include <qframe.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qwidgetstack.h>

#include <kdebug.h>
#include <kapplication.h>

#include "look_basic.h"
#include "look_details.h"

ViewContainer::ViewContainer(QWidget *parent, const char* name )
    : QWidget(parent, name),
      m_look(0)
{
    QBoxLayout *topLayout = new QVBoxLayout( this );
    topLayout->setMargin( 3 );
    topLayout->setSpacing( 3 );

    QBoxLayout *styleLayout = new QHBoxLayout( topLayout );    

    QLabel *tlStyle = new QLabel( i18n("Style:"), this );
    styleLayout->addWidget( tlStyle );

    cbStyle = new QComboBox( this );
    styleLayout->addWidget( cbStyle );

    QFrame *frameRuler = new QFrame( this );
    frameRuler->setFrameShape( QFrame::HLine );
    frameRuler->setFrameShadow( QFrame::Sunken );
    topLayout->addWidget( frameRuler );

    frameDetails = new QWidgetStack( this );
    topLayout->addWidget( frameDetails, 1 );

    registerLooks();

#if 1
    // Hide detailed view selection combo box, because we currently have
    // only one. Reenable it when there are more detailed views.
    tlStyle->hide();
    cbStyle->hide();
    frameRuler->hide();
#endif
}


KABBasicLook *ViewContainer::look()
{
    return m_look;
}

void ViewContainer::registerLooks()
{
    m_lookFactories.append(new KABDetailedViewFactory(frameDetails));
    cbStyle->clear();
    for(unsigned int i=0; i<m_lookFactories.count(); ++i)
    {
        cbStyle->insertItem(m_lookFactories.at(i)->description());
    }
    // selected first look:
    if(!m_lookFactories.isEmpty())
    {
        slotStyleSelected(0);
    }
}

void ViewContainer::slotStyleSelected(int index)
{
    KConfig *config;
    if(index>=0 && index<cbStyle->count())
    {
        if(m_look!=0)
        {
            // WORK_TO_DO: save changes
            delete m_look;
            m_look=0;
        }
        KABLookFactory *factory=m_lookFactories.at(index);
        kdDebug(5720) << "ViewContainer::slotStyleSelected: "
                  << "creating look "
                  << factory->description() << endl;
        m_look=factory->create();
        frameDetails->raiseWidget( m_look );
        connect(m_look, SIGNAL(sendEmail(const QString&)), this,
                SIGNAL(sendEmail(const QString&)));
        connect(m_look, SIGNAL(browse(const QString&)), this,
                SIGNAL(browse(const QString&)));
    }
    // WORK_TO_DO: set current entry
    // ----- configure the style:
    config=kapp->config();
    m_look->configure(config);
}

void ViewContainer::setAddressee(const KABC::Addressee& addressee)
{
  if( m_look != 0 ) {
    m_look->setEntry(addressee);
  }
}

KABC::Addressee ViewContainer::addressee()
{
    static KABC::Addressee empty; // do not use!
    if(m_look==0)
    {
        return empty;
    } else {
        return m_look->entry();
    }
}

void ViewContainer::setReadonly(bool state)
{
    if(m_look!=0)
    {
        m_look->setReadonly(state);
    }
}

