/* Simple Addressbook for KMail
 * Author: Stefan Taferner <taferner@kde.org>
 * This code is under GPL
 */
#ifndef KMAddrBook_h
#define KMAddrBook_h

#include <qstrlist.h>

#define KMAddrBookInherited QStrList
class KMAddrBook: protected QStrList
{
public:
  KMAddrBook();
  virtual ~KMAddrBook();

  /** Insert given address to the addressbook. Sorted. Duplicate
    addresses are not inserted. */
  virtual void insert(const QString address);

  /** Remove given address from the addressbook. */
  virtual void remove(const QString address);

  /** Returns first address in addressbook or NULL if addressbook is empty. */
  virtual QString first(void) { return KMAddrBookInherited::first(); }

  /** Returns next address in addressbook or NULL. */
  virtual QString next(void) { return KMAddrBookInherited::next(); }

  /** Clear addressbook (remove the contents). */
  virtual void clear(void);

  /** Open addressbook file and read contents. The default addressbook
    file is used if no filename is given.
    Returns IO_Ok on success and an IO device status on failure -- see
    QIODevice::status(). */
  virtual int load(const QString &fileName=QString::null);

  /** Store addressbook in file or in same file of last load() call
    if no filename is given. Returns IO_Ok on success and an IO device
    status on failure -- see QIODevice::status(). */
  virtual int store(const QString &fileName=QString::null);

  /** Read/write configuration options. Uses the group "Addressbook"
    in the app's config file. */
  virtual void readConfig(void);
  virtual void writeConfig(bool withSync=TRUE);

  /** Test if the addressbook has unsaved changes. */
  virtual bool modified(void) const { return mModified; }

protected:
  virtual int compareItems(Item item1, Item item2);

  /** Displays a detailed message box and returns 'status' */
  virtual int fileError(int status) const;

  QString mDefaultFileName;
  bool mModified;
};

#endif /*KMAddrBook_h*/
