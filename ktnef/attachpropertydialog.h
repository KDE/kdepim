/*
  This file is part of KTnef.

  Copyright (C) 2002 Michael Goffioul <kdeprint@swing.be>
  Copyright (c) 2012 Allen Winter <winter@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software Foundation,
  Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
*/

#ifndef ATTACHPROPERTYDIALOG_H
#define ATTACHPROPERTYDIALOG_H

#include "ui_attachpropertydialogbase.h"

#include <QDialog>
#include <QMap>
#include <QPixmap>

namespace KTnef {
  class KTNEFAttach;
  class KTNEFProperty;
  class KTNEFPropertySet;
}
using namespace KTnef;

class QTreeWidget;
class QTreeWidgetItem;


class AttachPropertyDialog : public QDialog
{
  public:
    explicit AttachPropertyDialog( QWidget *parent = 0 );
    ~AttachPropertyDialog();

    void setAttachment( KTNEFAttach *attach );

  public slots:
    void accept();

  protected:
    Ui::AttachPropertyDialogBase mUI;

  private:
    KTNEFAttach *mAttach;
};

void formatProperties( const QMap<int,KTNEFProperty*> &, QTreeWidget *,
                       QTreeWidgetItem *, const QString & = QLatin1String("prop") );
void formatPropertySet( KTNEFPropertySet *, QTreeWidget * );
void saveProperty( QTreeWidget *, KTNEFPropertySet *, QWidget * );
QPixmap loadRenderingPixmap( KTNEFPropertySet *, const QColor & );

#endif
