#include "menu_service.h"
#include "data/menu_repository.h"
#include "data/recipe_repository.h"
#include "data/database_manager.h"
#include "utils/uuid_utils.h"
#include "utils/time_utils.h"
#include <QRandomGenerator>
#include <QDate>
#include <QSqlQuery>
#include <QDebug>

namespace smart_diet {

MenuService::MenuService(DailyMenuRepository *dm, MenuItemRepository *mi,
                         RecipeRepository *rr, QObject *parent)
    : BaseService(parent), dmRepo_(dm), miRepo_(mi), recipeRepo_(rr) {}

QString MenuService::findOrCreateMenu(const QString &familyId, const QString &date, const QString &mealType) {
    auto opt = dmRepo_->getByDate(familyId, date, mealType);
    if (opt.has_value()) return opt->id;

    DailyMenu m;
    m.id        = generateUuid();
    m.family_id = familyId;
    m.date      = date;
    m.meal_type = mealType;
    m.created_at = utcNow();
    m.updated_at = m.created_at;
    dmRepo_->save(m);
    return m.id;
}

QVariantMap MenuService::getTodayMenu(const QString &familyId, const QString &date) {
    QVariantMap result;
    QStringList meals = {QStringLiteral("breakfast"), QStringLiteral("lunch"), QStringLiteral("dinner")};
    for (const auto &mt : meals) {
        auto opt = dmRepo_->getByDate(familyId, date, mt);
        QVariantList items;
        if (opt.has_value()) {
            auto mItems = miRepo_->getByMenu(opt->id);
            for (const auto &mi : mItems) {
                auto recipe = recipeRepo_->getById(mi.recipe_id);
                if (recipe.has_value()) {
                    // 份数倍率：sv = 当前份数 / 默认份数
                    double sv = mi.servings_override > 0 ? mi.servings_override / 10.0 : 1.0;
                    double defSv = recipe->servings > 0 ? recipe->servings : 2.0;
                    double ratio = sv / defSv;

                    QVariantMap item;
                    item[QStringLiteral("itemId")]       = mi.id;
                    item[QStringLiteral("recipeId")]     = mi.recipe_id;
                    item[QStringLiteral("name")]         = recipe->name;
                    // 实际摄入 = 每份营养 × 当前份数
                    double perSv = defSv > 0 ? 1.0 / defSv : 0.5;
                    item[QStringLiteral("calPerServing")]    = recipe->total_calories * perSv * sv;
                    item[QStringLiteral("proPerServing")]    = recipe->total_protein  * perSv * sv;
                    item[QStringLiteral("carbPerServing")]   = recipe->total_carbs    * perSv * sv;
                    item[QStringLiteral("fatPerServing")]    = recipe->total_fat      * perSv * sv;
                    item[QStringLiteral("cookingTime")]   = recipe->cooking_time;
                    item[QStringLiteral("isLocked")]       = mi.is_locked;
                    item[QStringLiteral("status")]         = mi.status;
                    item[QStringLiteral("servingsOverride")] = mi.servings_override / 10.0;
                    items.append(item);
                }
            }
            QVariantMap slot;
            slot[QStringLiteral("menuId")]   = opt->id;
            slot[QStringLiteral("items")]    = items;
            slot[QStringLiteral("isConfirmed")] = opt->is_confirmed;
            result[mt] = slot;
        } else {
            QVariantMap slot;
            slot[QStringLiteral("menuId")] = QString();
            slot[QStringLiteral("items")]  = items;
            result[mt] = slot;
        }
    }
    return result;
}

QVariantMap MenuService::generateMenu(const QString &familyId, const QString &date,
                                       const QString &mealType, int count,
                                       const QString &dietMode, int dinerCount) {
    QVariantMap result;
    QString menuId = findOrCreateMenu(familyId, date, mealType);

    // 清除未锁定菜品，保留锁定的
    auto existing = miRepo_->getByMenu(menuId);
    QSet<QString> lockedIds;
    int lockedCount = 0;
    double lockedCal = 0, lockedPro = 0, lockedCarb = 0, lockedFat = 0;
    for (const auto &mi : existing) {
        if (mi.is_locked) {
            lockedIds.insert(mi.recipe_id);
            lockedCount++;
            auto r = recipeRepo_->getById(mi.recipe_id);
            if (r.has_value()) {
                double sv = mi.servings_override > 0 ? mi.servings_override : r->servings;
                double ratio = sv / (r->servings > 0 ? r->servings : 2.0);
                lockedCal  += r->total_calories * ratio;
                lockedPro  += r->total_protein  * ratio;
                lockedCarb += r->total_carbs    * ratio;
                lockedFat  += r->total_fat      * ratio;
            }
        }
    }
    miRepo_->clearUnlocked(menuId);

    int need = count - lockedCount;
    if (need < 0) need = 0;

    // 计算本餐营养目标
    // 全天比例：减脂/增肌 早25%午40%晚35%，维持/聚餐/放纵 早20%午45%晚35%
    double mealRatio = 0.35;
    if (mealType == QStringLiteral("breakfast")) mealRatio = 0.22;
    else if (mealType == QStringLiteral("lunch")) mealRatio = 0.42;

    double dailyTarget = 2000.0; // 默认，实际由用户 TDEE 决定——这里用调用方传入或取平均
    double mealCalTarget  = dailyTarget * mealRatio;
    double mealProTarget  = mealCalTarget * 0.30 / 4.0;
    double mealCarbTarget = mealCalTarget * 0.50 / 4.0;
    double mealFatTarget  = mealCalTarget * 0.20 / 9.0;

    // 扣除已锁定菜品
    double remainingCal  = mealCalTarget  - lockedCal;
    double remainingPro  = mealProTarget  - lockedPro;
    double remainingCarb = mealCarbTarget - lockedCarb;
    double remainingFat  = mealFatTarget  - lockedFat;
    if (remainingCal < 0) remainingCal = mealCalTarget * 0.3;

    // 加载候选菜谱（排除锁定 + 排除今日其他餐已选的）
    auto allRecipes = recipeRepo_->getAll();
    QSet<QString> todayUsedIds = lockedIds;
    // 收集今日所有菜单已选菜品
    QStringList allMeals = {QStringLiteral("breakfast"), QStringLiteral("lunch"), QStringLiteral("dinner")};
    for (const auto &mt : allMeals) {
        auto opt = dmRepo_->getByDate(familyId, date, mt);
        if (!opt.has_value() || opt->id == menuId) continue;
        auto items = miRepo_->getByMenu(opt->id);
        for (const auto &mi : items) todayUsedIds.insert(mi.recipe_id);
    }

    QList<Recipe> candidates;
    for (const auto &r : allRecipes) {
        if ((r.meal_type == mealType || r.meal_type == QStringLiteral("any"))
            && !todayUsedIds.contains(r.id))
            candidates.append(r);
    }

    // 贪心选取
    int added = 0;
    double curCal = 0, curPro = 0, curCarb = 0, curFat = 0;

    for (int i = 0; i < need && !candidates.isEmpty(); i++) {
        int bestIdx = -1;
        double bestScore = -1e9;
        double bestServings = 1.0;

        for (int j = 0; j < candidates.size(); j++) {
            const auto &rc = candidates[j];
            double defSv = rc.servings > 0 ? (double)rc.servings : 2.0;

            // 遍历可选份数：0.5, 1.0, 1.5, 2.0, 2.5, 3.0
            double maxSv = dinerCount * 2.5;
            if (maxSv > 10.0) maxSv = 10.0;
            if (maxSv < 2.5)  maxSv = 2.5;
            for (double sv = 0.5; sv <= maxSv; sv += 0.5) {
                double ratio = sv / defSv;
                double addCal  = rc.total_calories * ratio;
                double addPro  = rc.total_protein  * ratio;
                double addCarb = rc.total_carbs    * ratio;
                double addFat  = rc.total_fat      * ratio;

                double newCal = curCal + addCal;
                double newPro = curPro + addPro;
                double newCarb = curCarb + addCarb;
                double newFat = curFat + addFat;

                // 热量偏差评分（接近目标为优）
                double calError = (remainingCal > 0) ? (newCal - remainingCal) / remainingCal * 100 : 100;
                if (calError < 0) calError = -calError;
                double calScore = 100.0 - calError;
                if (calScore < 0) calScore = 0;

                // 用严格度：减脂/增肌 ±10%，其他 ±30%
                bool strict = (dietMode == QStringLiteral("weight_loss") || dietMode == QStringLiteral("muscle_gain"));
                double tolerance = strict ? 10.0 : 30.0;
                if (calError > tolerance) calScore *= 0.3;

                // 营养比例评分
                double newTotalPro  = curPro  + addPro  + lockedPro;
                double newTotalCarb = curCarb + addCarb + lockedCarb;
                double newTotalFat  = curFat  + addFat  + lockedFat;
                double newTotalMacro = newTotalPro + newTotalCarb + newTotalFat;
                if (newTotalMacro > 0) {
                    double pRatio = newTotalPro  / newTotalMacro;
                    double cRatio = newTotalCarb / newTotalMacro;
                    double fRatio = newTotalFat  / newTotalMacro;
                    double pErr = qAbs(pRatio - 0.30) * 100;
                    double cErr = qAbs(cRatio - 0.50) * 100;
                    double fErr = qAbs(fRatio - 0.20) * 100;
                    double macroPenalty = pErr + cErr + fErr;
                    calScore -= macroPenalty * 0.5;
                }

                // 份数惩罚：超过理想倍率开始扣分
                if (sv > maxSv * 0.5) calScore -= (sv - maxSv * 0.5) * 15;

                if (calScore > bestScore) {
                    bestScore = calScore;
                    bestIdx = j;
                    bestServings = sv;
                }
            }
        }

        if (bestIdx < 0) break;

        const auto &best = candidates[bestIdx];
        double ratio = bestServings / (best.servings > 0 ? (double)best.servings : 2.0);

        MenuItem mi;
        mi.id         = generateUuid();
        mi.menu_id    = menuId;
        mi.recipe_id  = best.id;
        mi.servings_override = (int)(bestServings * 10); // 存为 10 倍整数 (1.5→15)
        mi.sort_order = lockedCount + i;
        mi.created_at = utcNow();
        mi.updated_at = mi.created_at;
        if (miRepo_->addItem(mi)) {
            curCal  += best.total_calories * ratio;
            curPro  += best.total_protein  * ratio;
            curCarb += best.total_carbs    * ratio;
            curFat  += best.total_fat      * ratio;
            candidates.removeAt(bestIdx);
            added++;
        }
    }

    result[QStringLiteral("menuId")] = menuId;
    result[QStringLiteral("added")]  = added;
    emit menuGenerated(familyId, date);
    emit operationCompleted(QStringLiteral("菜单已生成"));
    return result;
}

bool MenuService::addDish(const QString &menuId, const QString &recipeId) {
    auto existing = miRepo_->getByMenu(menuId);
    MenuItem mi;
    mi.id        = generateUuid();
    mi.menu_id   = menuId;
    mi.recipe_id = recipeId;
    mi.sort_order = existing.size();
    mi.created_at = utcNow();
    mi.updated_at = mi.created_at;
    return miRepo_->addItem(mi);
}

bool MenuService::removeDish(const QString &menuId, const QString &recipeId) {
    return miRepo_->removeItem(menuId, recipeId);
}

bool MenuService::swapDish(const QString &menuId, const QString &oldRecipeId) {
    // 找到被锁定的菜以外的候选
    auto all = recipeRepo_->getAll();
    auto existing = miRepo_->getByMenu(menuId);
    QSet<QString> existingIds;
    for (const auto &mi : existing) existingIds.insert(mi.recipe_id);

    QList<Recipe> candidates;
    for (const auto &r : all) {
        if (!existingIds.contains(r.id)) candidates.append(r);
    }
    if (candidates.isEmpty()) return false;

    // 移除旧菜，加新菜
    miRepo_->removeItem(menuId, oldRecipeId);
    int idx = QRandomGenerator::global()->bounded(candidates.size());
    return addDish(menuId, candidates[idx].id);
}

bool MenuService::adjustServings(const QString &menuId, const QString &recipeId, double servings) {
    if (servings < 0.5) servings = 0.5;
    QSqlQuery q(miRepo_->database());
    q.prepare(QStringLiteral("UPDATE menu_items SET servings_override=:sv, updated_at=:ua WHERE menu_id=:mid AND recipe_id=:rid"));
    q.bindValue(QStringLiteral(":sv"), (int)(servings * 10));
    q.bindValue(QStringLiteral(":ua"), utcNow());
    q.bindValue(QStringLiteral(":mid"), menuId);
    q.bindValue(QStringLiteral(":rid"), recipeId);
    return q.exec();
}

bool MenuService::toggleLock(const QString &menuId, const QString &recipeId) {
    auto items = miRepo_->getByMenu(menuId);
    for (const auto &mi : items) {
        if (mi.recipe_id == recipeId) {
            QSqlQuery q(miRepo_->database());
            q.prepare(QStringLiteral("UPDATE menu_items SET is_locked=:l,updated_at=:ua WHERE menu_id=:mid AND recipe_id=:rid"));
            q.bindValue(QStringLiteral(":l"), mi.is_locked ? 0 : 1);
            q.bindValue(QStringLiteral(":ua"), utcNow());
            q.bindValue(QStringLiteral(":mid"), menuId);
            q.bindValue(QStringLiteral(":rid"), recipeId);
            return q.exec();
        }
    }
    return false;
}

QVariantMap MenuService::getNutritionSummary(const QString &familyId, const QString &date,
                                                const QString &dietMode, int dinerCount,
                                                double userTdee) {
    QVariantMap result;

    // 每日推荐摄入（基于用户实际 TDEE，按饮食模式修正）
    double baseTdee = userTdee * dinerCount;
    double tdeeMultiplier = 1.0;
    if (dietMode == QStringLiteral("weight_loss"))  tdeeMultiplier = 0.80;
    else if (dietMode == QStringLiteral("muscle_gain")) tdeeMultiplier = 1.15;
    else if (dietMode == QStringLiteral("gathering"))   tdeeMultiplier = 1.30;
    else if (dietMode == QStringLiteral("cheat_day"))   tdeeMultiplier = 1.20;
    double targetCal = baseTdee * tdeeMultiplier;

    // 默认宏量营养素配比：碳水50% 蛋白30% 脂肪20%
    double targetPro  = targetCal * 0.30 / 4.0;   // 蛋白 4 kcal/g
    double targetCarb = targetCal * 0.50 / 4.0;   // 碳水 4 kcal/g
    double targetFat  = targetCal * 0.20 / 9.0;   // 脂肪 9 kcal/g

    // 计算当前实际摄入
    double curCal = 0, curPro = 0, curCarb = 0, curFat = 0;
    QStringList meals = {QStringLiteral("breakfast"), QStringLiteral("lunch"), QStringLiteral("dinner")};
    for (const auto &mt : meals) {
        auto opt = dmRepo_->getByDate(familyId, date, mt);
        if (!opt.has_value()) continue;
        auto items = miRepo_->getByMenu(opt->id);
        for (const auto &mi : items) {
            auto recipe = recipeRepo_->getById(mi.recipe_id);
            if (!recipe.has_value()) continue;
            // 每道菜贡献 = 整份营养值 × 份数倍率
            double sv = mi.servings_override > 0 ? mi.servings_override / 10.0 : 1.0;
            double defSv = recipe->servings > 0 ? recipe->servings : 2.0;
            double ratio = sv / defSv;
            curCal  += recipe->total_calories * ratio;
            curPro  += recipe->total_protein  * ratio;
            curCarb += recipe->total_carbs    * ratio;
            curFat  += recipe->total_fat      * ratio;
        }
    }

    result[QStringLiteral("targetCal")]  = targetCal;
    result[QStringLiteral("targetPro")]  = targetPro;
    result[QStringLiteral("targetCarb")] = targetCarb;
    result[QStringLiteral("targetFat")]  = targetFat;
    result[QStringLiteral("currentCal")] = curCal;
    result[QStringLiteral("currentPro")] = curPro;
    result[QStringLiteral("currentCarb")]= curCarb;
    result[QStringLiteral("currentFat")] = curFat;

    return result;
}

} // namespace smart_diet
