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
#include "data/family_repository.h"
#include "data/family_member_repository.h"
#include "data/ingredient_repository.h"
#include "data/recipe_repository.h"
#include "data/recipe_favorite_repository.h"
#include "data/tag_repository.h"
#include "data/menu_repository.h"
#include "core/menu_service.h"
#include "core/purchase_service.h"
#include "core/fridge_service.h"
#include "data/inventory_repository.h"
#include "data/database_manager.h"

#include "core/family_service.h"
#include "core/ingredient_service.h"
#include "core/recipe_service.h"
#include "core/tag_service.h"

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

    auto *familyRepo       = new smart_diet::FamilyRepository(&app);
    auto *familyMemberRepo = new smart_diet::FamilyMemberRepository(&app);
    auto *familyService    = new smart_diet::FamilyService(familyRepo, familyMemberRepo, &app);
    auto *ingredientRepo   = new smart_diet::IngredientRepository(&app);
    auto *ingredientService = new smart_diet::IngredientService(ingredientRepo, &app);
    auto *recipeRepo       = new smart_diet::RecipeRepository(&app);
    auto *recipeIngRepo    = new smart_diet::RecipeIngredientRepository(&app);
    auto *recipeFavRepo    = new smart_diet::RecipeFavoriteRepository(&app);
    auto *tagRepo          = new smart_diet::TagRepository(&app);
    auto *tagService       = new smart_diet::TagService(tagRepo, &app);
    auto *recipeService    = new smart_diet::RecipeService(recipeRepo, recipeIngRepo, ingredientRepo, recipeFavRepo, &app);
    auto *dailyMenuRepo    = new smart_diet::DailyMenuRepository(&app);
    auto *menuItemRepo     = new smart_diet::MenuItemRepository(&app);
    auto *menuService      = new smart_diet::MenuService(dailyMenuRepo, menuItemRepo, recipeRepo, &app);
    auto *purchaseService   = new smart_diet::PurchaseService(dailyMenuRepo, menuItemRepo, recipeRepo, recipeIngRepo, ingredientRepo, &app);
    auto *invBatchRepo        = new smart_diet::InventoryBatchRepository(smart_diet::DatabaseManager::instance().database(), &app);
    auto *fridgeService       = new smart_diet::FridgeService(invBatchRepo, ingredientRepo, &app);


    QObject::connect(authService, &smart_diet::AuthService::userLoggedIn,
                     session, [session](const QString &userId) {
                         session->setSession(userId, QString());
                     });

    engine.rootContext()->setContextProperty(QStringLiteral("authService"), authService);
    engine.rootContext()->setContextProperty(QStringLiteral("session"), session);
    engine.rootContext()->setContextProperty(QStringLiteral("familyService"), familyService);
    engine.rootContext()->setContextProperty(QStringLiteral("ingredientService"), ingredientService);
    engine.rootContext()->setContextProperty(QStringLiteral("recipeService"), recipeService);
    engine.rootContext()->setContextProperty(QStringLiteral("tagService"), tagService);
    engine.rootContext()->setContextProperty(QStringLiteral("menuService"), menuService);
    engine.rootContext()->setContextProperty(QStringLiteral("purchaseService"), purchaseService);
    engine.rootContext()->setContextProperty(QStringLiteral("fridgeService"), fridgeService);


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
