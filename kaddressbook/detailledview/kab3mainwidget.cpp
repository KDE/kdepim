#include <klocale.h>
#include <kconfig.h>
#include <kcompletion.h>
#include <klineedit.h>
#include <kdebug.h>
#include <knotifyclient.h>
#include <qlayout.h>
#include <qcombobox.h>
#include <qsplitter.h>
#include <qpushbutton.h>
#include <qvaluelist.h>
#include <klistview.h>
#include "detailsviewcontainer.h"
#include "kab3mainwidget.h"
#include "kab3mainwidget.moc"
#include "look_details.h"

Kab3MainWidget::Kab3MainWidget(KABC::AddressBook *ab_,
                               QWidget* parent,  const char* name)
    : Kab3MainWidgetBase( parent, name),
      modified(false),
      showList(true),
      showDetails(true),
      m_ab(ab_)
{
    viewTree->addColumn(i18n("Contact"));
    viewTree->addColumn(i18n("Email Address"));
    viewTree->addColumn(i18n("Phone"));
    // -----
    m_look=viewContainer->look();
    connect(this, SIGNAL(readonlyChanged(bool)),
            m_look, SLOT(setReadonly(bool)));
    connect(m_look, SIGNAL(entryChanged()),
            SLOT(slotEntryChanged()));
    // ----- set size relations to 50/50, dunno if this is the
    // intended use but works fine:
    QValueList<int> sizes;
    sizes.append(1); sizes.append(1);
    splitter->setSizes(sizes);
    // ----- set up the incremental search map:
    timerIndex=new QTimer(this);
    connect(timerIndex, SIGNAL(timeout()), SLOT(slotMakeSearchIndex()));
    connect(cbSortBy, SIGNAL(activated(int)),
            SLOT(slotSortByChanged(int)));
    connect(pbHideList, SIGNAL(clicked()),
            SLOT(slotShowHideList()));
    connect(pbHideDetails, SIGNAL(clicked()),
            SLOT(slotShowHideDetails()));
    connect(cbCategory, SIGNAL(activated(int)),
            SLOT(slotCategorySelected(int)));
    // -----
    patterns.setAutoDelete(true);
}

Kab3MainWidget::~Kab3MainWidget()
{
}

void Kab3MainWidget::slotContactSelected(QListViewItem *i)
{
    // ----- save possible changes:
    if(modified)
    {
        commit();
    }
    // ----- find out about the ID of the selected entry:
    Kab3ListViewItem *item=dynamic_cast<Kab3ListViewItem*>(i);
    if (!item)
	return;

    KABC::Addressee addressee=m_ab->findByUid(item->id());
    // ----- display it:
    m_look->setEntry(addressee);
    modified=false;
    emit(selected(item->id()));
}

Kab3ListViewItem::Kab3ListViewItem(QListView *parent, const KABC::Addressee& a,
                                   Kab3NameDisplay d_)
    : QListViewItem(parent),
      m_contact(a),
      m_display(d_)
{
    KABC::PhoneNumber::List phones;
    QString name;
    bool initials=false, reverse=false, business=false;
    switch(m_display)
    {
    case LastNameFirstName:
        reverse=true;
        initials=true;
        business=false;
        break;
    case FirstNameLastName:
        business=false;
        reverse=false;
        initials=false;
        break;
    case BusinessLastName:
    default:
        business=true;
        reverse=false;
        initials=true;
    };
    phones=m_contact.phoneNumbers();
    // could use realname somehow?
    setText(0, fullName(m_contact, reverse, initials, business));
    setText(1, m_contact.preferredEmail());
    if(!phones.isEmpty())
    {
        setText(2, phones[0].number());
    }
}


QString Kab3ListViewItem::id()
{
    return m_contact.uid();
}

KABC::Addressee Kab3ListViewItem::addressee()
{
    return m_contact;
}

void Kab3MainWidget::slotEntryChanged()
{
    kdDebug() << "Kab3MainWidget::slotEntryChanged: addressee modified." << endl;
    modified=true;
    emit(databaseModified());
}

