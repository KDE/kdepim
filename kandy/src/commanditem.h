// $Id$

#ifndef _COMMAND_ITEM_H_
#define _COMMAND_ITEM_H_

#include <qlistview.h>

class ATCommand;
class ATParameter;
class QDomElement;
class QDomDocument;

/**
  QListView item representing a modem command.
*/
class CommandItem : public QListViewItem {
  public:
    CommandItem(QListView *listView,ATCommand *command);
    ~CommandItem();
    
    ATCommand *command();

    void load(QDomElement *c);
    void save(QDomDocument *doc,QDomElement *parent);
    
    void setItemText();

  protected:
    void saveParameter(ATParameter *p, QDomDocument *doc,QDomElement *parent);
  
  private:
    ATCommand *mCommand;
};

#endif
