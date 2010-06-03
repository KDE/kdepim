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

QCheckBox, QLabel, QPushButton {
  color: black
}

QCheckBox, QComboBox, QPushButton {
  border-image: url(@STYLE_IMAGE_PATH@/button-border.png) 10 10 10 10;
  border-top: 10px;
  border-bottom: 10px;
  border-left: 10px;
  border-right: 10px;
  min-height: 48px;
}

QLineEdit, QTextEdit {
  border: 2px;
  border-color: grey;
  border-radius: 8px;
  border-style: inset;
  padding: 4px;
}

QCheckBox:disabled {
  color: grey;
}

QCheckBox::indicator:disabled {
  background-color: rgba(0,0,0,0);
}

QComboBox::drop-down, QComboBox::down-arrow {
  background-color: rgba(0,0,0,0);
}

