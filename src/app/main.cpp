#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QLocale>
#include <QTranslator>

#include "core/registration_validator.h"
#include "core/tdee_calculator.h"
#include "core/auth_service.h"
#include "data/database_manager.h"
#include "data/user_repository.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // ================================================================
    // 初始化数据库（必须先于任何 Repository）
    // ================================================================
    smart_diet::DatabaseManager::instance().initialize();

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

    // ================================================================
    // 注册 C++ 类型到 QML（无状态工具类）
    // ================================================================
    qmlRegisterType<smart_diet::RegistrationValidator>(
        "SmartDiet.Core", 1, 0, "RegistrationValidator");
    qmlRegisterType<smart_diet::TdeeCalculator>(
        "SmartDiet.Core", 1, 0, "TdeeCalculator");

    // 注册全局主题单例
    qmlRegisterSingletonType(QUrl(QStringLiteral("qrc:/qml/styles/Theme.qml")),
                              "SmartDiet.Style", 1, 0, "Theme");

    // ================================================================
    // 创建服务层（依赖注入）
    // ================================================================
    auto *userRepo = new smart_diet::UserRepository(&app);
    auto *authService = new smart_diet::AuthService(userRepo, &app);
    engine.rootContext()->setContextProperty(QStringLiteral("authService"), authService);

    // TODO: 阶段 2 完成后根据登录状态选择加载窗口
    engine.load(QUrl(QStringLiteral("qrc:/qml/RegisterWindow.qml")));
    // engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
