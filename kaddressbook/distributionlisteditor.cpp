#include "distributionlisteditor.h"
#include "distributionlisteditor_p.h"

#include <libkdepim/addresseelineedit.h>
#include <libkdepim/distributionlist.h>
#include <libemailfunctions/email.h>

#include <kabc/addressbook.h>

#include <kapplication.h>
#include <kdialogbase.h>
#include <kglobal.h> 
#include <kiconloader.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <qlabel.h>
#include <qlayout.h>
#include <qsignalmapper.h>
#include <qtoolbutton.h>

class KPIM::DistributionListEditor::EditorWidgetPrivate
{
public:
    QScrollView* scrollView;
    QSignalMapper* mapper;
    KABC::AddressBook* addressBook;
    QString distListUid;
    QLabel* nameLabel;
    QLabel* memberListLabel;
    KLineEdit* nameLineEdit;
    QWidget* memberListWidget;
    QVBoxLayout* addresseeLayout;
    QValueList<KPIM::DistributionListEditor::Line*> addressees;
    KPIM::DistributionList distributionList;
    void addLineForEntry( const KPIM::DistributionList::Entry& entry );
    int lastLineId;
};


KPIM::DistributionListEditor::Line::Line( KABC::AddressBook* book, QWidget* parent ) : QWidget( parent ), m_addressBook( book )
{
    Q_ASSERT( m_addressBook );
    QBoxLayout* layout = new QHBoxLayout( this );
    layout->setSpacing( KDialog::spacingHint() );
    m_lineEdit = new KPIM::DistributionListEditor::LineEdit( this );
    connect( m_lineEdit, SIGNAL( textChanged( const QString& ) ),
             this, SLOT( textChanged( const QString& ) ) );
    layout->addWidget( m_lineEdit );
    QToolButton *button = new QToolButton( this );
    button->setIconSet( KApplication::reverseLayout() ? SmallIconSet("locationbar_erase") : SmallIconSet( "clear_left" ) );
 

    layout->addWidget( button );
    connect( button, SIGNAL( clicked() ), m_lineEdit, SLOT( clear() ) );
}

void KPIM::DistributionListEditor::Line::textChanged( const QString& text )
{
    if ( text.isEmpty() )
        emit cleared();
    emit textChanged();
}

void KPIM::DistributionListEditor::Line::setEntry( const KPIM::DistributionList::Entry& entry )
{
    m_uid = entry.addressee.uid();
    m_initialText = entry.addressee.fullEmail( entry.email );
    m_lineEdit->setText( m_initialText ); 
}

KABC::Addressee KPIM::DistributionListEditor::Line::findAddressee( const QString& name, const QString& email ) const
{
    if ( name.isEmpty() && email.isEmpty() )
        return KABC::Addressee();

    typedef KABC::Addressee::List List;
    const List byEmail = m_addressBook->findByEmail( email );
    if ( !byEmail.isEmpty() )
    {        
        const List::ConstIterator end = byEmail.end();
        for ( List::ConstIterator it = byEmail.begin(); it != end; ++it )
        {
            if ( (*it).formattedName() == name )
                return *it;
        }
        return byEmail.first();
    }
    // no entry found, create new addressee:
    KABC::Addressee addressee;
    addressee.setUid( KApplication::randomString( 10 ) );
    addressee.setFormattedName( name );
    addressee.setEmails( email );
    m_addressBook->insertAddressee( addressee );
    return addressee;
}

KPIM::DistributionList::Entry KPIM::DistributionListEditor::Line::entry() const
{
    const QString text = m_lineEdit->text();
    QString name;
    QString email;
    KPIM::getNameAndMail(m_lineEdit->text(), name, email );

    KPIM::DistributionList::Entry res;
    if ( !m_uid.isNull() )
    {
        const KABC::Addressee addr = m_addressBook->findByUid( m_uid );
        if ( m_initialText == text || addr.formattedName() == name )
            res.addressee = addr;
    }
    if ( res.addressee.isEmpty() )
        res.addressee = findAddressee( name, email ); 
    res.email = res.addressee.preferredEmail() != email ? email : QString();
    return res;
}


KPIM::DistributionListEditor::LineEdit::LineEdit( QWidget* parent ) : KPIM::AddresseeLineEdit( parent )
{
}


