/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#include <QWidget>
#include <QVBoxLayout>

#include <kaboutdata.h>
#include <kapplication.h>
#include <qdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>

#include "../widgets/spellchecklineedit.h"

int main(int argc, char* argv[])
{
  KAboutData aboutData("testspellchecklineedit", 0,ki18n("Test SpellCheckLineEdit"),"0.1");
  KCmdLineArgs::init(argc,argv,&aboutData);

  KApplication app;
  QWidget *w = new QWidget;
  QVBoxLayout *vbox = new QVBoxLayout(w);

  KPIM::SpellCheckLineEdit *spellCheckLineEdit = new KPIM::SpellCheckLineEdit(w, QLatin1String("testspecklineeditrc"));
  vbox->addWidget(spellCheckLineEdit);
  vbox->addStretch();

  w->resize( 400, 400 );
  w->show();

  return app.exec();

}
    
