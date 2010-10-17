/*
    This file is part of Blogilo, A KDE Blogging Client

    It is a modified version of "objecteditorimage.cpp" from
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

#include "objecteditorimage.h"

ObjectEditorImage::ObjectEditorImage(TextEditor *_textEditor, QWidget *parent)
   : ObjectEditor (_textEditor, parent)
{
   setupUi(this);
}

void ObjectEditorImage::editObject(TextEditorObject *_object)
{
   object = dynamic_cast<TextEditorObjectImage*>(_object);
   if (!object)
   {
      qDebug("ObjectEditorImage::editObject(): the received object is not an image object");
      return;
   }
   
   QImage image = object -> getImage();
   widthSpinBox -> setValue(image.width());
   heightSpinBox -> setValue(image.height());
   if (constrainRatioCheckBox -> isChecked())
   {
      ratioWidth = image.width();
      ratioHeight = image.height();
   }
}

void ObjectEditorImage::finishEditing()
{
   
}

// bool ObjectEditorImage::isChanged() const
// {

// }

// const TextEditorObject* ObjectEditorImage::getObject() const
// {

// }

void ObjectEditorImage::on_widthSpinBox_valueChangedByUser(int value)
{
   if (constrainRatioCheckBox -> isChecked())
      heightSpinBox -> setValue(static_cast<double>(value)*ratioHeight/ratioWidth);
}

void ObjectEditorImage::on_heightSpinBox_valueChangedByUser(int value)
{
   if (constrainRatioCheckBox -> isChecked())
      widthSpinBox -> setValue(static_cast<double>(value)*ratioWidth/ratioHeight);
}

void ObjectEditorImage::on_constrainRatioCheckBox_toggled(bool state)
{
   if (state)
   {
      ratioWidth = widthSpinBox -> value();
      ratioHeight = heightSpinBox -> value();
   }
}

void ObjectEditorImage::on_closeButton_clicked()
{
   emit editingFinished();
}

void ObjectEditorImage::on_resizeButton_clicked()
{
   textEditor -> addCommand(new CommandResizeImage(object, QSize(widthSpinBox->value(), heightSpinBox->value())));
}

void ObjectEditorImage::on_flipButton_clicked()
{
   textEditor -> addCommand(new CommandFlipImage(object, Qt::Vertical));
}

void ObjectEditorImage::on_mirrorButton_clicked()
{
   textEditor -> addCommand(new CommandFlipImage(object, Qt::Horizontal));
}

void ObjectEditorImage::on_rotateRightButton_clicked()
{
   textEditor -> addCommand(new CommandRotateImage(object, CommandRotateImage::RotateRight));
}

void ObjectEditorImage::on_rotateLeftButton_clicked()
{
   textEditor -> addCommand(new CommandRotateImage(object, CommandRotateImage::RotateLeft));
}

