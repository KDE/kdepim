#include <qlayout.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qlistview.h>
#include <qdragobject.h>

#include <kdialog.h>
#include <klocale.h>
#include <kabc/distributionlist.h>
#include <kabc/distributionlisteditor.h>
#include <klineeditdlg.h>
#include <kmessagebox.h>
#include <kdebug.h>

#include "addresseeutil.h"
#include "featuredistributionlist.h"
#include "featuredistributionlistview.h"

namespace KABC
{

// MOSTLY A COPY FROM kdelibs/kabc:
    class EntryItem : public QListViewItem
    {
    protected:
        FeatureDistributionList *list;
    public:
        EntryItem(FeatureDistributionList *l,
                  QListView *parent, const Addressee &addressee,
                  const QString &email=QString::null ) :
            QListViewItem( parent ),
            list(l),
            mAddressee( addressee ),
            mEmail( email )
            {
                setDropEnabled(true);
                setText( 0, addressee.realName() );
                if( email.isEmpty() ) {
                    setText( 1, addressee.preferredEmail() );
                    setText( 2, i18n("Yes") );
                } else {
                    setText( 1, email );
                    setText( 2, i18n("No") );
                }
            }

        Addressee addressee() const
            {
                return mAddressee;
            }

        QString email() const
            {
                return mEmail;
            }
    protected:
        bool acceptDrop ( const QMimeSource * /* mime */ )
            { // WORK_TO_DO: check data type
                return true;
            }
        void dropped(QDropEvent *e)
            {
                list->slotDropped(e);
            }
    private:
        Addressee mAddressee;
        QString mEmail;
    };
}

FeatureDistributionList::FeatureDistributionList(KABC::AddressBook *doc,
                                                 QWidget *parent, const char* name)
    : FeatureDistributionListBase(parent, name),
      mDoc(doc),
      mManager(new KABC::DistributionListManager(doc))
{
    QLayout *l=layout();
    if(l!=0) // should be...
    {
        l->setMargin(KDialog::marginHint());
        l->setSpacing(KDialog::spacingHint());
    }
    connect(mLvAddressees, SIGNAL(selectionChanged()),
            SLOT(slotAddresseeSelectionChanged()));
    connect(mLvAddressees, SIGNAL(dropped(QDropEvent*)),
            SLOT(slotDropped(QDropEvent*)));
    mLvAddressees->addColumn( i18n("Name") );
    mLvAddressees->addColumn( i18n("Email") );
    mLvAddressees->addColumn( i18n("Use Preferred") );

    mManager->load();
}

FeatureDistributionList::~FeatureDistributionList()
{
    delete mManager;
}

void FeatureDistributionList::update()
{
    int index=mCbListSelect->currentItem();
    mLvAddressees->clear();
    mCbListSelect->clear();
    mCbListSelect->insertStringList(mManager->listNames());
    if(index<mCbListSelect->count())
    {
        mCbListSelect->setCurrentItem(index);
    }
    updateGUI();
}

void FeatureDistributionList::updateGUI()
{
    KABC::DistributionList *list = mManager->list(mCbListSelect->currentText());
    if(!list)
    {
        mPbListRename->setEnabled(false);
        mPbListRemove->setEnabled(false);
        mPbChangeEmail->setEnabled(false);
        mPbEntryRemove->setEnabled(false);
        mLvAddressees->setEnabled(false);
        mLvAddressees->clear();
        return;
    } else {
        mPbListRename->setEnabled(true);
        mPbListRemove->setEnabled(true);
        mLvAddressees->setEnabled(true);
        KABC::DistributionList::Entry::List entries = list->entries();
        KABC::DistributionList::Entry::List::ConstIterator it;
        for(it=entries.begin(); it!=entries.end(); ++it)
        {
            new KABC::EntryItem(this, mLvAddressees, (*it).addressee, (*it).email);
        }
    }
  KABC::EntryItem *entryItem = static_cast<KABC::EntryItem *>(mLvAddressees->selectedItem());
  bool state=entryItem;
  mPbChangeEmail->setEnabled(state);
  mPbEntryRemove->setEnabled(state);
}

