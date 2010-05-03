#include "mailwindow.h"

#include <QDateTime>
#include <QDir>

#include <grantlee/context.h>
#include <grantlee/engine.h>
#include "grantlee_paths.h"

MailWindow::MailWindow(QWidget *parent)
    : QMainWindow(parent)
{
    splitter = new QSplitter(Qt::Horizontal,this);
    webView = new QWebView( splitter );

    engine = new Grantlee::Engine();
    Grantlee::FileSystemTemplateLoader::Ptr loader(new Grantlee::FileSystemTemplateLoader());

    loader->setTemplateDirs( QStringList() << GRANTLEE_TEMPLATE_PATH );

    engine->addTemplateLoader( loader );
    engine->setPluginPaths( QStringList() << GRANTLEE_PLUGIN_PATH );

    renderMail();
    setCentralWidget( splitter );
}

void MailWindow::renderMail()
{
    QString themeName = "kde.html";
    Grantlee::Template t = engine->loadByName( themeName );

    QString message;
            message += "I've created the branch branches/work/soc-pim ";
            message += "The branch is intended to be shared by all PIM-related GSoC projects,";
            message += "Happy Summer of Code to all the students.\n";

    QDateTime dateTime = QDateTime::currentDateTime();

    QVariantHash data;
    data.insert( "subject" , " [Kde-pim] Work branches for GSoC " );
    data.insert( "from" , " PIM Hacker <pimhacker@kde.org> " );
    data.insert( "to" , " KDE PIM <kde-pim@kde.org> " );
    data.insert( "date" , dateTime );
    data.insert( "message" , message);

    Grantlee::Context c( data );
    QString content = t->render(&c);

    webView->setHtml( content , QUrl::fromLocalFile(QDir::currentPath()) );
    splitter->addWidget( webView );
}

MailWindow::~MailWindow()
{

}

#include "mailwindow.moc"
