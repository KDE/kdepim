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

#include "customtextedit.h"

#include <KLocale>
#include <sonnet/speller.h>

#include <QActionGroup>
#include <QMenu>
#include <QAction>

using namespace PimCommon;
class CustomTextEdit::Private
{
public:
    Private(const QString &_configFile)
        : configFile(_configFile),
          speller(0)
    {
    }
    ~Private()
    {
        delete speller;
    }

    QString configFile;
    Sonnet::Speller* speller;
};

CustomTextEdit::CustomTextEdit(QWidget *parent)
    : KTextEdit(parent), d(new Private(QString()))
{
    connect(this, SIGNAL(aboutToShowContextMenu(QMenu*)), this, SLOT(insertLanguageMenu(QMenu*)));
}

CustomTextEdit::CustomTextEdit(const QString &configName, QWidget *parent)
    : KTextEdit(parent), d(new Private(configName))
{
    connect(this, SIGNAL(aboutToShowContextMenu(QMenu*)), this, SLOT(insertLanguageMenu(QMenu*)));
}

CustomTextEdit::~CustomTextEdit()
{
    delete d;
}

void CustomTextEdit::setConfigName(const QString &name)
{
    d->configFile = name;
}

QString CustomTextEdit::configName() const
{
    return d->configFile;
}

void CustomTextEdit::createHighlighter()
{
    Sonnet::Highlighter *highlighter = new Sonnet::Highlighter(this, d->configFile);
    highlighter->setAutomatic( false );

    KTextEdit::setHighlighter(highlighter);

    if (!spellCheckingLanguage().isEmpty()) {
        setSpellCheckingLanguage( spellCheckingLanguage() );
    }
}

static inline QString i18n_kdelibs4(const char *str) { return ki18n(str).toString(QLatin1String("kdelibs4")); }

void CustomTextEdit::insertLanguageMenu(QMenu* contextMenu)
{
    if (!checkSpellingEnabled())
        return;
    QAction* spellCheckAction = 0;

    foreach (QAction* action, contextMenu->actions()) {
        if (action->text() == i18n_kdelibs4("Auto Spell Check")) {
            spellCheckAction = action;
            break;
        }
    }

    if (spellCheckAction) {
        QMenu* languagesMenu = new QMenu(i18n("Spell Checking Language"), contextMenu);
        QActionGroup* languagesGroup = new QActionGroup(languagesMenu);
        languagesGroup->setExclusive(true);

        if (!d->speller)
            d->speller = new Sonnet::Speller();

        QMapIterator<QString, QString> i(d->speller->availableDictionaries());

        while (i.hasNext()) {
            i.next();

            QAction* languageAction = languagesMenu->addAction(i.key());
            languageAction->setCheckable(true);
            languageAction->setChecked(spellCheckingLanguage() == i.value() || (spellCheckingLanguage().isEmpty()
                && d->speller->defaultLanguage() == i.value()));
            languageAction->setData(i.value());
            languageAction->setActionGroup(languagesGroup);
            connect(languageAction, SIGNAL(triggered(bool)), this, SLOT(languageSelected()));
        }

        contextMenu->insertMenu(spellCheckAction, languagesMenu);
    }
}

void CustomTextEdit::languageSelected()
{
    QAction* languageAction = static_cast<QAction*>(QObject::sender());
    setSpellCheckingLanguage(languageAction->data().toString());
}


#include "customtextedit.moc"
