#ifndef KAB3MAINWIDGET_H
#define KAB3MAINWIDGET_H

#include <qlistview.h>
#include <qtimer.h>
#include <kabc/addressbook.h>
#include "kab3mainwidget_base.h"

class KConfig;

class KABBasicLook;

/**
  @internal
*/
class Pattern
{
protected:
    QString mKey;
    QString mDesc;
    QString mUid; // unique ID of the addressee
public:
    Pattern(QString k=QString::null, QString d=QString::null,
            QString i=QString::null)
        : mKey(k), mDesc(d), mUid(i) {}
    /** The pattern that is searched for. */
    QString key() { return mKey; }
    void setKey(const QString& k) { mKey=k; }
    /** The descriptive string displayed as a possible match. */
    QString desc() { return mDesc; }
    void setDesc(const QString& d) { mDesc=d; }
    /** The UID of the hit in the database. */
    const QString& uid() { return mUid; }
    void setUid(const QString& i) { mUid=i; }
};

/**
  @internal
*/
class PatternList : public QPtrList<Pattern>
{
public:
    PatternList() : QPtrList<Pattern> () {}
protected:
    int compareItems(QPtrCollection::Item i1, QPtrCollection::Item i2)
        {
            Pattern* item1=(Pattern*) i1;
            Pattern* item2=(Pattern*) i2;
            // ----- first compare the key:
            if(item1->key()>item2->key()) return 1;
            if(item1->key()<item2->key()) return -1;
            // ----- then the description:
            if(item1->desc()>item2->desc()) return 1;
            if(item1->desc()<item2->desc()) return -1;
            // ----- this items are equal
            return 0;
        }
};

using namespace KABC;

enum Kab3NameDisplay
{
    LastNameFirstName,
    FirstNameLastName,
    BusinessLastName
};

class Kab3ListViewItem : public QListViewItem
{
public:
    Kab3ListViewItem(QListView *parent, const Addressee&, Kab3NameDisplay d);
    /** Return the id of the addressee asigned to this list view
        item. Use this method to look up the right database entry.
    */
    QString id();
    /** Return the entry associated with the item.
     */
    Addressee addressee();
    /** Construct a name from various fields.
        This should be moved to Addressee after 3.0.
        Please note that reverse and initials are overriden by the formatted name!
        @param reverse If true, first names appear after the last name
        @param initials If true, first names appear as initials
        @param business If true, the business (organization) is put in front of
        the name
    */
    QString fullName(const Addressee& addressee, bool reverse=false,
                     bool initials=false, bool business=false);
protected:
    Addressee m_contact;
    Kab3NameDisplay m_display;
};

class Kab3MainWidget : public Kab3MainWidgetBase
{
    Q_OBJECT
public:
    Kab3MainWidget(AddressBook *ab_,
                   QWidget* parent = 0, const char* name = 0);
    ~Kab3MainWidget();
    /** Initialize from the settings in the configuration file. */
    void init(KConfig *);
    /** Put all changes back into the database. */
    void commit();
    /** Return the current selection of entries by their uids. */
    QStringList selectedUids();
    /** Set the selection according to the parameters. */
    void setSelected(QString uid, bool selected);
public slots:
    virtual void slotAddressBookChanged(AddressBook*);
    virtual void slotContactSelected( QListViewItem * item );
    virtual void slotEntryChanged();
    virtual void slotMakeSearchIndex();
    virtual void slotCategorySelected(int);
    void slotSortByChanged(int);
    void slotShowHideList();
    void slotShowHideDetails();
signals:
    void databaseModified();
    void selected(const QString&);
protected:
    KABBasicLook *m_look;
    /** THe current entry has been modified. */
    bool modified;
    /** Is the list view visible? */
    bool showList;
    /** Is the details frame visible? */
    bool showDetails;
    AddressBook *m_ab;
    /** Timer to start creating the incremental search map in the
        background. */
    QTimer *timerIndex;
    /** The list view items. */
    QPtrList<Kab3ListViewItem> listViewItems;
    /** Stored information about the incremental search index. */
    PatternList patterns;
    /** Overloaded from the base class. */
    void slotPatternEntered(const QString& text);
    /** Handle close events. */
    bool queryClose();
};

#endif // KAB3MAINWIDGET_H
