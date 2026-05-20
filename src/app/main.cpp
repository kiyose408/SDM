#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QLocale>
#include <QTranslator>
#include <QWindow>

#include "core/registration_validator.h"
#include "core/tdee_calculator.h"
#include "core/auth_service.h"
#include "core/session_manager.h"
#include "data/database_manager.h"
#include "data/user_repository.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

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

    qmlRegisterType<smart_diet::RegistrationValidator>(
        "SmartDiet.Core", 1, 0, "RegistrationValidator");
    qmlRegisterType<smart_diet::TdeeCalculator>(
        "SmartDiet.Core", 1, 0, "TdeeCalculator");
    qmlRegisterSingletonType(QUrl(QStringLiteral("qrc:/qml/styles/Theme.qml")),
                              "SmartDiet.Style", 1, 0, "Theme");

    auto *userRepo    = new smart_diet::UserRepository(&app);
    auto *authService = new smart_diet::AuthService(userRepo, &app);
    auto *session     = new smart_diet::SessionManager(&app);

    QObject::connect(authService, &smart_diet::AuthService::userLoggedIn,
                     session, [session](const QString &userId) {
                         session->setSession(userId, QString());
                     });

    engine.rootContext()->setContextProperty(QStringLiteral("authService"), authService);
    engine.rootContext()->setContextProperty(QStringLiteral("session"), session);

    // ================================================================
    // 登录成功后 → 关闭认证窗口 → 加载主界面
    // ================================================================
    QObject::connect(session, &smart_diet::SessionManager::loggedIn,
                     &engine, [&engine, &app]() {
                         // 关闭当前窗口
                         const auto roots = engine.rootObjects();
                         for (auto *obj : roots) {
                             if (auto *win = qobject_cast<QWindow *>(obj))
                                 win->close();
                         }
                         // 清缓存，加载主界面
                         engine.clearComponentCache();
                         engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));
                     });

    // ================================================================
    // 根据登录状态选择初始窗口
    // ================================================================
    if (session->restoreSession()) {
        engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));
    } else {
        engine.load(QUrl(QStringLiteral("qrc:/qml/LoginWindow.qml")));
    }

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
