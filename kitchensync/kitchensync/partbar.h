/* This file is part of the KDE libraries
   Copyright (C) 2001,2002 Carsten Pfeiffer <pfeiffer@kde.org>
   for his work on KURLBar and related classes


*/

#include <klistbox.h>

#include "manipulatorpart.h"

namespace KitchenSync {
  class PartBar;
  class PartBarItem : public QListBoxPixmap
    {
    public:
      PartBarItem( PartBar*, ManipulatorPart * );
      ~PartBarItem();

      ManipulatorPart* part();
      /**
       * returns the width of this item.
       */
      virtual int width( const QListBox * ) const;
      /**
       * returns the height of this item.
       */
      virtual int height( const QListBox * ) const;

    private:
      ManipulatorPart m_Part;

    };

  class KListBox;
  class PartBar  : public QFrame
    {
      //Q_OBJECT
    public:
      PartPar( QWidget *parent = 0, const char *name = 0, WFlags f = 0 );
      ~PartBar();
      virtual PartBarItem* insertItem(ManipulatorPart *part );

      virtual void setListBox(  KListBox*);
      KListBox *listBox() const { return m_listBox; }
      virtual void clear();
      virtual QSize sizeHint() const;
      virtual QSize minimumSizeHint() const;

      PartBarItem *currentItem() const;

    signals:
      void activated( PartManipulator *part );

    protected slots:
      virtual void slotSelected( QListBoxItem * );

    protected:
      virtual void resizeEvent( QResizeEvent * );

    private:
      KListBox *m_listBox;

    };



};






  };

}
