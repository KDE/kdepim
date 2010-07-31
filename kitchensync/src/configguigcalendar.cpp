/*
    This file is part of KitchenSync.

    Copyright (c) 2005 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2006 Eduardo Habkost <ehabkost@raisama.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
    USA.
*/

#include "configguigcalendar.h"

#include <klocale.h>

#include <tqlayout.h>
#include <tqlabel.h>
#include <tqdom.h>
#include <tqlineedit.h>

ConfigGuiGoogleCalendar::ConfigGuiGoogleCalendar( const QSync::Member &member, TQWidget *parent )
  : ConfigGui( member, parent )
{
  TQBoxLayout *userLayout = new TQHBoxLayout( topLayout() );

  TQLabel *userLbl= new TQLabel( i18n("Username:"), this );
  userLayout->addWidget(userLbl);

  mUsername = new TQLineEdit(this);
  userLayout->addWidget(mUsername);


  TQBoxLayout *passLayout = new TQHBoxLayout( topLayout() );

  TQLabel *passLbl = new TQLabel( i18n("Password:"), this );
  passLayout->addWidget(passLbl);

  mPassword = new TQLineEdit(this);
  mPassword->setEchoMode(TQLineEdit::Password);
  passLayout->addWidget(mPassword);

  topLayout()->addWidget(new TQLabel( i18n("Please notice that currently the password is stored as plain text in the plugin configuration file"), this ));

  TQBoxLayout *urlLayout = new TQHBoxLayout( topLayout() );
  TQLabel *urlLbl = new TQLabel( i18n("Calendar URL:"), this );
  urlLayout->addWidget(urlLbl);

  mUrl = new TQLineEdit(this);
  urlLayout->addWidget(mUrl);

  topLayout()->addStretch( 1 );
}

void ConfigGuiGoogleCalendar::load( const TQString &xml )
{
  TQDomDocument doc;
  doc.setContent( xml );
  TQDomElement docElement = doc.documentElement();
  TQDomNode n;
  for( n = docElement.firstChild(); !n.isNull(); n = n.nextSibling() ) {
    TQDomElement e = n.toElement();
    if ( e.tagName() == "username" ) {
      mUsername->setText(e.text());
    } else if ( e.tagName() == "password" ) {
      mPassword->setText(e.text());
    } else if ( e.tagName() == "url" ) {
      mUrl->setText(e.text());
    }
  }
}

TQString ConfigGuiGoogleCalendar::save() const
{
  TQDomDocument doc;
  TQDomElement root = doc.createElement("config");
  doc.appendChild(root);

  TQDomElement un = doc.createElement("username");
  root.appendChild(un);
  un.appendChild(doc.createTextNode(mUsername->text()));

  TQDomElement pass = doc.createElement("password");
  root.appendChild(pass);
  pass.appendChild(doc.createTextNode(mPassword->text()));

  TQDomElement url = doc.createElement("url");
  root.appendChild(url);
  url.appendChild(doc.createTextNode(mUrl->text()));

  //TODO: Implement me!
  return doc.toString();
}
