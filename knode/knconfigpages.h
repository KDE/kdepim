/*
    knconfigpages.h

    KNode, the KDE newsreader
    Copyright (c) 2004 Volker Krause <volker.krause@rwth-aachen.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

#ifndef KNCONFIGPAGES_H
#define KNCONFIGPAGES_H


#include <kcmodule.h>

class BaseWidget;
class IdentityWidget;

namespace KNConfig {

/*
 * BasePageWithTabs represents a kcm with several tabs.
 * It simply forwards load and save operations to all tabs.
 * Code mostly taken from kmail.
 */
class KDE_EXPORT BasePageWithTabs : public KCModule {
  Q_OBJECT
  public:
    BasePageWithTabs( QWidget * parent=0, const char * name=0 );
    ~BasePageWithTabs() {};

    virtual void load();
    virtual void save();
    virtual void defaults();

  protected:
    void addTab( KCModule* tab, const QString & title );
    
  private:
    QTabWidget *mTabWidget;

};


// accounts page
class AccountsPage : public BasePageWithTabs {
  Q_OBJECT

  public:
    AccountsPage(QWidget *parent = 0, const char *name = 0);
};


// read news page
class KDE_EXPORT ReadNewsPage : public BasePageWithTabs {
  Q_OBJECT
 
  public:
    ReadNewsPage(QWidget *parent = 0, const char *name = 0);
};

// post news page
class KDE_EXPORT PostNewsPage : public BasePageWithTabs {
  Q_OBJECT

  public:
    PostNewsPage(QWidget *parent = 0, const char *name = 0);
};


} //KNConfig

#endif //KNCONFIGPAGES_H
