#include <QApplication>
#include <QQmlApplicationEngine>
#include <QLocale>
#include <QTranslator>
#include "core/registration_validator.h"

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

    // 注册 C++ 类型到 QML
    qmlRegisterType<smart_diet::RegistrationValidator>(
        "SmartDiet.Core", 1, 0, "RegistrationValidator");

    // 注册全局主题单例：QML 端通过 import SmartDiet.Style 1.0 访问 Theme
    qmlRegisterSingletonType(QUrl(QStringLiteral("qrc:/qml/styles/Theme.qml")),
                              "SmartDiet.Style", 1, 0, "Theme");

    // TODO: 阶段 2 完成后根据登录状态选择加载窗口
    engine.load(QUrl(QStringLiteral("qrc:/qml/RegisterWindow.qml")));
    // engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
