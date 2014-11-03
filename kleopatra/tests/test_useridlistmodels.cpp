/* -*- mode: c++; c-basic-offset:4 -*-
    test_useridlistmodels.cpp

    This file is part of Kleopatra's test suite.
    Copyright (c) 2008 Klarälvdalens Datakonsult AB

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

#include <models/useridlistmodel.h>

#include <KAboutData>

#include <QTreeView>

#ifdef KLEO_MODEL_TEST
# include <models/modeltest.h>
#endif

#include <qgpgme/eventloopinteractor.h>

#include <gpgme++/context.h>
#include <gpgme++/error.h>
#include <gpgme++/key.h>
#include <gpgme++/keylistresult.h>

#include <gpg-error.h>

#include <memory>
#include <stdexcept>
#include <vector>
#include <string>
#include <iostream>
#include <cassert>
#include <QApplication>
#include <KLocalizedString>
#include <QCommandLineParser>
#include <QCommandLineOption>

class KeyResolveJob : QObject
{
    Q_OBJECT
public:
    explicit KeyResolveJob(GpgME::Protocol proto = GpgME::OpenPGP, QObject *p = 0)
        : QObject(p),
          m_ctx(GpgME::Context::createForProtocol(proto)),
          m_done(false),
          m_loop(0)
    {
        assert(m_ctx.get());
        connect(QGpgME::EventLoopInteractor::instance(), SIGNAL(nextKeyEventSignal(GpgME::Context*,GpgME::Key)),
                this, SLOT(slotNextKey(GpgME::Context*,GpgME::Key)));
        connect(QGpgME::EventLoopInteractor::instance(), SIGNAL(operationDoneEventSignal(GpgME::Context*,GpgME::Error)),
                this, SLOT(slotDone(GpgME::Context*,GpgME::Error)));

        m_ctx->setManagedByEventLoopInteractor(true);
    }

    GpgME::Error start(const char *pattern, bool secretOnly = false)
    {
        m_ctx->addKeyListMode(GpgME::Signatures | GpgME::SignatureNotations);
        return m_ctx->startKeyListing(pattern, secretOnly);
    }

    GpgME::Error waitForDone()
    {
        if (m_done) {
            return m_error;
        }
        QEventLoop loop;
        m_loop = &loop;
        loop.exec();
        m_loop = 0;
        return m_error;
    }

    std::vector<GpgME::Key> keys() const
    {
        return m_keys;
    }

private Q_SLOTS:
    void slotNextKey(GpgME::Context *ctx, const GpgME::Key &key)
    {
        if (ctx != m_ctx.get()) {
            return;
        }
        m_keys.push_back(key);
    }
    void slotDone(GpgME::Context *ctx, const GpgME::Error &err)
    {
        if (ctx != m_ctx.get()) {
            return;
        }
        m_error = err;
        m_done = true;
        if (m_loop) {
            m_loop->quit();
        }
    }

private:
    std::auto_ptr<GpgME::Context> m_ctx;
    GpgME::Error m_error;
    bool m_done;
    std::vector<GpgME::Key> m_keys;
    QEventLoop *m_loop;
};

using namespace GpgME;
using namespace Kleo;

static void start(const QString &str, Protocol proto)
{
    const QByteArray arg = str.toUtf8();

    KeyResolveJob job(proto);

    if (const GpgME::Error err = job.start(arg)) {
        throw std::runtime_error(std::string("startKeyListing: ") + gpg_strerror(err.encodedError()));
    }

    if (const GpgME::Error err = job.waitForDone()) {
        throw std::runtime_error(std::string("nextKey: ") + gpg_strerror(err.encodedError()));
    }

    const Key key = job.keys().front();

    if (key.isNull()) {
        throw std::runtime_error(std::string("key is null"));
    }

    QTreeView *const tv = new QTreeView;
    tv->setWindowTitle(QString::fromLatin1("UserIDListModel Test - %1").arg(str));

    UserIDListModel *const model = new UserIDListModel(tv);
#ifdef KLEO_MODEL_TEST
    new ModelTest(model);
#endif
    model->setKey(key);

    tv->setModel(model);

    tv->show();
}

int main(int argc, char *argv[])
{

    KAboutData aboutData(QLatin1String("test_useridlistmodels"), i18n("UserIDListModel Test"), QLatin1String("0.1"));
    QApplication app(argc, argv);
    QCommandLineParser parser;
    KAboutData::setApplicationData(aboutData);
    parser.addVersionOption();
    parser.addHelpOption();
    parser.addOption(QCommandLineOption(QStringList() <<  QLatin1String("p"), i18n("OpenPGP certificate to look up"), QLatin1String("pattern")));
    parser.addOption(QCommandLineOption(QStringList() <<  QLatin1String("x"), i18n("X.509 certificate to look up"), QLatin1String("pattern")));

    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

    if (parser.values(QLatin1String("p")).empty() && parser.values(QLatin1String("x")).empty()) {
        return 1;
    }

    try {

        Q_FOREACH (const QString &arg, parser.values(QLatin1String("p"))) {
            start(arg, OpenPGP);
        }

        Q_FOREACH (const QString &arg, parser.values(QLatin1String("x"))) {
            start(arg, CMS);
        }

        return app.exec();

    } catch (const std::exception &e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        return 1;
    }
}

#include "test_useridlistmodel.moc"
