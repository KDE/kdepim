#include "distributionlistentryview.h"
#include "imagewidget.h"
#include <interfaces/core.h>

#include <kabc/resourceabc.h>

#include <kabc/addressbook.h>
#include <kabc/resource.h>

#include <kdialog.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kurllabel.h>

#include <qcombobox.h>
#include <qlabel.h>
#include <qradiobutton.h>
#include <qstringlist.h>
//Added by qt3to4:
#include <QButtonGroup>
#include <QPixmap>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>


KAB::DistributionListEntryView::DistributionListEntryView( KAB::Core* core, QWidget* parent ) : QWidget( parent ), m_core( core ), m_emailGroup( 0 )
{
    m_mainLayout = new QVBoxLayout( this );
    m_mainLayout->setSpacing( KDialog::spacingHint() );
    m_mainLayout->setMargin( KDialog::marginHint() );

    QWidget* headerWidget = new QWidget;
    QBoxLayout* headerLayout = new QHBoxLayout( headerWidget );
    headerLayout->setSpacing( KDialog::spacingHint() * 3 );

    m_imageLabel = new QLabel( this );
    headerLayout->addWidget( m_imageLabel, 0, Qt::AlignTop );

    m_addresseeLabel = new QLabel( this );
    headerLayout->addWidget( m_addresseeLabel, 0, Qt::AlignTop );
    headerLayout->addStretch();

    m_mainLayout->addWidget( headerWidget );

    QWidget* distWidget = new QWidget;
    QBoxLayout* distLayout = new QHBoxLayout( distWidget );
    distLayout->setSpacing( KDialog::spacingHint() );

    QLabel* distLabel = new QLabel( this );
    distLabel->setText( i18n( "<b>Distribution list:</b>" ) );
    distLabel->setWordWrap( false );
    distLayout->addWidget( distLabel );

    m_distListLabel = new KUrlLabel( this );
    distLabel->setBuddy( m_distListLabel );
    connect( m_distListLabel, SIGNAL( leftClickedUrl( const QString& ) ),
             this, SIGNAL( distributionListClicked( const QString& ) ) );
    distLayout->addWidget( m_distListLabel );
    distLayout->addStretch();
    m_mainLayout->addWidget( distWidget );

    QLabel* emailLabel = new QLabel( this );
    emailLabel->setText( i18n( "<b>Email address to use in this list:</b>" ) );
    emailLabel->setWordWrap( false );
    m_mainLayout->addWidget( emailLabel );

    QWidget* emailWidget = new QWidget;
    QBoxLayout* emailLayout = new QHBoxLayout( emailWidget );
    emailLayout->setSpacing( KDialog::spacingHint() );
    emailLayout->addSpacing( 30 );

    QWidget* radioWidget = new QWidget;
    m_radioLayout = new QGridLayout( radioWidget );
    emailLayout->addWidget( radioWidget );
    emailLayout->addStretch();
    m_mainLayout->addWidget( emailWidget );

    QWidget* resourceWidget = new QWidget;
    QBoxLayout* resourceLayout = new QHBoxLayout( resourceWidget );
    resourceLayout->setSpacing( KDialog::spacingHint() );
    m_resourceLabel = new QLabel( this );
    resourceLayout->addWidget( m_resourceLabel );
    resourceLayout->addStretch();

    m_mainLayout->addWidget( resourceWidget );
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

    delete m_emailGroup;
    m_emailGroup = 0;

    QPixmap pixmap;
    if ( m_entry.addressee.photo().data().isNull() )
      pixmap = KIcon( "personal" ).pixmap( 100, 140 );
    else
      pixmap = QPixmap::fromImage( m_entry.addressee.photo().data() );
    m_imageLabel->setPixmap( pixmap );
    m_addresseeLabel->setText( i18nc( "Formatted name, role, organization", "<qt><h2>%1</h2><p>%2<br/>%3</p></qt>", m_entry.addressee.formattedName(), m_entry.addressee.role(), m_entry.addressee.organization() ) );
    m_distListLabel->setUrl( m_list.name() );
    m_distListLabel->setText( m_list.name() );
    m_resourceLabel->setText( i18n( "<b>Address book:</b> %1", (m_entry.addressee.resource() ? m_entry.addressee.resource()->resourceName() : QString()) ) );
    m_resourceLabel->setWordWrap( false );
    m_emailGroup = new QWidget( this );
    QBoxLayout *emailGroupLayout = new QVBoxLayout( m_emailGroup );
    QButtonGroup* buttonGroup = new QButtonGroup( m_emailGroup );
    buttonGroup->setExclusive( true );

    const QString preferred = m_entry.email.isNull() ? m_entry.addressee.preferredEmail() : m_entry.email;
    const QStringList mails = m_entry.addressee.emails();
    m_idToEmail.clear();
    int nextId = 0;
    foreach ( const QString it, mails )
    {
        QRadioButton* button = new QRadioButton( m_emailGroup );
        button->setText( it );
        buttonGroup->addButton( button, ++nextId );
        m_idToEmail.insert( nextId, it );
        if ( it == preferred )
            button->setChecked( true );
        button->setVisible( true );
        emailGroupLayout->addWidget( button );
    }
    connect( buttonGroup, SIGNAL( buttonClicked( int ) ),
             this, SLOT( emailButtonClicked( int ) ) );
    m_radioLayout->addWidget( m_emailGroup, 0, 0 );
    m_emailGroup->setVisible( true );
    m_mainLayout->invalidate();
}


#include "distributionlistentryview.moc"
