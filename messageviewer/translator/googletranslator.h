/*
  Copyright (c) 2012 Montel Laurent <montel@kde.org>
  
  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.
  
  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.
  
  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef GOOGLETRANSLATOR_H
#define GOOGLETRANSLATOR_H

#include "abstracttranslator.h"

class QWebView;
class KComboBox;

namespace MessageViewer {
class GoogleTranslator : public AbstractTranslator
{
  Q_OBJECT
public:
  explicit GoogleTranslator();
  ~GoogleTranslator();

  QMap<QString, QMap<QString, QString> > initListLanguage(KComboBox* from);
  void translate();
protected Q_SLOTS:
  void slotLoadFinished(bool result);
private:
  QWebView *mWebView;
};
}

#endif // GOOGLETRANSLATOR_H
