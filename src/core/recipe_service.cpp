#include "recipe_service.h"
#include "data/recipe_repository.h"
#include "data/ingredient_repository.h"
#include "utils/uuid_utils.h"
#include "utils/time_utils.h"
#include <QDebug>

namespace smart_diet {

RecipeService::RecipeService(RecipeRepository *r, RecipeIngredientRepository *ri,
                             IngredientRepository *ir, QObject *parent)
    : BaseService(parent), repo_(r), riRepo_(ri), ingRepo_(ir) { seedSystemRecipes(); }

QVariantMap RecipeService::toMap(const Recipe &r) const {
    QVariantMap m;
    m[QStringLiteral("id")] = r.id; m[QStringLiteral("name")] = r.name;
    m[QStringLiteral("description")] = r.description; m[QStringLiteral("cookingTime")] = r.cooking_time;
    m[QStringLiteral("servings")] = r.servings; m[QStringLiteral("mealType")] = r.meal_type;
    m[QStringLiteral("totalCalories")] = r.total_calories; m[QStringLiteral("totalProtein")] = r.total_protein;
    m[QStringLiteral("totalCarbs")] = r.total_carbs; m[QStringLiteral("totalFat")] = r.total_fat;
    return m;
}

QVariantList RecipeService::getAll() {
    QVariantList list;
    for (const auto &r : repo_->getAll()) list.append(toMap(r));
    return list;
}

QVariantList RecipeService::getByFamily(const QString &fid) {
    QVariantList list;
    for (const auto &r : repo_->getByFamily(fid)) list.append(toMap(r));
    return list;
}

QVariantMap RecipeService::getById(const QString &id) {
    auto opt = repo_->getById(id);
    return opt.has_value() ? toMap(opt.value()) : QVariantMap();
}

QVariantList RecipeService::getIngredients(const QString &rid) {
    QVariantList list;
    for (const auto &ri : riRepo_->getByRecipe(rid)) {
        auto ing = ingRepo_->getById(ri.ingredient_id);
        QVariantMap m;
        m[QStringLiteral("name")]   = ing.has_value() ? ing->name : QStringLiteral("?");
        m[QStringLiteral("amount")] = ri.amount;
        m[QStringLiteral("unit")]   = ing.has_value() ? ing->unit : QStringLiteral("g");
        list.append(m);
    }
    return list;
}

void RecipeService::seedSystemRecipes() {
    if (repo_->getAll().count() > 0) return;

    auto ing = [&](const QString &name) -> QString {
        auto list = ingRepo_->searchByName(name);
        return list.isEmpty() ? QString() : list.first().id;
    };

    const QString now = utcNow();

    struct { const char *n; const char *d; int t; const char *mt; struct { const char *in; double a; } items[5]; } recipes[] = {
        {"番茄炒蛋", "家常经典", 10, "any", {{"鸡蛋", 200}, {"西红柿", 300}, {"盐", 3}, {"食用油", 15}}},
        {"蛋炒饭",   "快手主食", 8,  "any", {{"大米", 200}, {"鸡蛋", 100}, {"食用油", 10}, {"盐", 2}}},
        {"清炒土豆丝","素菜",    8,  "any", {{"土豆", 300}, {"食用油", 10}, {"盐", 3}}},
    };

    for (const auto &rc : recipes) {
        Recipe r;
        r.id         = generateUuid();
        r.name       = QString::fromUtf8(rc.n);
        r.description= QString::fromUtf8(rc.d);
        r.cooking_time = rc.t;
        r.meal_type  = QString::fromUtf8(rc.mt);
        r.servings   = 2;
        r.is_system  = true;
        r.created_at = now; r.updated_at = now;
        repo_->save(r);

        double totalCal=0, totalPro=0, totalCarb=0, totalFat=0;
        for (int j = 0; j < 5 && rc.items[j].in; j++) {
            QString iid = ing(QString::fromUtf8(rc.items[j].in));
            if (iid.isEmpty()) continue;
            RecipeIngredient ri;
            ri.id = generateUuid(); ri.recipe_id = r.id;
            ri.ingredient_id = iid; ri.amount = rc.items[j].a; ri.created_at = now;
            riRepo_->addIngredient(ri);

            auto ingData = ingRepo_->getById(iid);
            if (ingData.has_value()) {
                double ratio = rc.items[j].a / 100.0;
                totalCal  += ingData->calories_per_100 * ratio;
                totalPro   += ingData->protein_per_100  * ratio;
                totalCarb += ingData->carbs_per_100    * ratio;
                totalFat  += ingData->fat_per_100      * ratio;
            }
        }
        r.total_calories = totalCal; r.total_protein = totalPro;
        r.total_carbs = totalCarb; r.total_fat = totalFat;
        r.updated_at = utcNow();
        repo_->update(r);
    }
    qDebug() << "[RecipeService] Seeded" << sizeof(recipes)/sizeof(recipes[0]) << "recipes";
}

} // namespace smart_diet
