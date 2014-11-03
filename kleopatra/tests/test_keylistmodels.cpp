/* -*- mode: c++; c-basic-offset:4 -*-
    test_keylistmodels.cpp

    This file is part of Kleopatra's test suite.
    Copyright (c) 2007 Klarälvdalens Datakonsult AB

    Kleopatra is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
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

#include <config-kleopatra.h>

#include <models/keylistmodel.h>
#include <models/keylistsortfilterproxymodel.h>

#include <utils/formatting.h>

#include <KAboutData>

#include <QTreeView>
#include <QLineEdit>
#include <QTimer>
#include <QEventLoop>
#include <QDateTime>
#include <QDebug>

#include <qgpgme/eventloopinteractor.h>

#include <gpgme++/context.h>
#include <gpgme++/error.h>
#include <gpgme++/key.h>

#include <memory>
#include <vector>
#include <string>
#include <cassert>
#include <QApplication>
#include <KLocalizedString>
#include <QCommandLineParser>
#include <QCommandLineOption>

class Relay : public QObject
{
    Q_OBJECT
public:
    explicit Relay(QObject *p = 0) : QObject(p) {}

public Q_SLOTS:
    void slotNextKeyEvent(GpgME::Context *, const GpgME::Key &key)
    {
        qDebug("next key");
        mKeys.push_back(key);
        // push out keys in chunks of 1..16 keys
        if (mKeys.size() > qrand() % 16U) {
            emit nextKeys(mKeys);
            mKeys.clear();
        }
    }

    void slotOperationDoneEvent(GpgME::Context *, const GpgME::Error &error)
    {
        qDebug("listing done error: %d", error.encodedError());
    }

Q_SIGNALS:
    void nextKeys(const std::vector<GpgME::Key> &keys);

private:
    std::vector<GpgME::Key> mKeys;
};

int main(int argc, char *argv[])
{

    if (const GpgME::Error initError = GpgME::initializeLibrary(0)) {
        qDebug() << "Error initializing gpgme:" << QString::fromLocal8Bit(initError.asString()) ;
        return 1;
    }

    KAboutData aboutData("test_flatkeylistmodel", 0, i18n("FlatKeyListModel Test"), "0.2");
    QApplication app(argc, argv);
    QCommandLineParser parser;
    KAboutData::setApplicationData(aboutData);
    parser.addVersionOption();
    parser.addHelpOption();
    parser.addOption(QCommandLineOption(QStringList() <<  QLatin1String("flat"), i18n("Perform flat certificate listing")));
    parser.addOption(QCommandLineOption(QStringList() <<  QLatin1String("hierarchical"), i18n("Perform hierarchical certificate listing")));
    parser.addOption(QCommandLineOption(QStringList() <<  QLatin1String("disable-smime"), i18n("Do not list SMIME certificates")));
    parser.addOption(QCommandLineOption(QStringList() <<  QLatin1String("secret"), i18n("List secret keys only")));

    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

    const bool showFlat = parser.isSet(QLatin1String("flat")) || !parser.isSet(QLatin1String("hierarchical"));
    const bool showHier = parser.isSet(QLatin1String("hierarchical")) || !parser.isSet(QLatin1String("flat"));
    const bool disablesmime = parser.isSet(QLatin1String("disable-smime"));
    const bool secretOnly = parser.isSet(QLatin1String("secret"));

    qsrand(QDateTime::currentDateTime().toTime_t());

    QWidget flatWidget, hierarchicalWidget;
    QVBoxLayout flatLay(&flatWidget), hierarchicalLay(&hierarchicalWidget);
    QLineEdit flatLE(&flatWidget), hierarchicalLE(&hierarchicalWidget);
    QTreeView flat(&flatWidget), hierarchical(&hierarchicalWidget);

    flat.setSortingEnabled(true);
    flat.sortByColumn(Kleo::AbstractKeyListModel::Fingerprint, Qt::AscendingOrder);
    hierarchical.setSortingEnabled(true);
    hierarchical.sortByColumn(Kleo::AbstractKeyListModel::Fingerprint, Qt::AscendingOrder);

    flatLay.addWidget(&flatLE);
    flatLay.addWidget(&flat);

    hierarchicalLay.addWidget(&hierarchicalLE);
    hierarchicalLay.addWidget(&hierarchical);

    flatWidget.setWindowTitle(QLatin1String("Flat Key Listing"));
    hierarchicalWidget.setWindowTitle(QLatin1String("Hierarchical Key Listing"));

    Kleo::KeyListSortFilterProxyModel flatProxy, hierarchicalProxy;

    QObject::connect(&flatLE, SIGNAL(textChanged(QString)), &flatProxy, SLOT(setFilterFixedString(QString)));
    QObject::connect(&hierarchicalLE, SIGNAL(textChanged(QString)), &hierarchicalProxy, SLOT(setFilterFixedString(QString)));

    Relay relay;
    QObject::connect(QGpgME::EventLoopInteractor::instance(), SIGNAL(nextKeyEventSignal(GpgME::Context*,GpgME::Key)),
                     &relay, SLOT(slotNextKeyEvent(GpgME::Context*,GpgME::Key)));
    QObject::connect(QGpgME::EventLoopInteractor::instance(), SIGNAL(operationDoneEventSignal(GpgME::Context*,GpgME::Error)),
                     &relay, SLOT(slotOperationDoneEvent(GpgME::Context*,GpgME::Error)));

    if (showFlat)
        if (Kleo::AbstractKeyListModel *const model = Kleo::AbstractKeyListModel::createFlatKeyListModel(&flat)) {
            QObject::connect(&relay, SIGNAL(nextKeys(std::vector<GpgME::Key>)), model, SLOT(addKeys(std::vector<GpgME::Key>)));
            model->setToolTipOptions(Kleo::Formatting::AllOptions);
            flatProxy.setSourceModel(model);
            flat.setModel(&flatProxy);

            flatWidget.show();
        }

    if (showHier)
        if (Kleo::AbstractKeyListModel *const model = Kleo::AbstractKeyListModel::createHierarchicalKeyListModel(&hierarchical)) {
            QObject::connect(&relay, SIGNAL(nextKeys(std::vector<GpgME::Key>)), model, SLOT(addKeys(std::vector<GpgME::Key>)));
            model->setToolTipOptions(Kleo::Formatting::AllOptions);
            hierarchicalProxy.setSourceModel(model);
            hierarchical.setModel(&hierarchicalProxy);

            hierarchicalWidget.show();
        }

    const char *pattern[] = { 0 };

    const std::auto_ptr<GpgME::Context> pgp(GpgME::Context::createForProtocol(GpgME::OpenPGP));
    pgp->setManagedByEventLoopInteractor(true);
    pgp->setKeyListMode(GpgME::Local);

    if (const GpgME::Error e = pgp->startKeyListing(pattern, secretOnly)) {
        qDebug() << "pgp->startKeyListing() ->" << e.asString();
    }

    if (!disablesmime) {
        const std::auto_ptr<GpgME::Context> cms(GpgME::Context::createForProtocol(GpgME::CMS));
        cms->setManagedByEventLoopInteractor(true);
        cms->setKeyListMode(GpgME::Local);

        if (const GpgME::Error e = cms->startKeyListing(pattern, secretOnly)) {
            qDebug() << "cms" << e.asString();
        }

        QEventLoop loop;
        QTimer::singleShot(2000, &loop, SLOT(quit()));
        loop.exec();

        const std::auto_ptr<GpgME::Context> cms2(GpgME::Context::createForProtocol(GpgME::CMS));
        cms2->setManagedByEventLoopInteractor(true);
        cms2->setKeyListMode(GpgME::Local);

        if (const GpgME::Error e = cms2->startKeyListing(pattern, secretOnly)) {
            qDebug() << "cms2" << e.asString();
        }
    }

    return app.exec();
}

#include "test_flatkeylistmodel.moc"
