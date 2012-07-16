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

#ifndef BABELFISHTRANSLATOR_H
#define BABELFISHTRANSLATOR_H

#include "abstracttranslator.h"
#include <kio/job.h>

namespace MessageViewer {
class BabelFishTranslator : public MessageViewer::AbstractTranslator
{
  Q_OBJECT
public:
  explicit BabelFishTranslator();
  ~BabelFishTranslator();

  QMap<QString, QMap<QString, QString> > initListLanguage(KComboBox* from);
  void translate();
protected Q_SLOTS:
  void slotJobDone(KJob *job);
};
}

#endif // BABELFISHTRANSLATOR_H
