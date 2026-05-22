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
    // 每 100g 归一化值（复用上方 totalWeight）
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

    struct { const char *n; const char *d; int t; const char *mt; const char *tags[5]; struct { const char *in; double a; } items[8]; } recipes[] = {
        // 家常菜
        {"番茄炒蛋","家常经典",10,"any",{"家常菜","咸鲜","高蛋白"},{{"鸡蛋",200},{"西红柿",300},{"盐",3},{"食用油",15}}},
        {"蛋炒饭","快手主食",8,"any",{"家常菜","主食","咸鲜"},{{"大米",200},{"鸡蛋",100},{"食用油",10},{"盐",2},{"大葱",10}}},
        {"青椒炒肉","下饭神器",10,"lunch",{"家常菜","咸鲜","辣"},{{"猪瘦肉",200},{"青椒",150},{"食用油",10},{"酱油",10},{"蒜",5},{"盐",3}}},
        {"红烧肉","慢炖经典",60,"lunch",{"家常菜","宴客节日","咸鲜","热量炸弹"},{{"猪五花肉",500},{"酱油",20},{"糖",15},{"姜",10},{"料酒",10},{"八角",3}}},
        {"地三鲜","东北经典",15,"lunch",{"家常菜","咸鲜","轻食"},{{"土豆",200},{"茄子",200},{"青椒",100},{"食用油",20},{"蒜",10},{"酱油",10}}},
        {"麻婆豆腐","川味名菜",15,"lunch",{"家常菜","辣","麻香","高蛋白"},{{"豆腐",400},{"猪瘦肉",100},{"豆瓣酱",20},{"花椒",5},{"生抽",10},{"蒜",5}}},
        {"宫保鸡丁","经典川菜",20,"lunch",{"家常菜","辣","咸鲜","宴客节日"},{{"鸡胸肉",250},{"花生",50},{"辣椒",10},{"酱油",15},{"醋",10},{"糖",10}}},
        {"回锅肉","川味经典",20,"lunch",{"家常菜","辣","咸鲜","热量炸弹"},{{"猪五花肉",300},{"蒜",20},{"豆瓣酱",15},{"青椒",100},{"酱油",10}}},
        {"蒜蓉西兰花","健康素菜",8,"any",{"家常菜","轻食","减脂"},{{"西兰花",300},{"蒜",15},{"食用油",10},{"盐",3}}},
        {"红烧排骨","宴客硬菜",50,"dinner",{"家常菜","宴客节日","咸鲜","高蛋白"},{{"猪排骨",500},{"酱油",20},{"糖",15},{"姜",10},{"料酒",10},{"八角",3}}},
        {"干煸四季豆","下饭素菜",15,"lunch",{"家常菜","辣","咸鲜"},{{"豆角",300},{"猪瘦肉",80},{"辣椒",10},{"花椒",5},{"酱油",10},{"蒜",5}}},
        {"醋溜白菜","快手素菜",8,"any",{"家常菜","酸","减脂"},{{"白菜",400},{"醋",20},{"食用油",10},{"盐",3},{"蒜",5}}},
        // 汤羹甜品
        {"西红柿蛋汤","快手汤品",10,"any",{"汤羹甜品","轻食"},{{"西红柿",200},{"鸡蛋",100},{"盐",3},{"食用油",5},{"香菜",5}}},
        {"酸辣汤","开胃暖身",20,"any",{"汤羹甜品","酸","辣","轻食"},{{"豆腐",200},{"鸡蛋",100},{"木耳",20},{"醋",25},{"辣椒粉",5},{"生抽",10},{"淀粉",15}}},
        {"冬瓜排骨汤","清淡滋补",60,"dinner",{"汤羹甜品","咸鲜","轻食"},{{"猪排骨",300},{"冬瓜",500},{"姜",10},{"盐",5},{"料酒",10}}},
        {"南瓜粥","暖胃早餐",30,"breakfast",{"汤羹甜品","轻食","甜"},{{"大米",100},{"南瓜",300},{"糖",10}}},
        // 主食
        {"番茄鸡蛋面","一餐搞定",15,"breakfast",{"主食","咸鲜","高蛋白"},{{"面条",200},{"鸡蛋",100},{"西红柿",200},{"食用油",10},{"盐",3}}},
        {"葱油拌面","上海经典",10,"any",{"主食","咸鲜"},{{"面条",200},{"食用油",30},{"大葱",50},{"生抽",15},{"糖",5}}},
        {"红薯饭","粗粮主食",0,"any",{"主食","减脂","轻食"},{{"大米",150},{"红薯",150}}},
        // 小吃
        {"炸薯条","追剧必备",15,"any",{"小吃","热量炸弹","空气炸锅"},{{"土豆",400},{"食用油",30},{"盐",3}}},
        {"烤鸡翅","聚会必备",30,"any",{"小吃","高蛋白","烤箱","宴客节日"},{{"鸡翅",500},{"酱油",20},{"糖",10},{"姜",10},{"料酒",10}}},
        // 凉菜沙拉
        {"凉拌黄瓜","清脆开胃",5,"any",{"凉菜沙拉","免开火","酸","减脂"},{{"黄瓜",300},{"蒜",15},{"醋",20},{"生抽",10},{"糖",5},{"辣椒粉",3}}},
        {"凉拌木耳","营养小菜",5,"any",{"凉菜沙拉","免开火","酸","减脂"},{{"木耳",100},{"醋",15},{"生抽",10},{"蒜",10},{"香油",5},{"香菜",10}}},
        {"皮蛋豆腐","经典凉菜",5,"any",{"凉菜沙拉","免开火","高蛋白"},{{"豆腐",300},{"生抽",15},{"香油",5},{"蒜",5}}},
        // 烤箱/空气炸锅
        {"烤三文鱼","健康西餐",20,"dinner",{"烤箱","高蛋白","优质脂肪","减脂"},{{"三文鱼",200},{"橄榄油",10},{"盐",3},{"黑胡椒",2},{"柠檬",20}}},
        {"烤蔬菜拼盘","低卡饱腹",25,"dinner",{"烤箱","轻食","减脂","增肌"},{{"西兰花",150},{"胡萝卜",100},{"南瓜",150},{"食用油",15},{"盐",5},{"黑胡椒",3}}},
        {"空气炸鸡胸","健身神器",18,"any",{"空气炸锅","增肌","高蛋白","减脂"},{{"鸡胸肉",300},{"生抽",10},{"料酒",5},{"蒜",10},{"辣椒粉",3}}},
        // 宴客节日
        {"清蒸鲈鱼","宴客大菜",25,"dinner",{"宴客节日","高蛋白","咸鲜","轻食"},{{"鲈鱼",600},{"姜",15},{"大葱",20},{"生抽",15},{"食用油",10}}},
        {"红烧牛肉","冬季硬菜",90,"dinner",{"宴客节日","咸鲜","高蛋白"},{{"牛腩",500},{"酱油",25},{"糖",20},{"姜",15},{"料酒",15},{"八角",5},{"花椒",3}}},
        {"香辣虾","宴客快手菜",15,"dinner",{"宴客节日","辣","咸鲜","高蛋白"},{{"虾仁",400},{"辣椒",20},{"蒜",15},{"食用油",15},{"盐",3},{"花椒",5}}},
        // 减脂/增肌
        {"鸡胸肉沙拉","健身标配",10,"any",{"凉菜沙拉","免开火","增肌","减脂","高蛋白"},{{"鸡胸肉",200},{"生菜",100},{"西红柿",100},{"黄瓜",100},{"橄榄油",10},{"醋",15}}},
        {"燕麦牛奶","早餐营养",5,"breakfast",{"免开火","增肌","高蛋白"},{{"牛奶",250},{"燕麦",50},{"糖",5}}},
        {"水煮蛋","最简单蛋白质",10,"breakfast",{"高蛋白","增肌"},{{"鸡蛋",200}}},
        {"牛肉沙拉","健身大餐",15,"any",{"凉菜沙拉","免开火","增肌","高蛋白"},{{"牛肉",150},{"生菜",100},{"西兰花",100},{"西红柿",100},{"橄榄油",10},{"盐",3}}},
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
        for (int j = 0; j < 6 && rc.items[j].in; j++) {
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
        for (int t = 0; t < 5 && rc.tags[t]; t++) {
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
