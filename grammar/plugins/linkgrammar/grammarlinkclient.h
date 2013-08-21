/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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


#ifndef GRAMMARLINKCLIENT_H
#define GRAMMARLINKCLIENT_H

#include "grammar/grammarclient_p.h"
#include "grammar/grammarplugin_p.h"
#include <QtCore/QVariantList>

class GrammarLinkClient : public Grammar::GrammarClient
{
    Q_OBJECT
public:
    GrammarLinkClient(QObject *parent, const QVariantList &);
    ~GrammarLinkClient();

    int reliability() const;
    Grammar::GrammarPlugin *createGrammarChecker(const QString &language);

    QStringList languages() const;

    QString name() const;

private:
    QStringList searchLanguages() const;
};

#endif // GRAMMARLINKCLIENT_H