void Kab3MainWidget::slotAddressBookChanged(KABC::AddressBook *ab)
{
    QMap<QString, int> categories;
    QMap<QString, int>::iterator mit;
    QStringList cat;
    QStringList::iterator cit;
    // ab should in fact be m_ab:
    KABC::AddressBook::Iterator it;
    viewTree->clear();
    listViewItems.clear();
    QListViewItem *first;
    QString category;
    // -----
    if(cbCategory->currentItem()==0) // "All"
    {
        category="";
    } else {
        category=cbCategory->currentText();
    }
    kdDebug() << "Kab3MainWidget::slotAddressBookChanged: category is "
              << (category.isEmpty() ? ("empty") : category) << "."
              << endl;
    cbCategory->clear();
    cbCategory->insertItem(i18n("All"));
    // -----
    for(it=ab->begin(); it!=ab->end(); ++it)
    {
        Kab3NameDisplay d=Kab3NameDisplay(cbSortBy->currentItem()>0
                                          ? cbSortBy->currentItem()
                                          : 0);
        if((*it).hasCategory(category) || category.isEmpty())
        {
            listViewItems.append(new Kab3ListViewItem(viewTree, *it, d));
        }
        cat=(*it).categories();
        for(cit=cat.begin(); cit!=cat.end(); ++cit)
        {
            mit=categories.find(*cit);
            if(mit==categories.end())
            {
                categories[*cit]=0;
                cbCategory->insertItem(*cit);
            }
        }
    }
    // ----- reset category selection:
    // ...
    // -----
    first=viewTree->firstChild();
    viewTree->setCurrentItem(first);
    slotContactSelected(first);
    leSearch->setEnabled(false);
    leSearch->setText(i18n("..."));
    leSearch->setCursor(waitCursor);
    timerIndex->start(500, true);
}

void Kab3MainWidget::slotMakeSearchIndex()
{
    leSearch->setText(i18n("Calculating..."));
    KCompletion *comp;
    int count=0;
    bool avail=false;
    KABC::AddressBook::Iterator it;
    QString text, realname, familyName, givenName, addName, formattedName, nick;
    QStringList emails;
    KABC::PhoneNumber::List phones;
    // ----- create the lookup index first:
    for(it=m_ab->begin(); it!=m_ab->end(); ++it)
    {
        realname=(*it).realName();
        if(!realname.isEmpty())
        {
            patterns.append(new Pattern(realname, realname, (*it).uid()));
        }
        familyName=(*it).familyName();
        if(!familyName.isEmpty())
        {
            patterns.append(new Pattern(familyName, realname, (*it).uid()));
        }
        givenName=(*it).givenName();
        if(!givenName.isEmpty())
        {
            patterns.append(new Pattern(givenName, realname, (*it).uid()));
        }
        addName=(*it).additionalName();
        if(!addName.isEmpty())
        {
            patterns.append(new Pattern(addName, realname, (*it).uid()));
        }
        formattedName=(*it).formattedName();
        if(!formattedName.isEmpty())
        {
            patterns.append(new Pattern(formattedName, realname, (*it).uid()));
        }
        nick=(*it).nickName();
        if(!nick.isEmpty())
        {
            patterns.append(new Pattern(nick, realname, (*it).uid()));
        }
        emails=(*it).emails();
        for(unsigned i=0; i<emails.count(); ++i)
        {
            text=emails[i];
            if(!text.isEmpty())
            {
                patterns.append(new Pattern(text, realname, (*it).uid()));
            }
        }
        phones=(*it).phoneNumbers();
        for(unsigned i=0; i<phones.count(); ++i)
        {
            text=phones[i].number();
            if(!text.isEmpty())
            {
                patterns.append(new Pattern(text, realname, (*it).uid()));
            }
        }
        ++count;
    }
    patterns.sort();
    avail=!patterns.isEmpty();
    leSearch->setText(avail ? QString("") : i18n("No items to search)"));
    leSearch->setEnabled(avail);
    leSearch->setCursor(avail ? arrowCursor : forbiddenCursor);
    // ----- put it's contents in the completion object:
    comp=leSearch->completionObject();
    comp->setIgnoreCase(true);
    for(unsigned int i=0; i<patterns.count(); ++i)
    {
        comp->addItem(patterns.at(i)->key());
    }
}

