/*
  Copyright (c) 2007 Till Adam <adam@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

  In addition, as a special exception, the copyright holders give
  permission to link the code of this program with any edition of
  the Qt library by Trolltech AS, Norway (or with modified versions
  of Qt that use the same license as Qt), and distribute linked
  combinations including the two.  You must obey the GNU General
  Public License in all respects for all of the code used other than
  Qt.  If you modify this file, you may extend this exception to
  your version of the file, but you are not obligated to do so.  If
  you do not wish to do so, delete this exception statement from
  your version.
*/

#include "filterimporterexporter.h"
#include "filterselectiondialog.h"
#include "filteractions/filteraction.h"
#include "filtermanager.h"
#include "mailfilter.h"
#include "mailcommon_debug.h"
#include "filterimporter/filterimporterthunderbird.h"
#include "filterimporter/filterimporterevolution.h"
#include "filterimporter/filterimportersylpheed.h"
#include "filterimporter/filterimporterprocmail.h"
#include "filterimporter/filterimporterbalsa.h"
#include "filterimporter/filterimporterclawsmail.h"
#include "filterimporter/filterimportergmail.h"
#include "dialog/selectthunderbirdfilterfilesdialog.h"


#include <MessageViewer/MessageViewerUtil>

#include <KConfig>

#include <KListWidgetSearchLine>
#include <KMessageBox>
#include <QPushButton>
#include <QFileDialog>

#include <QRegExp>
#include <QFileDialog>
#include <QRegularExpression>

using namespace MailCommon;

QList<MailFilter *> FilterImporterExporter::readFiltersFromConfig(
    const KSharedConfig::Ptr config, QStringList &emptyFilters)
{
    const KConfigGroup group = config->group("General");

    const int numFilters = group.readEntry("filters", 0);

    bool filterNeedUpdate = false;
    QList<MailFilter *> filters;
    for (int i = 0; i < numFilters; ++i) {
        const QString groupName = QStringLiteral("Filter #%1").arg(i);

        const KConfigGroup group = config->group(groupName);
        bool update = false;
        MailFilter *filter = new MailFilter(group, true/*interactive*/, update);
        filter->purify();
        if (update) {
            filterNeedUpdate = true;
        }
        if (filter->isEmpty()) {
#ifndef NDEBUG
            qCDebug(MAILCOMMON_LOG) << "Filter" << filter->asString() << "is empty!";
#endif
            emptyFilters << filter->name();
            delete filter;
        } else {
            filters.append(filter);
        }
    }
    if (filterNeedUpdate) {
        KSharedConfig::Ptr config = KSharedConfig::openConfig(QStringLiteral("akonadi_mailfilter_agentrc"));

        // Now, write out the new stuff:
        FilterImporterExporter::writeFiltersToConfig(filters, config);
        KConfigGroup group = config->group("General");
        group.sync();
    }
    return filters;
}

void FilterImporterExporter::writeFiltersToConfig(const QList<MailFilter *> &filters,
        KSharedConfig::Ptr config,
        bool exportFiler)
{
    // first, delete all filter groups:
    const QStringList filterGroups =
        config->groupList().filter(QRegularExpression(QStringLiteral("Filter #\\d+")));

    foreach (const QString &group, filterGroups) {
        config->deleteGroup(group);
    }

    int i = 0;
    foreach (const MailFilter *filter, filters) {
        if (!filter->isEmpty()) {
            const QString groupName = QStringLiteral("Filter #%1").arg(i);

            KConfigGroup group = config->group(groupName);
            filter->writeConfig(group, exportFiler);
            ++i;
        }
    }

    KConfigGroup group = config->group("General");
    group.writeEntry("filters", i);

    config->sync();
}

class Q_DECL_HIDDEN FilterImporterExporter::Private
{
public:
    Private(QWidget *parent)
        : mParent(parent)
    {
    }
    void warningInfoAboutInvalidFilter(const QStringList &emptyFilters) const;
    QWidget *mParent;
};

