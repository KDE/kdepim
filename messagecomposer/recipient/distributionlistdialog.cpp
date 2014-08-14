/*
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>
    This file was part of KMail.
    Copyright (c) 2005 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/


#include "distributionlistdialog.h"

#include <AkonadiWidgets/collectiondialog.h>
#include <Akonadi/Contact/ContactGroupSearchJob>
#include <Akonadi/Contact/ContactSearchJob>
#include <AkonadiCore/itemcreatejob.h>
#include <KPIMUtils/kpimutils/email.h>

#include <KLocalizedString>
#include <QDebug>
#include <QLineEdit>
#include <KMessageBox>
#include <QInputDialog>

#include <QLabel>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <KSharedConfig>
#include <QDialogButtonBox>
#include <KConfigGroup>
#include <QPushButton>

using namespace MessageComposer;

namespace MessageComposer {
class DistributionListItem : public QTreeWidgetItem
{
public:
    DistributionListItem( QTreeWidget *tree )
        : QTreeWidgetItem( tree )
    {
        setFlags( flags() | Qt::ItemIsUserCheckable );
    }

    void setAddressee( const KABC::Addressee &a, const QString &email )
    {
        init( a, email );
    }

    void init( const KABC::Addressee &a, const QString &email )
    {
        mAddressee = a;
        mEmail = email;
        mId = -1;
        setText( 0, mAddressee.realName() );
        setText( 1, mEmail );
    }

    KABC::Addressee addressee() const
    {
        return mAddressee;
    }

    QString email() const
    {
        return mEmail;
    }

    bool isTransient() const
    {
        return mId == -1;
    }

    void setId( Akonadi::Entity::Id id )
    {
        mId = id;
    }

    Akonadi::Entity::Id id() const
    {
        return mId;
    }

private:
    KABC::Addressee mAddressee;
    QString mEmail;
    Akonadi::Entity::Id mId;
};
}


DistributionListDialog::DistributionListDialog( QWidget *parent )
    : QDialog( parent )
{
    QFrame *topFrame = new QFrame( this );
    setWindowTitle( i18nc("@title:window", "Save Distribution List") );
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(topFrame);
    mUser1Button = new QPushButton;
    buttonBox->addButton(mUser1Button, QDialogButtonBox::ActionRole);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    //PORTING SCRIPT: WARNING mainLayout->addWidget(buttonBox) must be last item in layout. Please move it.
    mainLayout->addWidget(buttonBox);
    mUser1Button->setDefault(true);
    setModal( false );
    mUser1Button->setText(i18nc("@action:button","Save List"));
    mUser1Button->setEnabled(false);

    QBoxLayout *topLayout = new QVBoxLayout( topFrame );
    //PORT QT5 topLayout->setSpacing( spacingHint() );

    QBoxLayout *titleLayout = new QHBoxLayout();
    //PORT QT5 titleLayout->setSpacing( spacingHint() );
    topLayout->addItem( titleLayout );

    QLabel *label = new QLabel(
                i18nc("@label:textbox Name of the distribution list.", "&Name:"), topFrame );
    titleLayout->addWidget( label );

    mTitleEdit = new QLineEdit( topFrame );
    titleLayout->addWidget( mTitleEdit );
    mTitleEdit->setFocus();
    mTitleEdit->setClearButtonEnabled( true );
    label->setBuddy( mTitleEdit );

    mRecipientsList = new QTreeWidget( topFrame );
    mRecipientsList->setHeaderLabels(
                QStringList() << i18nc( "@title:column Name of the recipient","Name" )
                << i18nc( "@title:column Email of the recipient", "Email" )
                );
    mRecipientsList->setRootIsDecorated( false );
    mRecipientsList->header()->setMovable(false);
    topLayout->addWidget( mRecipientsList );
    connect(mUser1Button, SIGNAL(clicked()),
             this, SLOT(slotUser1()) );
    connect( mTitleEdit, SIGNAL(textChanged(QString)),
             this, SLOT(slotTitleChanged(QString)) );
    readConfig();
}

DistributionListDialog::~DistributionListDialog()
{
    writeConfig();
}

// This starts one ContactSearchJob for each of the specified recipients.
void DistributionListDialog::setRecipients( const Recipient::List &recipients )
{
    Recipient::List::ConstIterator end( recipients.constEnd() );
    for( Recipient::List::ConstIterator it = recipients.constBegin(); it != end; ++it ) {
        const QStringList emails = KPIMUtils::splitAddressList( (*it)->email() );
        QStringList::ConstIterator end2( emails.constEnd() );
        for( QStringList::ConstIterator it2 = emails.constBegin(); it2 != end2; ++it2 ) {
            QString name;
            QString email;
            KABC::Addressee::parseEmailAddress( *it2, name, email );
            if ( !email.isEmpty() ) {
                Akonadi::ContactSearchJob *job = new Akonadi::ContactSearchJob(this);
                job->setQuery( Akonadi::ContactSearchJob::Email, email.toLower(), Akonadi::ContactSearchJob::ExactMatch );
                job->setProperty( "name", name );
                job->setProperty( "email", email );
                connect( job, SIGNAL(result(KJob*)), SLOT(slotDelayedSetRecipients(KJob*)) );
            }
        }
    }
}

// This result slot will be called once for each of the original recipients.
// There could potentially be more than one Akonadi item returned per
// recipient, in the case where email addresses are duplicated between contacts.
void DistributionListDialog::slotDelayedSetRecipients( KJob *job )
{
    const Akonadi::ContactSearchJob *searchJob = qobject_cast<Akonadi::ContactSearchJob*>( job );
    const Akonadi::Item::List akItems = searchJob->items();

    const QString email = searchJob->property( "email" ).toString();
    QString name = searchJob->property( "name" ).toString();
    if ( name.isEmpty() ) {
        const int index = email.indexOf( QLatin1Char( '@' ) );
        if ( index != -1 ) {
            name = email.left( index );
        } else {
            name = email;
        }
    }

    if ( akItems.isEmpty() ) {
        KABC::Addressee contact;
        contact.setNameFromString( name );
        contact.insertEmail( email );

        DistributionListItem *item = new DistributionListItem( mRecipientsList );
        item->setAddressee( contact, email );
        item->setCheckState( 0, Qt::Checked );
    } else {
        bool isFirst = true;
        foreach ( const Akonadi::Item &akItem, akItems ) {
            if ( akItem.hasPayload<KABC::Addressee>() ) {
                const KABC::Addressee contact = akItem.payload<KABC::Addressee>();

                DistributionListItem *item = new DistributionListItem( mRecipientsList );
                item->setAddressee( contact, email );

                // Need to record the Akonadi ID of the contact, so that
                // it can be added as a reference later.  Setting an ID
                // makes the item non-transient.
                item->setId( akItem.id() );

                // If there were multiple contacts returned for an email address,
                // then check the first one and uncheck any subsequent ones.
                if ( isFirst ) {
                    item->setCheckState( 0, Qt::Checked );
                    isFirst = false;
                } else {
                    // Need this to create an unchecked item, as otherwise the
                    // item will have no checkbox at all.
                    item->setCheckState( 0, Qt::Unchecked );
                }
            }
        }
    }
}

void DistributionListDialog::slotUser1()
{
    bool isEmpty = true;
    const int numberOfTopLevel( mRecipientsList->topLevelItemCount() );
    for (int i = 0; i < numberOfTopLevel; ++i) {
        DistributionListItem *item = static_cast<DistributionListItem *>(
                    mRecipientsList->topLevelItem( i ));
        if ( item && item->checkState( 0 ) == Qt::Checked ) {
            isEmpty = false;
            break;
        }
    }

    if ( isEmpty ) {
        KMessageBox::information( this,
                                  i18nc("@info", "There are no recipients in your list. "
                                        "First select some recipients, "
                                        "then try again.") );
        return;
    }

    QString name = mTitleEdit->text();

    if ( name.isEmpty() ) {
        bool ok = false;
        name = QInputDialog::getText( this, i18nc("@title:window","New Distribution List"),
                                      i18nc("@label:textbox","Please enter name:"), QLineEdit::Normal, QString(), &ok );
        if ( !ok || name.isEmpty() )
            return;
    }

    Akonadi::ContactGroupSearchJob *job = new Akonadi::ContactGroupSearchJob();
    job->setQuery( Akonadi::ContactGroupSearchJob::Name, name );
    job->setProperty( "name", name );
    connect( job, SIGNAL(result(KJob*)), SLOT(slotDelayedUser1(KJob*)) );
}

void DistributionListDialog::slotDelayedUser1( KJob *job )
{
    const Akonadi::ContactGroupSearchJob *searchJob = qobject_cast<Akonadi::ContactGroupSearchJob*>( job );
    const QString name = searchJob->property( "name" ).toString();

    if ( !searchJob->contactGroups().isEmpty() ) {
        KMessageBox::information( this,
                                  xi18nc( "@info", "<para>Distribution list with the given name <resource>%1</resource> "
                                         "already exists. Please select a different name.</para>", name ) );
        return;
    }

    QPointer<Akonadi::CollectionDialog> dlg =
            new Akonadi::CollectionDialog( Akonadi::CollectionDialog::KeepTreeExpanded, 0, this );
    dlg->setMimeTypeFilter( QStringList() << KABC::Addressee::mimeType()
                            << KABC::ContactGroup::mimeType() );
    dlg->setAccessRightsFilter( Akonadi::Collection::CanCreateItem );
    dlg->setWindowTitle( i18nc( "@title:window", "Select Address Book" ) );
    dlg->setDescription( i18n( "Select the address book folder to store the contact group in:" ) );
    if ( dlg->exec() ) {
        const Akonadi::Collection targetCollection = dlg->selectedCollection();
        delete dlg;

        KABC::ContactGroup group( name );
        const int numberOfTopLevel( mRecipientsList->topLevelItemCount() );
        for ( int i = 0; i < numberOfTopLevel; ++i ) {
            DistributionListItem *item = static_cast<DistributionListItem *>( mRecipientsList->topLevelItem( i ) );
            if ( item && item->checkState( 0 ) == Qt::Checked ) {
                qDebug() << item->addressee().fullEmail() << item->addressee().uid();
                if ( item->isTransient() ) {
                    group.append( KABC::ContactGroup::Data( item->addressee().realName(), item->email() ) );
                } else {
                    KABC::ContactGroup::ContactReference reference( QString::number( item->id() ) );
                    if ( item->email() != item->addressee().preferredEmail() ) {
                        reference.setPreferredEmail( item->email() );
                    }
                    group.append( reference );
                }
            }
        }

        Akonadi::Item groupItem( KABC::ContactGroup::mimeType() );
        groupItem.setPayload<KABC::ContactGroup>( group );

        Akonadi::Job *createJob = new Akonadi::ItemCreateJob( groupItem, targetCollection );
        connect( createJob, SIGNAL(result(KJob*)), this, SLOT(slotContactGroupCreateJobResult(KJob*)) );
    }

    delete dlg;
}

void DistributionListDialog::slotContactGroupCreateJobResult( KJob *job )
{
    if ( job->error() ) {
        KMessageBox::information( this, i18n("Unable to create distribution list: %1", job->errorString() ));
        qWarning() << "Unable to create distribution list:" << job->errorText();
    } else {
        accept();
    }
}

void DistributionListDialog::slotTitleChanged( const QString& text )
{
    mUser1Button->setEnabled(!text.trimmed().isEmpty());
}

void DistributionListDialog::readConfig()
{
    KSharedConfig::Ptr cfg = KSharedConfig::openConfig();
    KConfigGroup group( cfg, "DistributionListDialog" );
    const QSize size = group.readEntry( "Size", QSize() );
    if ( !size.isEmpty() ) {
        resize( size );
    }
    mRecipientsList->header()->restoreState(group.readEntry("Header", QByteArray()));
}

void DistributionListDialog::writeConfig()
{
    KSharedConfig::Ptr cfg = KSharedConfig::openConfig();
    KConfigGroup group( cfg, "DistributionListDialog" );
    group.writeEntry( "Size", size() );
    group.writeEntry( "Header", mRecipientsList->header()->saveState() );
}


