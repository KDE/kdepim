#include "distributionlisteditor.h"
#include "distributionlisteditor_p.h"

#include <libkdepim/addresseelineedit.h>
#include <libkdepim/distributionlist.h>

#include <kabc/addressbook.h>

#include <kdialogbase.h> 
#include <klineedit.h>
#include <klocale.h>

#include <qlabel.h>
#include <qlayout.h>

class KPIM::DistributionListEditor::EditorWidgetPrivate
{
public:
    KABC::AddressBook* addressBook;
    QLabel* nameLabel;
    QLabel* memberListLabel;
    KLineEdit* nameLineEdit;
    QWidget* memberListWidget;
    QVBoxLayout* addresseeLayout;
    QValueList<KPIM::DistributionListEditor::Line*> addressees;

    KPIM::DistributionListEditor::Line* addLineForEntry( const KPIM::DistributionList::Entry& entry );
};


KPIM::DistributionListEditor::Line::Line( KABC::AddressBook* book, QWidget* parent ) : QWidget( parent ), m_addressBook( book )
{
    Q_ASSERT( m_addressBook );
    QVBoxLayout* layout = new QVBoxLayout( this );
    layout->setSpacing( KDialog::spacingHint() );
    m_lineEdit = new KPIM::DistributionListEditor::LineEdit( this );
    layout->addWidget( m_lineEdit );
}

void KPIM::DistributionListEditor::Line::setAddressee( const KABC::Addressee& addressee )
{
}

KABC::Addressee KPIM::DistributionListEditor::Line::addressee() const
{
    return KABC::Addressee();
}

KPIM::DistributionListEditor::LineEdit::LineEdit( QWidget* parent ) : KPIM::AddresseeLineEdit( parent )
{
}


KPIM::DistributionListEditor::EditorWidget::EditorWidget( KABC::AddressBook* book,  QWidget* parent ) 
    : KDialogBase( parent, /*name=*/0, /*modal=*/ true, /*caption=*/QString(), KDialogBase::Ok|KDialogBase::Cancel ), d( new DistributionListEditor::EditorWidgetPrivate )
{
    d->addressBook = book;
    Q_ASSERT( d->addressBook );
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

    QScrollView* scrollView = new QScrollView( main );
    scrollView->setFrameShape( QFrame::NoFrame );
    mainLayout->addWidget( scrollView, 2, 0 );

    d->addresseeLayout = new QVBoxLayout( scrollView->viewport() );
    d->memberListWidget = scrollView->viewport();
    d->addresseeLayout->setSpacing( KDialog::spacingHint() );
    
    setMainWidget( main );

    typedef KPIM::DistributionList::Entry Entry;
    d->addLineForEntry( Entry() );
    d->addLineForEntry( Entry() );
    d->addLineForEntry( Entry() );
}

KPIM::DistributionListEditor::EditorWidget::~EditorWidget()
{
    delete d;
}


void KPIM::DistributionListEditor::EditorWidget::setDistributionList( const KPIM::DistributionList& list )
{
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
    d->addLineForEntry( Entry() );
    d->addLineForEntry( Entry() );
}

KPIM::DistributionListEditor::Line* KPIM::DistributionListEditor::EditorWidgetPrivate::addLineForEntry( const KPIM::DistributionList::Entry& entry )
{  
   KPIM::DistributionListEditor::Line* line = new KPIM::DistributionListEditor::Line( addressBook, memberListWidget );
   addresseeLayout->addWidget( line );
   addressees.append( line );
   return line;
}

KPIM::DistributionList KPIM::DistributionListEditor::EditorWidget::distributionList() const
{
    KPIM::DistributionList list;
    list.setName( d->nameLineEdit->text() );

    typedef QValueList<KPIM::DistributionListEditor::Line*>::ConstIterator ListIterator;
    for ( ListIterator it = d->addressees.begin(), end = d->addressees.end(); it != end; ++it )
    {
        list.insertEntry( (*it)->addressee(), QString() );
    }
    return list;
}

#include "distributionlisteditor.moc"
#include "distributionlisteditor_p.moc"
