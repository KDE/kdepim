
#include "../thememanager.h"

#include <qtest_kde.h>

#include <QtCore/QObject>

class ThemeManagerTest : public QObject
{
  Q_OBJECT

  private Q_SLOTS:
    void testThemes();
    void testSignals();
};

QTEST_KDEMAIN( ThemeManagerTest, NoGUI )

void ThemeManagerTest::testThemes()
{
  Grantlee::ThemeManager manager;
  QCOMPARE( manager.themes().count(), 0 );

  const QString themesDir( KDESRCDIR "/themes/" );

  manager.setThemesPath( themesDir );
  QCOMPARE( manager.themes().count(), 3 );

  const Grantlee::Theme::List themes = manager.themes();
  qDebug() << "count=" << themes.count();
  foreach ( const Grantlee::Theme &theme, themes ) {
    if ( theme.identifier() == QLatin1String( "air" ) ) {
      QCOMPARE( theme.name(), QLatin1String( "Air" ) );
      QCOMPARE( theme.description(), QLatin1String( "An Air theme" ) );
    } else if ( theme.identifier() == QLatin1String( "simple" ) ) {
      QCOMPARE( theme.name(), QLatin1String( "Simple" ) );
      QCOMPARE( theme.description(), QLatin1String( "A Simple theme" ) );
    } else if ( theme.identifier() == QLatin1String( "test" ) ) {
      QCOMPARE( theme.name(), QLatin1String( "Test" ) );
      QCOMPARE( theme.description(), QLatin1String( "A Test theme" ) );
    }
  }
}

void ThemeManagerTest::testSignals()
{
  Grantlee::ThemeManager manager;
  QCOMPARE( manager.themes().count(), 0 );

  const QString themesDir( KDESRCDIR "/themes/" );
  manager.setThemesPath( themesDir );

  //QEventLoop loop;
  //loop.exec();
}

#include "thememanagertest.moc"
