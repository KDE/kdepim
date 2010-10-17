/*
    This file is part of Blogilo, A KDE Blogging Client

    It is a modified version of "objecteditorimage.h" from
    FlashQard project.

    Copyright (C) 2008-2009 Shahab <shahab@flashqard-project.org>
    Copyright (C) 2010 Mehrdad Momeny <mehrdad.momeny@gmail.com>
    Copyright (C) 2010 Golnaz Nilieh <g382nilieh@gmail.com>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy
    defined in Section 14 of version 3 of the license.


    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, see http://www.gnu.org/licenses/
*/


#ifndef OBJECTEDITORIMAGE_H
#define OBJECTEDITORIMAGE_H

#include "objecteditor.h"
#include "ui_objecteditorimage.h"

class ObjectEditorImage : public ObjectEditor , public Ui::ObjectEditorImage
{
   Q_OBJECT

   public:
      ObjectEditorImage(TextEditor*, QWidget* = 0);

      virtual void editObject(TextEditorObject*);
      virtual void finishEditing();
//      virtual bool isChanged() const;
//      virtual const TextEditorObject* getObject() const;

   private slots:
      void on_widthSpinBox_valueChangedByUser(int);
      void on_heightSpinBox_valueChangedByUser(int);
      void on_constrainRatioCheckBox_toggled(bool);
      void on_closeButton_clicked();
      void on_resizeButton_clicked();
      void on_flipButton_clicked();
      void on_mirrorButton_clicked();
      void on_rotateRightButton_clicked();
      void on_rotateLeftButton_clicked();

   private:
      TextEditorObjectImage* object;
      int ratioWidth;
      int ratioHeight;
};


#endif
