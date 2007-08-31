#include "distributionlistentryview.h"
#include "imagewidget.h"
#include <interfaces/core.h>

#include <kabc/addressbook.h>

#include <kdialog.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kurllabel.h>

#include <qbuttongroup.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qradiobutton.h>
#include <qstringlist.h>
#include <qvbuttongroup.h>

KAB::DistributionListEntryView::DistributionListEntryView( KAB::Core* core, QWidget* parent ) : QWidget( parent ), m_core( core ), m_emailGroup( 0 )
{
    m_mainLayout = new QVBoxLayout( this );
    m_mainLayout->setSpacing( KDialog::spacingHint() );
    m_mainLayout->setMargin( KDialog::marginHint() );

    QBoxLayout* headerLayout = new QHBoxLayout;
    headerLayout->setSpacing( KDialog::spacingHint() * 3 );

    m_imageLabel = new QLabel( this );
    m_imageLabel->setAutoResize( true );
    headerLayout->addWidget( m_imageLabel, 0, Qt::AlignTop );

    m_addresseeLabel = new QLabel( this );
    headerLayout->addWidget( m_addresseeLabel, 0, Qt::AlignTop );
    headerLayout->addStretch();

    m_mainLayout->addItem( headerLayout );

    QBoxLayout* distLayout = new QHBoxLayout;
    distLayout->setSpacing( KDialog::spacingHint() );

    QLabel* distLabel = new QLabel( this );
    distLabel->setText( i18n( "<b>Distribution list:</b>" ) );
    distLabel->setAlignment( Qt::SingleLine );
    distLayout->addWidget( distLabel );

    m_distListLabel = new KURLLabel( this );
    connect( m_distListLabel, SIGNAL( leftClickedURL( const QString& ) ), 
             this, SIGNAL( distributionListClicked( const QString& ) ) );
    distLayout->addWidget( m_distListLabel );
    distLayout->addStretch();
    m_mainLayout->addItem( distLayout );

    QLabel* emailLabel = new QLabel( this );
    emailLabel->setText( i18n( "<b>Email address to use in this list:</b>" ) );
    emailLabel->setAlignment( Qt::SingleLine );
    m_mainLayout->addWidget( emailLabel );

    QBoxLayout* emailLayout = new QHBoxLayout;
    emailLayout->setSpacing( KDialog::spacingHint() );
    emailLayout->addSpacing( 30 );

    m_radioLayout = new QGridLayout;
    emailLayout->addItem( m_radioLayout );
    emailLayout->addStretch();
    m_mainLayout->addItem( emailLayout );

    m_mainLayout->addStretch();
}

void KAB::DistributionListEntryView::emailButtonClicked( int id )
{
    const QString email = m_idToEmail[ id ];
    if ( m_entry.email == email )
        return;
    m_list.removeEntry( m_entry.addressee, m_entry.email );
    m_entry.email = email;
    m_list.insertEntry( m_entry.addressee, m_entry.email );
    m_core->addressBook()->insertAddressee( m_list ); 
}

void KAB::DistributionListEntryView::clear()
{
    setEntry( KPIM::DistributionList(), KPIM::DistributionList::Entry() );
}

void KAB::DistributionListEntryView::setEntry( const KPIM::DistributionList& list, const KPIM::DistributionList::Entry& entry )
{    
    m_list = list;
    m_entry = entry;

    if ( m_emailGroup )
        delete m_emailGroup;
    QPixmap pixmap;
    pixmap.convertFromImage( m_entry.addressee.photo().data() );
    m_imageLabel->setPixmap( pixmap.isNull() ? KGlobal::iconLoader()->loadIcon( "personal", KIcon::Desktop ) : pixmap );
    m_addresseeLabel->setText( i18n( "Formatted name, role, organization", "<qt><h2>%1</h2><p>%2<br/>%3</p></qt>" ).arg( m_entry.addressee.formattedName(), m_entry.addressee.role(), m_entry.addressee.organization() ) );
    m_distListLabel->setURL( m_list.name() );
    m_distListLabel->setText( m_list.name() );

    m_emailGroup = new QVButtonGroup( this );
    m_emailGroup->setFlat( true );
    m_emailGroup->setExclusive( true );
    m_emailGroup->setFrameShape( QFrame::NoFrame );

    const QString preferred = m_entry.email.isNull() ? m_entry.addressee.preferredEmail() : m_entry.email;
    const QStringList mails = m_entry.addressee.emails();
    m_idToEmail.clear();
    for ( QStringList::ConstIterator it = mails.begin(); it != mails.end(); ++it )
    {
        QRadioButton* button = new QRadioButton( m_emailGroup );
        button->setText( *it );
        m_idToEmail.insert( m_emailGroup->insert( button ), *it );
        if ( *it == preferred )
            button->setChecked( true );
        button->setShown( true );
    }
    connect( m_emailGroup, SIGNAL( clicked( int ) ), 
             this, SLOT( emailButtonClicked( int ) ) ); 
    m_radioLayout->addWidget( m_emailGroup, 0, 0 );
    m_emailGroup->setShown( true );
    m_mainLayout->invalidate();
}


#include "distributionlistentryview.moc"
