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

#include <qlabel.h>
#include <qlayout.h>
#include <qsignalmapper.h>
#include <qtoolbutton.h>
#include <qguardedptr.h>

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
    QGuardedPtr<KABC::Resource> resource;
    KPIM::DistributionList distributionList;
    KPIM::DistributionListEditor::Line* addLineForEntry( const KPIM::DistributionList::Entry& entry );
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
    m_clearButton = new QToolButton( this );
    m_clearButton->setIconSet( KApplication::reverseLayout() ? SmallIconSet("locationbar_erase") : SmallIconSet( "clear_left" ) );
    m_clearButton->setEnabled( false );
    layout->addWidget( m_clearButton );
    connect( m_clearButton, SIGNAL( clicked() ), m_lineEdit, SLOT( clear() ) );
}

void KPIM::DistributionListEditor::Line::textChanged( const QString& text )
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
    QVBoxLayout* mainLayout = new QVBoxLayout( main );
    mainLayout->setMargin( KDialog::marginHint() );
    mainLayout->setSpacing( KDialog::spacingHint() );

    QHBoxLayout* nameLayout = new QHBoxLayout;
    nameLayout->setSpacing( KDialog::spacingHint() );
    d->nameLabel = new QLabel( main );
    d->nameLabel->setText( i18n( "Name:" ) );
    nameLayout->addWidget( d->nameLabel );

    d->nameLineEdit = new KLineEdit( main );
    nameLayout->addWidget( d->nameLineEdit );

    mainLayout->addLayout( nameLayout );
    mainLayout->addSpacing( 30 );

    d->memberListLabel = new QLabel( main );
    d->memberListLabel->setText( i18n( "Distribution list members:" ) );
    mainLayout->addWidget( d->memberListLabel );

    d->scrollView = new QScrollView( main );
    d->scrollView->setFrameShape( QFrame::NoFrame );
    mainLayout->addWidget( d->scrollView );
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

    KPIM::DistributionListEditor::Line* const last = d->addLineForEntry( KPIM::DistributionList::Entry() );
    const QSize hint = sizeHint();
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
    d->resource = list.resource();

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
    KPIM::DistributionListEditor::Line* const last = d->addLineForEntry( Entry() );
    last->setFocusToLineEdit();
}

KPIM::DistributionListEditor::Line* KPIM::DistributionListEditor::EditorWidgetPrivate::addLineForEntry( const KPIM::DistributionList::Entry& entry )
{
    KPIM::DistributionListEditor::Line* line = new KPIM::DistributionListEditor::Line( addressBook, memberListWidget );
    line->setEntry( entry );
    addresseeLayout->addWidget( line );
    addressees.append( line );
    QObject::connect( line, SIGNAL( textChanged() ),
                      mapper, SLOT( map() ) );
    mapper->setMapping( line, ++lastLineId );
    line->setShown( true );
    return line;
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
    list.setResource( d->resource );
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
