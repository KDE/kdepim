/*
    This file is part of KContactManager.
    Copyright (c) 1996-2002 Mirko Boehm <mirko@kde.org>
                            Tobias Koenig <tokoe@kde.org>

    Copyright (c) 2009 Laurent Montel <montel@kde.org>

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

#ifndef PRINTINGWIZARD_H
#define PRINTINGWIZARD_H

#include <QtCore/QStringList>

#include <kassistantdialog.h>
#include <kabc/addressbook.h>

#include "printstyle.h"
#include "selectionpage.h"
#include "stylepage.h"


class QPrinter;

namespace Akonadi {
  class EntityTreeView;
}

namespace KABPrinting {

/**
  The PrintingWizard combines pages common for all print styles
  and those provided by the respective style.
*/
class PrintingWizard : public KAssistantDialog
{
  Q_OBJECT

  public:
    /**
      Construct a printing wizard. Give the addressbook instance to print.
     */
    PrintingWizard( QPrinter *printer,
                    Akonadi::EntityTreeView *itemView,
                    QWidget *parent = 0 );
    ~PrintingWizard();

    /**
      Modify this method to add a new PrintStyle.
     */
    void registerStyles();

    /**
      Perform the actual printing.
     */
    void print();

    /**
      Retrieve the printer to be used.
     */
    QPrinter* printer();

  protected Q_SLOTS:
    /**
      A print style has been selected. The argument is the index
      in the cbStyle combo and in styles.
     */
    void slotStyleSelected(int);

  protected:
    QList<PrintStyleFactory*> mStyleFactories;
    QList<PrintStyle*> mStyleList;
    QPrinter *mPrinter;
    Akonadi::EntityTreeView *mItemView;
    PrintStyle *mStyle;

    StylePage *mStylePage;
    SelectionPage *mSelectionPage;

    /**
      Overloaded accept slot. This is used to do the actual
      printing without having the wizard disappearing
      before. What happens is actually up to the print style,
      since it does the printing. It could display a progress
      window, for example (hint, hint).
     */
    void accept();
};

}

#endif
