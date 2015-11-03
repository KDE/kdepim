/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#include "filterimporterexportergui.h"
#include "filter/filterimporterexporter.h"
#include "filter/mailfilter.h"

#include <qapplication.h>
#include <QVBoxLayout>
#include <QMenu>
#include <qmenubar.h>
#include <QTextEdit>

Q_DECLARE_METATYPE(MailCommon::FilterImporterExporter::FilterType)
FilterImporterExporterGui::FilterImporterExporterGui(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    QMenuBar *menuBar = new QMenuBar(this);
    mainLayout->addWidget(menuBar);
    QMenu *menuFilter = menuBar->addMenu(QStringLiteral("filter"));
    QAction *act = new QAction(QStringLiteral("KMail filters"), this);
    act->setData(QVariant::fromValue(MailCommon::FilterImporterExporter::KMailFilter));
    menuFilter->addAction(act);

    act = new QAction(QStringLiteral("Thunderbird filters"), this);

    act->setData(QVariant::fromValue(MailCommon::FilterImporterExporter::ThunderBirdFilter));
    menuFilter->addAction(act);

    act = new QAction(QStringLiteral("Evolution filters"), this);
    act->setData(QVariant::fromValue(MailCommon::FilterImporterExporter::EvolutionFilter));
    menuFilter->addAction(act);

    act = new QAction(QStringLiteral("Sylpheed filters"), this);
    act->setData(QVariant::fromValue(MailCommon::FilterImporterExporter::SylpheedFilter));
    menuFilter->addAction(act);

    act = new QAction(QStringLiteral("Procmail filters"), this);
    act->setData(QVariant::fromValue(MailCommon::FilterImporterExporter::ProcmailFilter));
    menuFilter->addAction(act);

    act = new QAction(QStringLiteral("Balsa filters"), this);
    act->setData(QVariant::fromValue(MailCommon::FilterImporterExporter::BalsaFilter));
    menuFilter->addAction(act);

    act = new QAction(QStringLiteral("Claws Mail filters"), this);
    act->setData(QVariant::fromValue(MailCommon::FilterImporterExporter::ClawsMailFilter));
    menuFilter->addAction(act);

    act = new QAction(QStringLiteral("Icedove Mail filters"), this);
    act->setData(QVariant::fromValue(MailCommon::FilterImporterExporter::IcedoveFilter));
    menuFilter->addAction(act);
    act = new QAction(QStringLiteral("GMail filters"), this);
    act->setData(QVariant::fromValue(MailCommon::FilterImporterExporter::GmailFilter));
    menuFilter->addAction(act);
    connect(menuFilter, SIGNAL(triggered(QAction*)), SLOT(slotImportFilter(QAction*)));

    mTextEdit = new QTextEdit;
    mTextEdit->setReadOnly(true);
    mainLayout->addWidget(mTextEdit);

}

FilterImporterExporterGui::~FilterImporterExporterGui()
{

}

void FilterImporterExporterGui::slotImportFilter(QAction *act)
{
    if (act) {
        importFilters(act->data().value<MailCommon::FilterImporterExporter::FilterType>());
    }
}

void FilterImporterExporterGui::importFilters(MailCommon::FilterImporterExporter::FilterType type)
{
    MailCommon::FilterImporterExporter importer(this);
    bool canceled = false;
    QList<MailCommon::MailFilter *> filters = importer.importFilters(canceled, type);
    if (canceled) {
        mTextEdit->setText(QStringLiteral("Canceled"));
        return;
    }
    QString result;
    Q_FOREACH (MailCommon::MailFilter *filter, filters) {
        if (!result.isEmpty()) {
            result += QLatin1Char('\n');
        }
        result += filter->asString();
    }
    mTextEdit->setText(result);
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    FilterImporterExporterGui *w = new FilterImporterExporterGui();
    w->resize(800, 600);
    w->show();
    app.exec();
    delete w;
    return 0;
}
