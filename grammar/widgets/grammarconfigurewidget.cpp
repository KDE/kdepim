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

#include "grammarconfigurewidget.h"
#include "grammarcomboboxlanguage.h"
#include "grammarloader.h"
#include "grammarsettings.h"

#include <KLocale>
#include <KConfig>
#include <KConfigGroup>

#include <QHBoxLayout>
#include <QLabel>

namespace Grammar {

class GrammarConfigureWidgetPrivate {
public:
    GrammarConfigureWidgetPrivate(KConfig *_config, GrammarConfigureWidget *qq)
        : q(qq),
          config(_config)
    {
        init();
    }

    void init()
    {
        loader = GrammarLoader::openGrammarLoader();
        KConfigGroup group = config->group(QLatin1String("General"));
        loader->settings()->readSettings(group);
        QHBoxLayout *lay = new QHBoxLayout;
        QLabel *lab = new QLabel(i18n("Default language:"));
        lay->addWidget(lab);

        language = new GrammarComboBoxLanguage;
        lay->addWidget(language);

        q->setLayout(lay);
    }

    void setDefault()
    {
        //TODO
    }

    void save()
    {
        KConfigGroup group = config->group(QLatin1String("General"));
        loader->settings()->setDefaultLanguage(language->currentLanguage());
        loader->settings()->saveSettings(group);
    }

    void setLanguage(const QString &lang)
    {
        language->setCurrentLanguage(lang);
    }

    GrammarLoader *loader;
    GrammarComboBoxLanguage *language;
    GrammarConfigureWidget *q;
    KConfig *config;
};

GrammarConfigureWidget::GrammarConfigureWidget(KConfig *config, QWidget *parent)
    : QWidget(parent), d(new GrammarConfigureWidgetPrivate(config, this))
{
}

GrammarConfigureWidget::~GrammarConfigureWidget()
{
    delete d;
}

void GrammarConfigureWidget::setDefault()
{
    d->setDefault();
}

void GrammarConfigureWidget::setLanguage(const QString &lang)
{
    d->setLanguage(lang);
}

void GrammarConfigureWidget::save()
{
    d->save();
}

}

