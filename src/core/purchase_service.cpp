#include "purchase_service.h"
#include "data/menu_repository.h"
#include "data/recipe_repository.h"
#include "data/recipe_repository.h"
#include "data/ingredient_repository.h"
#include "data/database_manager.h"
#include <QSqlQuery>
#include <QMap>
#include <QDebug>

namespace smart_diet {

struct PurchaseItem {
    QString ingredientName;
    QString category;
    double  amount = 0;
};

PurchaseService::PurchaseService(DailyMenuRepository *dm, MenuItemRepository *mi,
                                 RecipeRepository *rr, RecipeIngredientRepository *ri,
                                 IngredientRepository *ir, QObject *parent)
    : BaseService(parent), dmRepo_(dm), miRepo_(mi), recipeRepo_(rr), riRepo_(ri), ingRepo_(ir) {}

QVariantList PurchaseService::getTodayPurchaseList(const QString &familyId, const QString &date) {
    QMap<QString, PurchaseItem> aggregated;

    QStringList meals = {QStringLiteral("breakfast"), QStringLiteral("lunch"), QStringLiteral("dinner")};
    for (const auto &mt : meals) {
        auto menuOpt = dmRepo_->getByDate(familyId, date, mt);
        if (!menuOpt.has_value()) continue;

        auto items = miRepo_->getByMenu(menuOpt->id);
        for (const auto &mi : items) {
            auto recipe = recipeRepo_->getById(mi.recipe_id);
            if (!recipe.has_value()) continue;

            // 份数倍率
            double sv = mi.servings_override > 0 ? mi.servings_override / 10.0 : 1.0;
            double defSv = recipe->servings > 0 ? recipe->servings : 2.0;
            double ratio = sv / (double)defSv;

            // 获取菜谱食材，按份数倍率换算采购量
            auto riList = riRepo_->getByRecipe(mi.recipe_id);
            for (const auto &ri : riList) {
                auto ing = ingRepo_->getById(ri.ingredient_id);
                if (!ing.has_value()) continue;

                // 采购量 = 食材量 × 份数倍率（sv / defSv）
                double adjusted = ri.amount * ratio;
                if (adjusted < 0.1) continue;

                if (aggregated.contains(ing->name)) {
                    aggregated[ing->name].amount += adjusted;
                } else {
                    PurchaseItem pi;
                    pi.ingredientName = ing->name;
                    pi.category       = ing->category;
                    pi.amount         = adjusted;
                    aggregated[ing->name] = pi;
                }
            }
        }
    }

    // 按类别排序输出
    QVariantList result;
    // 类别顺序：meat > seafood > vegetable > staple > dairy > condiment > other
    QStringList catOrder = {QStringLiteral("meat"), QStringLiteral("seafood"),
                             QStringLiteral("vegetable"), QStringLiteral("staple"),
                             QStringLiteral("dairy"), QStringLiteral("condiment"), QStringLiteral("other")};

    for (const auto &cat : catOrder) {
        for (auto it = aggregated.begin(); it != aggregated.end(); ++it) {
            if (it->category == cat) {
                QVariantMap m;
                m[QStringLiteral("name")]  = it->ingredientName;
                m[QStringLiteral("cat")]   = it->category;
                m[QStringLiteral("amount")] = QString::number(it->amount, 'f', 1) + QStringLiteral("g");
                result.append(m);
            }
        }
    }
    return result;
}

} // namespace smart_diet