KPIM::DistributionListEditor::EditorWidget::EditorWidget( KABC::AddressBook* book,  QWidget* parent ) 
    : KDialogBase( parent, /*name=*/0, /*modal=*/ true, /*caption=*/QString(), KDialogBase::Ok|KDialogBase::Cancel ), d( new DistributionListEditor::EditorWidgetPrivate )
{
    d->addressBook = book;
    Q_ASSERT( d->addressBook );
    d->lastLineId = 0;
    d->mapper = new QSignalMapper( this );
    connect( d->mapper, SIGNAL( mapped( int ) ), 
             this, SLOT( lineTextChanged( int ) ) ); 
    setCaption( i18n( "Edit Distribution List" ) );
    QWidget* main = new QWidget( this );
    QGridLayout* mainLayout = new QGridLayout( main );
    mainLayout->setMargin( KDialog::marginHint() );
    mainLayout->setSpacing( KDialog::spacingHint() );

    QHBoxLayout* nameLayout = new QHBoxLayout;
    nameLayout->setSpacing( KDialog::spacingHint() );

    d->nameLabel = new QLabel( main );
    d->nameLabel->setText( i18n( "Name:" ) );
    nameLayout->addWidget( d->nameLabel );

    d->nameLineEdit = new KLineEdit( main );
    nameLayout->addWidget( d->nameLineEdit );

    mainLayout->addLayout( nameLayout, 0, 0 );

    d->memberListLabel = new QLabel( main );
    d->memberListLabel->setText( i18n( "Distribution list members:" ) );
    mainLayout->addWidget( d->memberListLabel, 1, 0 );

    d->scrollView = new QScrollView( main );
    d->scrollView->setFrameShape( QFrame::NoFrame );
    mainLayout->addWidget( d->scrollView, 2, 0 );
    d->memberListWidget = new QWidget( d->scrollView->viewport() );
    d->memberListWidget->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding );
    QVBoxLayout* memberLayout = new QVBoxLayout( d->memberListWidget );
    d->addresseeLayout = new QVBoxLayout;
    d->addresseeLayout->setSpacing( KDialog::spacingHint() );
    memberLayout->addItem( d->addresseeLayout );
    memberLayout->addStretch();
    d->scrollView->addChild( d->memberListWidget );
    d->scrollView->setResizePolicy( QScrollView::AutoOneFit );
    
    setMainWidget( main );

    d->addLineForEntry( KPIM::DistributionList::Entry() );
}

KPIM::DistributionListEditor::EditorWidget::~EditorWidget()
{
    delete d;
}

void KPIM::DistributionListEditor::EditorWidget::lineTextChanged( int id )
{
    if ( id != d->lastLineId )
        return;
    d->addLineForEntry( KPIM::DistributionList::Entry() );
    d->scrollView->updateContents();
}

void KPIM::DistributionListEditor::EditorWidget::setDistributionList( const KPIM::DistributionList& list )
{
    d->distListUid = list.uid();
    d->nameLineEdit->setText( list.name() );

    using KPIM::DistributionListEditor::Line;
    typedef QValueList<Line*>::ConstIterator ListIterator;
    for ( ListIterator it = d->addressees.begin(), end = d->addressees.end(); it != end; ++it )
    {
        delete *it;
    }
    d->addressees.clear();

    typedef KPIM::DistributionList::Entry Entry;
    const Entry::List entries = list.entries( d->addressBook );

    for ( Entry::List::ConstIterator it = entries.begin(), end = entries.end(); it != end; ++it )
    {
        d->addLineForEntry( *it );
    }
    d->addLineForEntry( Entry() );
}

void KPIM::DistributionListEditor::EditorWidgetPrivate::addLineForEntry( const KPIM::DistributionList::Entry& entry )
{  
    KPIM::DistributionListEditor::Line* line = new KPIM::DistributionListEditor::Line( addressBook, memberListWidget );
    line->setEntry( entry );
    addresseeLayout->addWidget( line );
    addressees.append( line );
    QObject::connect( line, SIGNAL( textChanged() ), 
                      mapper, SLOT( map() ) );
    mapper->setMapping( line, ++lastLineId );
    line->setShown( true );
}

void KPIM::DistributionListEditor::EditorWidget::slotOk()
{
    const QString name = d->nameLineEdit->text();
    const KPIM::DistributionList existing = KPIM::DistributionList::findByName( d->addressBook, name );
    if ( !existing.isEmpty() && existing.uid() != d->distListUid )
    {
        KMessageBox::error( this, i18n( "A distribution list with the name %1 already exists. Please choose another name." ).arg( name ), i18n( "Name in Use" ) ); 
        return;
    }

    KPIM::DistributionList list;
    list.setUid( d->distListUid.isNull() ? KApplication::randomString( 10 ) :d->distListUid );
    list.setName( name );
    typedef QValueList<KPIM::DistributionListEditor::Line*>::ConstIterator ListIterator;
    for ( ListIterator it = d->addressees.begin(), end = d->addressees.end(); it != end; ++it )
    { 
        const KPIM::DistributionList::Entry entry = (*it)->entry();
        if ( entry.addressee.isEmpty() )
            continue;
        list.insertEntry( entry.addressee, entry.email );
    }
    d->distributionList = list;
    accept();
}

KPIM::DistributionList KPIM::DistributionListEditor::EditorWidget::distributionList() const
{
    return d->distributionList;
}

#include "distributionlisteditor.moc"
#include "distributionlisteditor_p.moc"