void FeatureDistributionList::showEvent(QShowEvent *)
{
    update();
}

void FeatureDistributionList::slotListNew()
{
  KLineEditDlg dlg(i18n("Please enter name:"), QString::null, this);
  dlg.setCaption(i18n("New Distribution List"));
  if (!dlg.exec()) return;

  new KABC::DistributionList(mManager, dlg.text());

  mCbListSelect->clear();
  mCbListSelect->insertStringList(mManager->listNames());
  mCbListSelect->setCurrentItem(mCbListSelect->count()-1);

  commit();
  update();
}

void FeatureDistributionList::slotListRename()
{
    QString oldName = mCbListSelect->currentText();

    KLineEditDlg dlg(i18n("Please change name:"), oldName, this);
    dlg.setCaption(i18n("Distribution List"));
    if (!dlg.exec()) return;

    KABC::DistributionList *list=mManager->list(oldName);
    list->setName(dlg.text());

    mCbListSelect->clear();
    mCbListSelect->insertStringList(mManager->listNames());
    mCbListSelect->setCurrentItem(mCbListSelect->count()-1);

    commit();
    update();
}

void FeatureDistributionList::slotListRemove()
{
    int result = KMessageBox::warningContinueCancel
                 (this,
                  i18n("Delete distibution list '%1'?").arg(mCbListSelect->currentText()),
                  QString::null, i18n("Delete"));

    if(result!=KMessageBox::Continue) return;

    delete mManager->list(mCbListSelect->currentText());
    mCbListSelect->removeItem(mCbListSelect->currentItem() );
    commit();
    updateGUI();
}

void FeatureDistributionList::slotEntryChangeEmail()
{
    KABC::DistributionList *list=mManager->list(mCbListSelect->currentText());
    if (!list) return;

    KABC::EntryItem *entryItem =
        static_cast<KABC::EntryItem *>(mLvAddressees->selectedItem());
    if (!entryItem) return;

    QString email=KABC::EmailSelectDialog::getEmail(entryItem->addressee().emails(),
                                                    entryItem->email(), this);
    list->removeEntry(entryItem->addressee(), entryItem->email());
    list->insertEntry(entryItem->addressee(), email);

    commit();
    update();
}

void FeatureDistributionList::slotEntryRemove()
{
    KABC::DistributionList *list=mManager->list(mCbListSelect->currentText());
    if (!list) return;

    KABC::EntryItem *entryItem =
        static_cast<KABC::EntryItem *>( mLvAddressees->selectedItem() );
    if(!entryItem) return;

    list->removeEntry(entryItem->addressee(), entryItem->email());
    delete entryItem;
    commit();
}

void FeatureDistributionList::slotListSelected(int)
{
    update();
}

void FeatureDistributionList::slotAddresseeSelectionChanged()
{
    KABC::EntryItem *entryItem =
        static_cast<KABC::EntryItem *>( mLvAddressees->selectedItem());
    bool state=entryItem;
    mPbChangeEmail->setEnabled(state);
    mPbEntryRemove->setEnabled(state);
}


void FeatureDistributionList::commit()
{
    mManager->save();
    emit(modified());
}

void FeatureDistributionList::dropEvent(QDropEvent *e)
{
    QString clipText;
    if (QTextDrag::decode(e, clipText))
    {
        KABC::Addressee::List aList;
        aList = AddresseeUtil::clipboardToAddressees(clipText);
        KABC::DistributionList *list=mManager->list(mCbListSelect->currentText());
        if ( !list ) {
            kdDebug(5700)
                << "FeatureDistributionList::dropEvent: No dist list '"
                << mCbListSelect->currentText() << "'" << endl;
            return;
            }

        KABC::Addressee::List::Iterator iter;
        for (iter = aList.begin(); iter != aList.end(); ++iter)
        {

            list->insertEntry(*iter);
        }
        commit();
        update();
    }
}

void FeatureDistributionList::slotDropped(QDropEvent *e)
{
    dropEvent(e);
}

#include "featuredistributionlist.moc"
