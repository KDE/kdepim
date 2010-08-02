/*
    This file is part of KAddressBook.
    Copyright (c) 2007 Klaralvdalens Datakonsult AB <frank@kdab.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

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

#include <tqlabel.h>
#include <tqlayout.h>
#include <tqsignalmapper.h>
#include <tqtoolbutton.h>

class KPIM::DistributionListEditor::EditorWidgetPrivate
{
public:
    TQScrollView* scrollView;
    TQSignalMapper* mapper;
    KABC::AddressBook* addressBook;
    TQString distListUid;
    TQLabel* nameLabel;
    TQLabel* memberListLabel;
    KLineEdit* nameLineEdit;
    TQWidget* memberListWidget;
    TQVBoxLayout* addresseeLayout;
    TQValueList<KPIM::DistributionListEditor::Line*> addressees;
    KPIM::DistributionList distributionList;
    KPIM::DistributionListEditor::Line* addLineForEntry( const KPIM::DistributionList::Entry& entry );
    int lastLineId;
};


KPIM::DistributionListEditor::Line::Line( KABC::AddressBook* book, TQWidget* parent ) : TQWidget( parent ), m_addressBook( book )
{
    Q_ASSERT( m_addressBook );
    TQBoxLayout* layout = new TQHBoxLayout( this );
    layout->setSpacing( KDialog::spacingHint() );
    m_lineEdit = new KPIM::DistributionListEditor::LineEdit( this );
    connect( m_lineEdit, TQT_SIGNAL( textChanged( const TQString& ) ),
             this, TQT_SLOT( textChanged( const TQString& ) ) );
    layout->addWidget( m_lineEdit );
    m_clearButton = new TQToolButton( this );
    m_clearButton->setIconSet( KApplication::reverseLayout() ? SmallIconSet("locationbar_erase") : SmallIconSet( "clear_left" ) );
    m_clearButton->setEnabled( false );
    layout->addWidget( m_clearButton );
    connect( m_clearButton, TQT_SIGNAL( clicked() ), m_lineEdit, TQT_SLOT( clear() ) );
}

void KPIM::DistributionListEditor::Line::textChanged( const TQString& text )
{
    m_clearButton->setEnabled( !text.isEmpty() );
    if ( text.isEmpty() )
        emit cleared();
    emit textChanged();
}

void KPIM::DistributionListEditor::Line::setFocusToLineEdit()
{
    m_lineEdit->setFocus();
}

void KPIM::DistributionListEditor::Line::setEntry( const KPIM::DistributionList::Entry& entry )
{
    m_uid = entry.addressee.uid();
    m_initialText = entry.addressee.fullEmail( entry.email );
    m_lineEdit->setText( m_initialText ); 
}

KABC::Addressee KPIM::DistributionListEditor::Line::findAddressee( const TQString& name, const TQString& email ) const
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
    const TQString text = m_lineEdit->text();
    TQString name;
    TQString email;
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
    res.email = res.addressee.preferredEmail() != email ? email : TQString();
    return res;
}


KPIM::DistributionListEditor::LineEdit::LineEdit( TQWidget* parent ) : KPIM::AddresseeLineEdit( parent )
{
}


KPIM::DistributionListEditor::EditorWidget::EditorWidget( KABC::AddressBook* book,  TQWidget* parent ) 
    : KDialogBase( parent, /*name=*/0, /*modal=*/ true, /*caption=*/TQString(), KDialogBase::Ok|KDialogBase::Cancel ), d( new DistributionListEditor::EditorWidgetPrivate )
{
    d->addressBook = book;
    Q_ASSERT( d->addressBook );
    d->lastLineId = 0;
    d->mapper = new TQSignalMapper( this );
    connect( d->mapper, TQT_SIGNAL( mapped( int ) ), 
             this, TQT_SLOT( lineTextChanged( int ) ) ); 
    setCaption( i18n( "Edit Distribution List" ) );
    TQWidget* main = new TQWidget( this );
    TQVBoxLayout* mainLayout = new TQVBoxLayout( main );
    mainLayout->setMargin( KDialog::marginHint() );
    mainLayout->setSpacing( KDialog::spacingHint() );

    TQHBoxLayout* nameLayout = new TQHBoxLayout;
    nameLayout->setSpacing( KDialog::spacingHint() );
    d->nameLabel = new TQLabel( main );
    d->nameLabel->setText( i18n( "Name:" ) );
    nameLayout->addWidget( d->nameLabel );

    d->nameLineEdit = new KLineEdit( main );
    nameLayout->addWidget( d->nameLineEdit );

    mainLayout->addLayout( nameLayout );
    mainLayout->addSpacing( 30 );

    d->memberListLabel = new TQLabel( main );
    d->memberListLabel->setText( i18n( "Distribution list members:" ) );
    mainLayout->addWidget( d->memberListLabel );

    d->scrollView = new TQScrollView( main );
    d->scrollView->setFrameShape( TQFrame::NoFrame );
    mainLayout->addWidget( d->scrollView );
    d->memberListWidget = new TQWidget( d->scrollView->viewport() );
    d->memberListWidget->setSizePolicy( TQSizePolicy::MinimumExpanding, TQSizePolicy::MinimumExpanding );
    TQVBoxLayout* memberLayout = new TQVBoxLayout( d->memberListWidget );
    d->addresseeLayout = new TQVBoxLayout;
    d->addresseeLayout->setSpacing( KDialog::spacingHint() );
    memberLayout->addItem( d->addresseeLayout );
    memberLayout->addStretch();
    d->scrollView->addChild( d->memberListWidget );
    d->scrollView->setResizePolicy( TQScrollView::AutoOneFit );
    
    setMainWidget( main );

    KPIM::DistributionListEditor::Line* const last = d->addLineForEntry( KPIM::DistributionList::Entry() );
    const TQSize hint = sizeHint();
    resize( hint.width() * 1.5, hint.height() );
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
    typedef TQValueList<Line*>::ConstIterator ListIterator;
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
    KPIM::DistributionListEditor::Line* const last = d->addLineForEntry( Entry() );
    last->setFocusToLineEdit();
}

KPIM::DistributionListEditor::Line* KPIM::DistributionListEditor::EditorWidgetPrivate::addLineForEntry( const KPIM::DistributionList::Entry& entry )
{  
    KPIM::DistributionListEditor::Line* line = new KPIM::DistributionListEditor::Line( addressBook, memberListWidget );
    line->setEntry( entry );
    addresseeLayout->addWidget( line );
    addressees.append( line );
    TQObject::connect( line, TQT_SIGNAL( textChanged() ), 
                      mapper, TQT_SLOT( map() ) );
    mapper->setMapping( line, ++lastLineId );
    line->setShown( true );
    return line;
}

void KPIM::DistributionListEditor::EditorWidget::slotOk()
{
    const TQString name = d->nameLineEdit->text();
    const KPIM::DistributionList existing = KPIM::DistributionList::findByName( d->addressBook, name );
    if ( !existing.isEmpty() && existing.uid() != d->distListUid )
    {
        KMessageBox::error( this, i18n( "A distribution list with the name %1 already exists. Please choose another name." ).arg( name ), i18n( "Name in Use" ) ); 
        return;
    }

    KPIM::DistributionList list;
    list.setUid( d->distListUid.isNull() ? KApplication::randomString( 10 ) :d->distListUid );
    list.setName( name );
    typedef TQValueList<KPIM::DistributionListEditor::Line*>::ConstIterator ListIterator;
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
