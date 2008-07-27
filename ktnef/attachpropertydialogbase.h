/***************************************************************************
                        attachpropertydialogbase.h    -  description
                             -------------------
    copyright            : (C) 2006 by Laurent Montel
    email                : montel@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ATTACH_PROPERTY_DIALOGBASE_H
#define ATTACH_PROPERTY_DIALOGBASE_H

#include "ui_attachpropertydialogbase.h"
#include <qdialog.h>

class AttachPropertyDialogBase : public QDialog, public Ui::AttachPropertyDialogBase
{
  Q_OBJECT
  public:
     AttachPropertyDialogBase(QWidget *parent);
};

#endif

