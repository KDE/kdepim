// $Id$

#ifndef COMMANDSET_H
#define COMMANDSET_H

#include <qlistview.h>

class ATCommand;
class ATParameter;
class QDomElement;
class QDomDocument;

/**
  QListView item representing a modem command.
*/
class CommandSet {
  public:
    CommandSet();
    ~CommandSet();
    
    void addCommand(ATCommand *);
    void deleteCommand(ATCommand *);
    
    bool loadFile(const QString &);
    bool saveFile(const QString &);
    
    void clear();
    
    QList<ATCommand> *commandList() { return &mList; }
    
  protected:
    void loadCommand(ATCommand *,QDomElement *c);
    void saveCommand(ATCommand *,QDomDocument *doc,QDomElement *parent);
    void saveParameter(ATParameter *p, QDomDocument *doc,QDomElement *parent);
  
  private:
    QList<ATCommand> mList;
};

#endif
