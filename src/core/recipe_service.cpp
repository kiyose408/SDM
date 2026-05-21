#include "recipe_service.h"
#include "data/recipe_repository.h"
#include "data/ingredient_repository.h"
#include "data/recipe_favorite_repository.h"
#include "data/database_manager.h"
#include <QSqlQuery>
#include "utils/uuid_utils.h"
#include "utils/time_utils.h"
#include <QDebug>

namespace smart_diet {

RecipeService::RecipeService(RecipeRepository *r, RecipeIngredientRepository *ri,
                             IngredientRepository *ir, RecipeFavoriteRepository *fr,
                             QObject *parent)
    : BaseService(parent), repo_(r), riRepo_(ri), ingRepo_(ir), favRepo_(fr) { seedSystemRecipes(); }

QVariantMap RecipeService::toMap(const Recipe &r) const {
    // 计算总重量用于每 100g 归一化
    double totalWeight = 0;
    for (const auto &ri : riRepo_->getByRecipe(r.id)) totalWeight += ri.amount;

    QVariantMap m;
    m[QStringLiteral("id")] = r.id; m[QStringLiteral("name")] = r.name;
    m[QStringLiteral("description")] = r.description; m[QStringLiteral("cookingTime")] = r.cooking_time;
    m[QStringLiteral("servings")] = r.servings; m[QStringLiteral("mealType")] = r.meal_type;
    m[QStringLiteral("totalCalories")] = r.total_calories; m[QStringLiteral("totalProtein")] = r.total_protein;
    m[QStringLiteral("totalCarbs")] = r.total_carbs; m[QStringLiteral("totalFat")] = r.total_fat;
    // 每 100g 归一化值
    m[QStringLiteral("calPer100")]  = totalWeight > 0 ? r.total_calories / totalWeight * 100 : 0;
    m[QStringLiteral("proPer100")]  = totalWeight > 0 ? r.total_protein  / totalWeight * 100 : 0;
    m[QStringLiteral("carbPer100")] = totalWeight > 0 ? r.total_carbs    / totalWeight * 100 : 0;
    m[QStringLiteral("fatPer100")]  = totalWeight > 0 ? r.total_fat      / totalWeight * 100 : 0;

    // 标签 ID 列表
    QVariantList tagIds;
    // 简单内联查询（TagRepository 已有方法）
    QSqlQuery tq(DatabaseManager::instance().database());
    tq.prepare(QStringLiteral("SELECT tag_id FROM recipe_tag_relations WHERE recipe_id=:rid"));
    tq.bindValue(QStringLiteral(":rid"), r.id);
    if (tq.exec()) while (tq.next()) tagIds.append(tq.value(0).toString());
    m[QStringLiteral("tags")] = tagIds;

    return m;
}

QVariantList RecipeService::getAll() {
    QVariantList list;
    for (const auto &r : repo_->getAll()) list.append(toMap(r));
    return list;
}

QVariantList RecipeService::searchByName(const QString &keyword) {
    QVariantList list;
    for (const auto &r : repo_->searchByName(keyword)) list.append(toMap(r));
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
    // 仅在菜品表为空时播种（首次运行）
    if (repo_->getAll().count() > 0) return;

    auto ing = [&](const QString &name) -> QString {
        auto list = ingRepo_->searchByName(name);
        return list.isEmpty() ? QString() : list.first().id;
    };

    const QString now = utcNow();

    struct { const char *n; const char *d; int t; const char *mt; const char *tags[4]; struct { const char *in; double a; } items[5]; } recipes[] = {
        {"番茄炒蛋", "家常经典", 10, "any", {"家常","快手菜","煎炒","高蛋白"}, {{"鸡蛋", 200}, {"西红柿", 300}, {"盐", 3}, {"食用油", 15}}},
        {"蛋炒饭",   "快手主食", 8,  "any", {"快手菜","煎炒","家常"}, {{"大米", 200}, {"鸡蛋", 100}, {"食用油", 10}, {"盐", 2}}},
        {"清炒土豆丝","素菜",    8,  "any", {"清淡","家常","低脂"}, {{"土豆", 300}, {"食用油", 10}, {"盐", 3}}},
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

        // 为种子菜谱分配标签
        for (int t = 0; t < 4 && rc.tags[t]; t++) {
            QString tagName = QString::fromUtf8(rc.tags[t]);
            QSqlQuery findT(DatabaseManager::instance().database());
            findT.prepare(QStringLiteral("SELECT id FROM tags WHERE name=:n"));
            findT.bindValue(QStringLiteral(":n"), tagName);
            if (findT.exec() && findT.next()) {
                QSqlQuery ins(DatabaseManager::instance().database());
                ins.prepare(QStringLiteral("INSERT OR IGNORE INTO recipe_tag_relations (id,recipe_id,tag_id,created_at) VALUES (:id,:rid,:tid,:ca)"));
                ins.bindValue(QStringLiteral(":id"), generateUuid());
                ins.bindValue(QStringLiteral(":rid"), r.id);
                ins.bindValue(QStringLiteral(":tid"), findT.value(0).toString());
                ins.bindValue(QStringLiteral(":ca"), now);
                ins.exec();
            }
        }
    }
    qDebug() << "[RecipeService] Seeded" << sizeof(recipes)/sizeof(recipes[0]) << "recipes";
}

QVariantMap RecipeService::createRecipe(const QVariantMap &data, const QVariantList &ingredients,
                                         const QString &creatorId, const QString &familyId) {
    QVariantMap result;
    const QString now = utcNow();

    Recipe r;
    r.id           = generateUuid();
    r.name         = data.value(QStringLiteral("name")).toString();
    r.description  = data.value(QStringLiteral("description")).toString();
    r.cooking_time = data.value(QStringLiteral("cookingTime"), 0).toInt();
    r.servings     = data.value(QStringLiteral("servings"), 2).toInt();
    r.meal_type    = data.value(QStringLiteral("mealType"), QStringLiteral("any")).toString();
    r.is_system    = false;
    r.creator_id   = creatorId;
    r.family_id    = familyId;
    r.created_at   = now;
    r.updated_at   = now;

    if (!repo_->save(r)) {
        result[QStringLiteral("success")] = false;
        result[QStringLiteral("error")]   = QStringLiteral("保存菜谱失败");
        return result;
    }

    double totalCal=0, totalPro=0, totalCarb=0, totalFat=0;
    for (const auto &v : ingredients) {
        auto ingMap = v.toMap();
        RecipeIngredient ri;
        ri.id            = generateUuid();
        ri.recipe_id     = r.id;
        ri.ingredient_id = ingMap.value(QStringLiteral("ingredientId")).toString();
        ri.amount        = ingMap.value(QStringLiteral("amount"), 0).toDouble();
        ri.created_at    = now;
        riRepo_->addIngredient(ri);

        auto ingData = ingRepo_->getById(ri.ingredient_id);
        if (ingData.has_value()) {
            double ratio = ri.amount / 100.0;
            totalCal  += ingData->calories_per_100 * ratio;
            totalPro  += ingData->protein_per_100  * ratio;
            totalCarb += ingData->carbs_per_100    * ratio;
            totalFat  += ingData->fat_per_100      * ratio;
        }
    }

    r.total_calories = totalCal; r.total_protein = totalPro;
    r.total_carbs = totalCarb; r.total_fat = totalFat;
    r.updated_at = utcNow();
    repo_->update(r);

    result[QStringLiteral("success")]  = true;
    result[QStringLiteral("recipeId")] = r.id;
    return result;
}

bool RecipeService::updateRecipe(const QString &recipeId, const QVariantMap &data,
                                  const QVariantList &ingredients) {
    auto opt = repo_->getById(recipeId);
    if (!opt.has_value()) return false;

    Recipe r = opt.value();
    r.name         = data.value(QStringLiteral("name"), r.name).toString();
    r.description  = data.value(QStringLiteral("description"), r.description).toString();
    r.cooking_time = data.value(QStringLiteral("cookingTime"), r.cooking_time).toInt();
    r.servings     = data.value(QStringLiteral("servings"), r.servings).toInt();
    r.meal_type    = data.value(QStringLiteral("mealType"), r.meal_type).toString();
    r.updated_at   = utcNow();

    riRepo_->removeByRecipe(recipeId);

    double totalCal=0, totalPro=0, totalCarb=0, totalFat=0;
    const QString now = utcNow();
    for (const auto &v : ingredients) {
        auto ingMap = v.toMap();
        RecipeIngredient ri;
        ri.id = generateUuid(); ri.recipe_id = r.id;
        ri.ingredient_id = ingMap.value(QStringLiteral("ingredientId")).toString();
        ri.amount = ingMap.value(QStringLiteral("amount"), 0).toDouble(); ri.created_at = now;
        riRepo_->addIngredient(ri);

        auto ingData = ingRepo_->getById(ri.ingredient_id);
        if (ingData.has_value()) {
            double ratio = ri.amount / 100.0;
            totalCal  += ingData->calories_per_100 * ratio;
            totalPro  += ingData->protein_per_100  * ratio;
            totalCarb += ingData->carbs_per_100    * ratio;
            totalFat  += ingData->fat_per_100      * ratio;
        }
    }

    r.total_calories = totalCal; r.total_protein = totalPro;
    r.total_carbs = totalCarb; r.total_fat = totalFat;
    r.updated_at = utcNow();
    return repo_->update(r);
}

bool RecipeService::deleteRecipe(const QString &recipeId) {
    return repo_->softDelete(recipeId);
}

bool RecipeService::toggleFavorite(const QString &userId, const QString &recipeId) {
    if (favRepo_->isFavorited(userId, recipeId))
        return favRepo_->removeFavorite(userId, recipeId);
    return favRepo_->addFavorite(userId, recipeId);
}

bool RecipeService::isFavorited(const QString &userId, const QString &recipeId) {
    return favRepo_->isFavorited(userId, recipeId);
}

QVariantList RecipeService::getFavorites(const QString &userId) {
    QVariantList list;
    for (const auto &rid : favRepo_->getFavoriteRecipeIds(userId)) {
        auto opt = repo_->getById(rid);
        if (opt.has_value()) list.append(toMap(opt.value()));
    }
    return list;
}

} // namespace smart_diet
