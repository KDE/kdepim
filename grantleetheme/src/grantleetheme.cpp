/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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

#include "grantleetheme.h"
#include "grantleetheme_p.h"
#include "grantleetheme_debug.h"
#include "config-grantleetheme.h"

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>

#include <QDir>
#include <QSharedPointer>

using namespace GrantleeTheme;

QSharedPointer<GrantleeKi18nLocalizer> GrantleeTheme::ThemePrivate::sLocalizer;
Grantlee::Engine *GrantleeTheme::ThemePrivate::sEngine = Q_NULLPTR;

ThemePrivate::ThemePrivate()
    : QSharedData()
{
}

ThemePrivate::ThemePrivate(const ThemePrivate &other)
    : QSharedData(other)
    , displayExtraVariables(other.displayExtraVariables)
    , themeFileName(other.themeFileName)
    , description(other.description)
    , name(other.name)
    , dirName(other.dirName)
    , absolutePath(other.absolutePath)
    , author(other.author)
    , email(other.email)
    , loader(other.loader)
{
}

ThemePrivate::~ThemePrivate()
{
}

void ThemePrivate::setupEngine()
{
    sEngine = new Grantlee::Engine;
    sEngine->addPluginPath(QStringLiteral(GRANTLEE_PLUGIN_INSTALL_DIR));
    sEngine->addDefaultLibrary(QStringLiteral("grantlee_i18ntags"));
    sEngine->addDefaultLibrary(QStringLiteral("kde_grantlee_plugin"));
    sEngine->setSmartTrimEnabled(true);
}

void ThemePrivate::setupLoader()
{
    // Get the parent dir with themes, we set the theme directory separately
    QDir dir(absolutePath);
    dir.cdUp();

    loader = QSharedPointer<Grantlee::FileSystemTemplateLoader>::create();
    loader->setTemplateDirs({ dir.absolutePath() });
    loader->setTheme(dirName);

    if (!sEngine) {
        ThemePrivate::setupEngine();
    }
    sEngine->addTemplateLoader(loader);
}

Grantlee::Context ThemePrivate::createContext(const QVariantHash &data)
{
    if (!sLocalizer) {
        sLocalizer.reset(new GrantleeKi18nLocalizer());
    }

    Grantlee::Context ctx(data);
    ctx.setLocalizer(sLocalizer);
    return ctx;
}

QString ThemePrivate::errorTemplate(const QString &reason,
                                    const QString &origTemplateName,
                                    const Grantlee::Template &failedTemplate)
{
    Grantlee::Template tpl = sEngine->newTemplate(
                                 QStringLiteral("<h1>{{ error }}</h1>\n"
                                         "<b>%1:</b> {{ templateName }}<br>\n"
                                         "<b>%2:</b> {{ errorMessage }}")
                                 .arg(i18n("Template"), i18n("Error message")),
                                 QStringLiteral("TemplateError"));

    Grantlee::Context ctx = createContext();
    ctx.insert(QStringLiteral("error"), reason);
    ctx.insert(QStringLiteral("templateName"), origTemplateName);
    ctx.insert(QStringLiteral("errorMessage"), failedTemplate->errorString());
    return tpl->render(&ctx);
}

Theme::Theme()
    : d(new ThemePrivate)
{
}

Theme::Theme(const QString &themePath, const QString &dirName, const QString &defaultDesktopFileName)
    : d(new ThemePrivate)
{
    const QString themeInfoFile = themePath + QDir::separator() + defaultDesktopFileName;
    KConfig config(themeInfoFile);
    KConfigGroup group(&config, QStringLiteral("Desktop Entry"));
    if (group.isValid()) {
        d->dirName = dirName;
        d->absolutePath = themePath;
        d->name = group.readEntry("Name", QString());
        d->description = group.readEntry("Description", QString());
        d->themeFileName = group.readEntry("FileName", QString());
        d->displayExtraVariables = group.readEntry("DisplayExtraVariables", QStringList());
    }
}

Theme::Theme(const Theme &other)
    : d(other.d)
{
}

Theme::~Theme()
{
}

bool Theme::operator==(const Theme &other) const
{
    return isValid() && other.isValid() && d->absolutePath == other.absolutePath();
}

Theme &Theme::operator=(const Theme &other)
{
    if (this != &other) {
        d = other.d;
    }

    return *this;
}

bool Theme::isValid() const
{
    return !d->themeFileName.isEmpty() && !d->name.isEmpty();
}

QString Theme::description() const
{
    return d->description;
}

QString Theme::themeFilename() const
{
    return d->themeFileName;
}

QString Theme::name() const
{
    return d->name;
}

QStringList Theme::displayExtraVariables() const
{
    return d->displayExtraVariables;
}

QString Theme::dirName() const
{
    return d->dirName;
}

QString Theme::absolutePath() const
{
    return d->absolutePath;
}

QString Theme::author() const
{
    return d->author;
}

QString Theme::authorEmail() const
{
    return d->email;
}

QString Theme::render(const QString &templateName, const QVariantHash &data)
{
    if (!d->loader) {
        d->setupLoader();
    }
    Q_ASSERT(d->loader);

    if (!d->loader->canLoadTemplate(templateName)) {
        qCWarning(GRANTLEETHEME_LOG) << "Cannot load template" << templateName << ", please check your installation";
        return QString();
    }

    Grantlee::Template tpl = d->loader->loadByName(templateName, ThemePrivate::sEngine);
    if (tpl->error()) {
        return d->errorTemplate(i18n("Template parsing error"), templateName, tpl);
    }

    Grantlee::Context ctx = d->createContext(data);
    const QString result = tpl->render(&ctx);
    if (tpl->error()) {
        return d->errorTemplate(i18n("Template rendering error"), templateName, tpl);
    }

    return result;
}

void Theme::addPluginPath(const QString &path)
{
    if (!ThemePrivate::sEngine) {
        ThemePrivate::setupEngine();
    }

    QStringList paths = ThemePrivate::sEngine->pluginPaths();
    if (!paths.contains(path)) {
        paths.prepend(path);
    }
    ThemePrivate::sEngine->setPluginPaths(paths);
}