void FilterImporterExporter::Private::warningInfoAboutInvalidFilter(
    const QStringList &emptyFilters) const
{
    if (!emptyFilters.isEmpty()) {
        KMessageBox::informationList(
            mParent,
            i18n("The following filters have not been saved because they were invalid "
                 "(e.g. containing no actions or no search rules)."),
            emptyFilters,
            QString(),
            QStringLiteral("ShowInvalidFilterWarning"));
    }
}

FilterImporterExporter::FilterImporterExporter(QWidget *parent)
    : d(new Private(parent))
{
}

FilterImporterExporter::~FilterImporterExporter()
{
    delete d;
}

QList<MailFilter *> FilterImporterExporter::importFilters(
    bool &canceled, FilterImporterExporter::FilterType type, const QString &filename)
{
    QString fileName(filename);

    QFile file;
    if ((type != ThunderBirdFilter) && (type != IcedoveFilter)) {
        if (fileName.isEmpty()) {
            QString title;
            QString defaultPath;
            switch (type) {
            case KMailFilter:
                title = i18n("Import KMail Filters");
                defaultPath = QDir::homePath();
                break;
            case ThunderBirdFilter:
            case IcedoveFilter:
                break;
            case EvolutionFilter:
                title = i18n("Import Evolution Filters");
                defaultPath = MailCommon::FilterImporterEvolution::defaultFiltersSettingsPath();
                break;
            case SylpheedFilter:
                title = i18n("Import Sylpheed Filters");
                defaultPath = MailCommon::FilterImporterSylpheed::defaultFiltersSettingsPath();
                break;
            case ProcmailFilter:
                title = i18n("Import Procmail Filters");
                defaultPath = MailCommon::FilterImporterProcmail::defaultFiltersSettingsPath();
                break;
            case BalsaFilter:
                title = i18n("Import Balsa Filters");
                defaultPath = MailCommon::FilterImporterBalsa::defaultFiltersSettingsPath();
                break;
            case ClawsMailFilter:
                title = i18n("Import Claws Mail Filters");
                defaultPath = MailCommon::FilterImporterClawsMails::defaultFiltersSettingsPath();
                break;
            case GmailFilter:
                title = i18n("Import Gmail Filters");
                defaultPath = MailCommon::FilterImporterGmail::defaultFiltersSettingsPath();
                break;
            }

            fileName = QFileDialog::getOpenFileName(d->mParent, title,  defaultPath);
            if (fileName.isEmpty()) {
                canceled = true;
                return QList<MailFilter *>(); // cancel
            }
        }
        file.setFileName(fileName);
        if (!file.open(QIODevice::ReadOnly)) {
            KMessageBox::error(
                d->mParent,
                i18n("The selected file is not readable. "
                     "Your file access permissions might be insufficient."));
            return QList<MailFilter *>();
        }
    }

    QList<MailFilter *> imported;
    QStringList emptyFilter;

    switch (type) {
    case KMailFilter: {
        const KSharedConfig::Ptr config = KSharedConfig::openConfig(fileName);
        imported = readFiltersFromConfig(config, emptyFilter);
        break;
    }
    case IcedoveFilter:
    case ThunderBirdFilter: {
        if (fileName.isEmpty()) {
            const QString defaultPath = (type == ThunderBirdFilter) ? MailCommon::FilterImporterThunderbird::defaultThunderbirdFiltersSettingsPath() : MailCommon::FilterImporterThunderbird::defaultIcedoveFiltersSettingsPath();

            SelectThunderbirdFilterFilesDialog *selectThunderBirdFileDialog = new SelectThunderbirdFilterFilesDialog(defaultPath, d->mParent);
            selectThunderBirdFileDialog->setStartDir(QUrl::fromLocalFile(defaultPath));
            if (selectThunderBirdFileDialog->exec()) {
                Q_FOREACH (const QString &url, selectThunderBirdFileDialog->selectedFiles()) {
                    QFile fileThunderbird(url);
                    if (!fileThunderbird.open(QIODevice::ReadOnly)) {
                        KMessageBox::error(
                            d->mParent,
                            i18n("The selected file is not readable. "
                                 "Your file access permissions might be insufficient."));
                    } else {

                        MailCommon::FilterImporterThunderbird *thunderBirdFilter =
                            new MailCommon::FilterImporterThunderbird(&fileThunderbird);

                        imported.append(thunderBirdFilter->importFilter());
                        emptyFilter.append(thunderBirdFilter->emptyFilter());
                        delete thunderBirdFilter;
                    }
                }
            } else {
                canceled = true;
                delete selectThunderBirdFileDialog;
                return QList<MailFilter *>();
            }
            delete selectThunderBirdFileDialog;
        } else {
            file.setFileName(fileName);
            if (!file.open(QIODevice::ReadOnly)) {
                KMessageBox::error(
                    d->mParent,
                    i18n("The selected file is not readable. "
                         "Your file access permissions might be insufficient."));
                return QList<MailFilter *>();
            }

            MailCommon::FilterImporterThunderbird *thunderBirdFilter =  new MailCommon::FilterImporterThunderbird(&file);
            imported = thunderBirdFilter->importFilter();
            emptyFilter = thunderBirdFilter->emptyFilter();
            delete thunderBirdFilter;
        }
        break;
    }
    case EvolutionFilter: {
        MailCommon::FilterImporterEvolution *filter =
            new MailCommon::FilterImporterEvolution(&file);

        imported = filter->importFilter();
        emptyFilter = filter->emptyFilter();
        delete filter;
        break;
    }
    case SylpheedFilter: {
        MailCommon::FilterImporterSylpheed *filter =
            new MailCommon::FilterImporterSylpheed(&file);

        imported = filter->importFilter();
        emptyFilter = filter->emptyFilter();
        delete filter;
        break;
    }
    case ProcmailFilter: {
        MailCommon::FilterImporterProcmail *filter =
            new MailCommon::FilterImporterProcmail(&file);

        imported = filter->importFilter();
        emptyFilter = filter->emptyFilter();
        delete filter;
        break;
    }
    case BalsaFilter: {
        MailCommon::FilterImporterBalsa *filter =
            new MailCommon::FilterImporterBalsa(&file);

        imported = filter->importFilter();
        emptyFilter = filter->emptyFilter();
        delete filter;
        break;
    }
    case ClawsMailFilter: {
        MailCommon::FilterImporterClawsMails *filter =
            new MailCommon::FilterImporterClawsMails(&file);

        imported = filter->importFilter();
        emptyFilter = filter->emptyFilter();
        delete filter;
        break;
    }
    case GmailFilter: {
        MailCommon::FilterImporterGmail *filter =
            new MailCommon::FilterImporterGmail(&file);

        imported = filter->importFilter();
        emptyFilter = filter->emptyFilter();
        delete filter;
        break;
    }

    }
    d->warningInfoAboutInvalidFilter(emptyFilter);
    file.close();

    FilterSelectionDialog dlg(d->mParent);
    dlg.setFilters(imported);
    if (dlg.exec() == QDialog::Accepted) {
        return dlg.selectedFilters();
    }
    canceled = true;
    return QList<MailFilter *>();
}

void FilterImporterExporter::exportFilters(const QList<MailFilter *> &filters, const QUrl &fileName, bool saveAll)
{
    QUrl saveUrl;
    if (fileName.isEmpty()) {
        saveUrl = QFileDialog::getSaveFileUrl(d->mParent, i18n("Export Filters"), QDir::homePath());

        if (saveUrl.isEmpty() ||
                !MessageViewer::Util::checkOverwrite(saveUrl, d->mParent)) {
            qDeleteAll(filters);
            return;
        }
    } else {
        saveUrl = fileName;
    }
    KSharedConfig::Ptr config = KSharedConfig::openConfig(saveUrl.toLocalFile());
    if (saveAll) {
        writeFiltersToConfig(filters, config, true);
        //qDeleteAll(filters);
    } else {
        QScopedPointer<FilterSelectionDialog> dlg(new FilterSelectionDialog(d->mParent));
        dlg->setFilters(filters);
        if (dlg->exec() == QDialog::Accepted && dlg) {
            QList<MailFilter *> lst = dlg->selectedFilters();
            writeFiltersToConfig(lst, config, true);
            qDeleteAll(lst);
        }
    }
}
