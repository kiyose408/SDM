#include <QApplication>
#include <QQmlApplicationEngine>
#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "SmartDietManager_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            app.installTranslator(&translator);
            break;
        }
    }

    QQmlApplicationEngine engine;

    // 注册全局主题单例：QML 端通过 import SmartDiet.Style 1.0 访问 Theme
    qmlRegisterSingletonType(QUrl(QStringLiteral("qrc:/qml/styles/Theme.qml")),
                              "SmartDiet.Style", 1, 0, "Theme");

    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
