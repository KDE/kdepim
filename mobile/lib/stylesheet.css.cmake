/*
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/


/*
 * Generic settings
 */

/* font colors */
QCheckBox, QLabel, QPushButton, QRadioButton:off
{
  color: black
}

/* idle button background */
QCheckBox,
QComboBox,
QDateTimeEdit::down-button,
QDateTimeEdit::up-button,
QRadioButton,
QPushButton,
QSpinBox::down-button,
QSpinBox::up-button
{
  border-image: url(@STYLE_IMAGE_PATH@/button-border.png) 10 10 10 10 repeat stretch;
  border-top: 10px;
  border-bottom: 10px;
  border-left: 10px;
  border-right: 10px;
  min-height: 48px;
}

/* active button background */
QCheckBox:pressed,
QComboBox:pressed,
QDateTimeEdit::down-button:pressed,
QDateTimeEdit::up-button:pressed,
QPushButton:pressed,
QRadioButton:on,
QSpinBox::down-button:pressed,
QSpinBox::up-button:pressed
{
  border-image: url(@STYLE_IMAGE_PATH@/button-border-active.png) 10 10 10 10 repeat stretch;
  color: white;
}

/* text input frames */
QDateTimeEdit,
QLineEdit,
QTextEdit,
QSpinBox
{
  background-color: white;
  border: 2px;
  border-color: grey;
  border-radius: 8px;
  border-style: inset;
  padding: 4px;
}



/*
 * Widget specific settings
 */

/* QCheckBox */
QCheckBox:disabled {
  color: grey;
}

QCheckBox::indicator:disabled {
  background-color: rgba(0,0,0,0);
}


/* QComboBox */
QComboBox::drop-down, QComboBox::down-arrow {
  background-color: rgba(0,0,0,0);
}


/* QRadioButton */
QRadioButton::indicator {
  background-color: rgba(0,0,0,0);
  width: 0;
}


/* QScrollArea */
QScrollArea {
  background-color: rgba(0,0,0,0);
}


/* QSpinBox and QDateTimeEdit */
QDateTimeEdit, QSpinBox {
  margin-left: 68px;
  margin-right: 68px;
  margin-top: 16px;
  margin-bottom: 16px;
  text-align: center;
  /* for some reason padding is increased by margin here... */
  padding-left: -64px;
  padding-right: -64px;
}

QDateTimeEdit::down-button,
QDateTimeEdit::up-button,
QSpinBox::down-button,
QSpinBox::up-button
{
  subcontrol-origin: margin;
  height: 48px;
  width: 48px;
}

QDateTimeEdit::down-button,
QSpinBox::down-button
{
  subcontrol-position: left;
}

QDateTimeEdit::up-button,
QSpinBox::up-button
{
  subcontrol-position: right;
}

QDateTimeEdit::down-arrow,
QSpinBox::down-arrow
{
  image: url(@STYLE_IMAGE_PATH@/button-minus.png);
}

QDateTimeEdit::down-arrow:pressed,
QSpinBox::down-arrow:pressed
{
  image: url(@STYLE_IMAGE_PATH@/button-minus-active.png);
}

QDateTimeEdit::down-arrow:disabled,
QDateTimeEdit::down-arrow:off,
QSpinBox::down-arrow:disabled,
QSpinBox::down-arrow:off
{
  image: url(@STYLE_IMAGE_PATH@/button-minus-disabled.png);
}

QDateTimeEdit::up-arrow,
QSpinBox::up-arrow
{
  image: url(@STYLE_IMAGE_PATH@/button-plus.png);
}

QDateTimeEdit::up-arrow:pressed,
QSpinBox::up-arrow:pressed
{
  image: url(@STYLE_IMAGE_PATH@/button-plus-active.png);
}

QDateTimeEdit::up-arrow:disabled,
QDateTimeEdit::up-arrow:off,
QSpinBox::up-arrow:disabled,
QSpinBox::up-arrow:off
{
  image: url(@STYLE_IMAGE_PATH@/button-plus-disabled.png);
}



/*
 * Hacks that should not be necessary at all
 */

/* Recipients editor does not have a transparent background by default on Maemo and tries very hard to avoid getting one :-/ */
MessageComposer--RecipientLine,
MessageComposer--RecipientsView > * > QWidget
{
  background-color: rgba(0,0,0,0);
}
