#include "distributionlistentryview.h"
#include "imagewidget.h"
#include <interfaces/core.h>

#include <kabc/addressbook.h>

#include <kdialog.h>
#include <klocale.h>
#include <kurllabel.h>

#include <qbuttongroup.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qradiobutton.h>
#include <qstringlist.h>

KAB::DistributionListEntryView::DistributionListEntryView( KAB::Core* core, QWidget* parent ) : QWidget( parent ), m_core( core ), m_emailGroup( 0 )
{

    QBoxLayout* mainLayout = new QVBoxLayout( this );
    mainLayout->setSpacing( KDialog::spacingHint() );

    QBoxLayout* headerLayout = new QHBoxLayout;
    headerLayout->setSpacing( KDialog::spacingHint() );

    m_imageLabel = new QLabel( this );
    m_imageLabel->setAutoResize( true );
    headerLayout->addWidget( m_imageLabel );

    m_addresseeLabel = new QLabel( this );
    headerLayout->addWidget( m_addresseeLabel );

    headerLayout->addStretch();
    mainLayout->addItem( headerLayout );


    QBoxLayout* distLayout = new QHBoxLayout;
    distLayout->setSpacing( KDialog::spacingHint() );

    QLabel* distLabel = new QLabel( this );
    distLabel->setText( i18n( "Distribution list:" ) );
    distLayout->addWidget( distLabel );

    m_distListLabel = new KURLLabel( this );
    distLayout->addWidget( m_distListLabel );
    distLayout->addStretch();
    mainLayout->addItem( distLayout );

    QLabel* emailLabel = new QLabel( this );
    emailLabel->setText( i18n( "Email address to use in this list:" ) );

    mainLayout->addWidget( emailLabel );

    QBoxLayout* emailLayout = new QHBoxLayout;
    emailLayout->setSpacing( KDialog::spacingHint() );
    emailLayout->addSpacing( 30 );

    m_radioLayout = new QGridLayout;
    emailLayout->addItem( m_radioLayout );
    emailLayout->addStretch();
    mainLayout->addItem( emailLayout );

    mainLayout->addStretch();
}

void KAB::DistributionListEntryView::setEntry( const KPIM::DistributionList& list, const KPIM::DistributionList::Entry& entry )
{
    m_list = list;
    m_entry = entry;
    QPixmap pixmap;
    pixmap.convertFromImage( m_entry.addressee.photo().data() );
    m_imageLabel->setPixmap( pixmap );
    m_addresseeLabel->setText( i18n( "Formatted name, role, organization", "<qt><h1>%1</h1><p>%2<br/>%3</p></qt>" ).arg( m_entry.addressee.formattedName(), m_entry.addressee.role(), m_entry.addressee.organization() ) );
    m_distListLabel->setURL( m_list.uid() );
    m_distListLabel->setText( m_list.name() );

    if ( m_emailGroup )
        delete m_emailGroup;
    m_emailGroup = new QButtonGroup( this );
    m_emailGroup->setFlat( true );
    m_emailGroup->setExclusive( true );
    m_radioLayout->addWidget( m_emailGroup, 0, 0 );

    const QString preferred = m_entry.email.isNull() ? m_entry.addressee.preferredEmail() : m_entry.email;
    const QStringList mails = m_entry.addressee.emails();
    for ( QStringList::ConstIterator it = mails.begin(); it != mails.end(); ++it )
    {
        QRadioButton* button = new QRadioButton( m_emailGroup );
        button->setText( *it );
        if ( *it == preferred )
            button->setChecked( true );
    }
}

void KAB::DistributionListEntryView::writeBackChanges()
{
    m_list.removeEntry( m_entry.addressee, m_entry.email );
    //read changes
    m_list.insertEntry( m_entry.addressee, m_entry.email );
    m_core->addressBook()->insertAddressee( m_list ); 
}

#include "distributionlistentryview.moc"
