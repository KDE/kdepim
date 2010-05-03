/*
  Example of theming using Grantlee.

  Copyright (c) 2010 Ronny Yabar Aizcorbe <ronnycontacto@gmail.com>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License version 3 only, as published by the Free Software Foundation.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License version 3 for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <QtGui/QApplication>
#include "mailwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MailWindow w;
    w.resize( 620,600 );
    w.show();
    return a.exec();
}
