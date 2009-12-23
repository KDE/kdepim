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

#include <kpimutils/email.h>

#include <kabc/addressbook.h>
#include <kabc/resource.h>
#include <kabc/distributionlist.h>

#include <kglobal.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <KRandom>

#include <QApplication>
#include <qlabel.h>
#include <qlayout.h>
#include <QScrollArea>
#include <qsignalmapper.h>
#include <QPointer>

class KPIM::DistributionListEditor::EditorWidgetPrivate
{
public:
    QScrollArea* scrollView;
    QSignalMapper* mapper;
    KABC::AddressBook* addressBook;
    QString distListUid;
    QLabel* nameLabel;
    QLabel* memberListLabel;
    KLineEdit* nameLineEdit;
    QWidget* memberListWidget;
    QVBoxLayout* addresseeLayout;
    QList<KPIM::DistributionListEditor::Line*> addressees;
    QPointer<KABC::Resource> resource;
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
    m_lineEdit->setClearButtonShown( true );
    connect( m_lineEdit, SIGNAL( textChanged( const QString& ) ),
             this, SLOT( textChanged( const QString& ) ) );
    layout->addWidget( m_lineEdit );
}

void KPIM::DistributionListEditor::Line::textChanged( const QString& text )
{
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
    addressee.setUid( KRandom::randomString( 10 ) );
    addressee.setFormattedName( name );
    addressee.setEmails( QStringList( email ) );
    m_addressBook->insertAddressee( addressee );
    return addressee;
}

KPIM::DistributionList::Entry KPIM::DistributionListEditor::Line::entry() const
{
    const QString text = m_lineEdit->text();
    QString name;
    QString email;
    KPIMUtils::extractEmailAddressAndName(m_lineEdit->text(), email, name );
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
  allowDistributionLists( false );
}

void KPIM::DistributionListEditor::LineEdit::addContact( const KABC::Addressee &addr, int weight, int source )
{
    if ( KPIM::DistributionList::isDistributionList( addr ) )
        return;
    KPIM::AddresseeLineEdit::addContact( addr, weight, source );
}

void KPIM::DistributionListEditor::EditorWidget::slotButtonClicked( int button )
{
    if ( button == Ok )
        saveList();

    KDialog::slotButtonClicked( button );
}

KPIM::DistributionListEditor::EditorWidget::EditorWidget( KABC::AddressBook* book,  QWidget* parent )
    : KDialog( parent ), d( new DistributionListEditor::EditorWidgetPrivate )
{
    d->addressBook = book;
    Q_ASSERT( d->addressBook );
    d->lastLineId = 0;
    d->mapper = new QSignalMapper( this );
    connect( d->mapper, SIGNAL( mapped( int ) ),
             this, SLOT( lineTextChanged( int ) ) );
    showButton( Ok, true );
    showButton( Cancel, true );
    setModal( true );
    setWindowTitle( i18n( "Edit Distribution List" ) );
    QWidget* main = new QWidget( this );
    QVBoxLayout* mainLayout = new QVBoxLayout( main );
    mainLayout->setMargin( KDialog::marginHint() );
    mainLayout->setSpacing( KDialog::spacingHint() );

    QWidget* nameWidget = new QWidget;
    QHBoxLayout* nameLayout = new QHBoxLayout( nameWidget );
    nameLayout->setSpacing( KDialog::spacingHint() );
    d->nameLabel = new QLabel;
    d->nameLabel->setText( i18n( "Name:" ) );
    nameLayout->addWidget( d->nameLabel );

    d->nameLineEdit = new KLineEdit;
    d->nameLabel->setBuddy( d->nameLineEdit );
    nameLayout->addWidget( d->nameLineEdit );

    mainLayout->addWidget( nameWidget );
    mainLayout->addSpacing( 30 );

    d->memberListLabel = new QLabel;
    d->memberListLabel->setText( i18n( "Distribution list members:" ) );
    mainLayout->addWidget( d->memberListLabel );

    d->scrollView = new QScrollArea;
    d->scrollView->setFrameShape( QFrame::NoFrame );
    d->scrollView->setWidgetResizable( true );
    mainLayout->addWidget( d->scrollView );

    d->memberListWidget = new QWidget( this );
    d->scrollView->setWidget( d->memberListWidget );
    d->memberListWidget->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding );
    QVBoxLayout* memberLayout = new QVBoxLayout( d->memberListWidget );
    QWidget* addresseeWidget = new QWidget;
    d->addresseeLayout = new QVBoxLayout( addresseeWidget );
    memberLayout->addWidget( addresseeWidget );
    memberLayout->addStretch();

    setMainWidget( main );

    const QSize hint = sizeHint();
    resize( hint.width() * 1.5, hint.height() * 1.5 );
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
}

void KPIM::DistributionListEditor::EditorWidget::setDistributionList( const KPIM::DistributionList& list )
{
    d->distListUid = list.uid();
    d->nameLineEdit->setText( list.name() );
    d->resource = list.resource();

    using KPIM::DistributionListEditor::Line;
    qDeleteAll( d->addressees );
    d->addressees.clear();

    typedef KPIM::DistributionList::Entry Entry;
    const Entry::List entries = list.entries( d->addressBook );

    Q_FOREACH( const KPIM::DistributionList::Entry i, entries )
      d->addLineForEntry( i );
    KPIM::DistributionListEditor::Line* const last = d->addLineForEntry( Entry() );
    last->setFocusToLineEdit();
}

KPIM::DistributionListEditor::Line* KPIM::DistributionListEditor::EditorWidgetPrivate::addLineForEntry( const KPIM::DistributionList::Entry& entry )
{
    KPIM::DistributionListEditor::Line* line = new KPIM::DistributionListEditor::Line( addressBook );
    line->setEntry( entry );
    addresseeLayout->addWidget( line );
    addressees.append( line );
    QObject::connect( line, SIGNAL( textChanged() ),
                      mapper, SLOT( map() ) );
    mapper->setMapping( line, ++lastLineId );
    line->setVisible( true );
    return line;
}

void KPIM::DistributionListEditor::EditorWidget::saveList()
{
    const QString name = d->nameLineEdit->text();
    const KABC::DistributionList *existing =
      d->addressBook->findDistributionListByName( name );
    if ( existing && existing->identifier() != d->distListUid )
    {
        KMessageBox::error( this, i18n( "A distribution list with the name %1 already exists. Please choose another name.", name ), i18n( "Name in Use" ) );
        return;
    }

    KPIM::DistributionList list;
    list.setUid( d->distListUid.isNull() ? KRandom::randomString( 10 ) :d->distListUid );
    list.setName( name );
    list.setResource( d->resource );
    foreach ( const KPIM::DistributionListEditor::Line* i, d->addressees )
    {
        const KPIM::DistributionList::Entry entry = i->entry();
        if ( entry.addressee.isEmpty() )
            continue;
        list.insertEntry( entry.addressee, entry.email );
    }
    d->distributionList = list;
}

KPIM::DistributionList KPIM::DistributionListEditor::EditorWidget::distributionList() const
{
    return d->distributionList;
}

#include "distributionlisteditor.moc"
#include "distributionlisteditor_p.moc"