void Kab3MainWidget::slotPatternEntered(const QString& pattern)
{
    unsigned count;
    QString uid;
    // ----- first find the pattern that was selected in the patterns index:
    for(count=0; count<patterns.count(); ++count)
    {
        if(patterns.at(count)->key()==pattern) break;
    }
    if(count==patterns.count())
    {
        KNotifyClient::beep();
    } else {
        // ----- now find the item in the list attached to it:
        uid=patterns.at(count)->uid();
        Kab3ListViewItem* item;
        for(item=listViewItems.first(); item; item=listViewItems.next())
        {
            if(item->id()==uid) break;
        }
        if(item)
        {
            viewTree->setSelected(item, true);
        }
    }
}

void Kab3MainWidget::init(KConfig *config)
{
    m_look->configure(config);
}

void Kab3MainWidget::commit()
{
    m_ab->insertAddressee(m_look->entry());
    modified=false;
}

QString Kab3ListViewItem::fullName(const Addressee& addressee,
                                 bool reverse, bool initials, bool business)
{
    QString result;
    if(addressee.formattedName().stripWhiteSpace().isEmpty())
    {
        QString fam=addressee.familyName();
        QString first=addressee.givenName();
        QString add=addressee.additionalName();
        if(initials)
        {
            if(!first.isEmpty())
            {
                first=first.left(1)+".";
            }
            if(!add.isEmpty())
            {
                add=add.left(1)+".";
            }
        }
        if(reverse)
        {
            result=fam;
            if(!first.isEmpty() || !add.isEmpty())
            {
                result+=QString(", ");
            }
            if(!first.isEmpty())
            {
                result+=first;
            }
            if(!add.isEmpty())
            {
                result+=QString(" ") + add;
            }
        } else {
            result=addressee.formattedName();
        }
    }
//     else {
//         result=addressee.formattedName();
//     }
    if(result.isEmpty())
    {
        result=addressee.realName();
    }
    if(result.isEmpty())
    {
        result=i18n("Unnamed Contact");
    }
    if(business && !addressee.organization().isEmpty())
    {
        result=addressee.organization()+QString(": ")+result;
    }
    return result;
}

void Kab3MainWidget::slotSortByChanged(int)
{
    slotAddressBookChanged(m_ab);
}

void Kab3MainWidget::slotShowHideList()
{
    showList=!showList;
    if(showList)
    {
        frmListView->show();
        pbHideList->setText(i18n("Hide List"));
    } else {
        frmListView->hide();
        pbHideList->setText(i18n("Show List"));
        if(!showDetails)
        {
            slotShowHideDetails();
        }
    }
}
void Kab3MainWidget::slotShowHideDetails()
{
    showDetails=!showDetails;
    if(showDetails)
    {
        frmDetails->show();
        pbHideDetails->setText(i18n("Hide Details"));
    } else {
        frmDetails->hide();
        pbHideDetails->setText(i18n("Show Details"));
        if(!showList)
        {
            slotShowHideList();
        }
    }
}

QStringList Kab3MainWidget::selectedUids()
{
    Kab3ListViewItem *current=(Kab3ListViewItem*)(viewTree->selectedItem());

    if(current!=0)
    {
        QStringList list;
        list.append(current->id());
        return list;
    } else {
        return QStringList();
    }
}

void Kab3MainWidget::setSelected(QString uid, bool selected)
{
    if(uid==QString::null)
    { // do nothing
    } else {
        // look up the list view item in the list:
        Kab3ListViewItem* item;
        for(item=listViewItems.first(); item; item=listViewItems.next())
        {
            if(item->id()==uid) break;
        }
        if(item)
        {
            viewTree->setSelected(item, selected);
        } else {
            kdDebug() << "Kab3MainWidget::setSelected: uid does not exist."
                      << endl;
        }
    }
}

void Kab3MainWidget::slotCategorySelected(int)
{
    slotAddressBookChanged(m_ab);
}
