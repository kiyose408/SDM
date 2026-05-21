#include "ingredient_service.h"
#include "data/ingredient_repository.h"
#include "utils/uuid_utils.h"
#include "utils/time_utils.h"

#include <QDebug>

namespace smart_diet {

IngredientService::IngredientService(IngredientRepository *repo, QObject *parent)
    : BaseService(parent), repo_(repo) { seedSystemIngredients(); }

void IngredientService::seedSystemIngredients() {
    if (repo_->getAll().count() > 0) return;  // 已有数据，跳过

    struct { const char *n, *c; double cal, pro, carb, fat; } items[] = {
        {"鸡蛋",     "meat",      144, 13.3, 1.5,  9.5},
        {"西红柿",   "vegetable",  20,  1.0,  3.5,  0.3},
        {"大米",     "staple",    116,  2.6, 25.9,  0.3},
        {"猪瘦肉",   "meat",      143, 20.3,  1.5,  6.2},
        {"土豆",     "vegetable",  81,  2.0, 17.5,  0.1},
        {"鸡胸肉",   "meat",      133, 31.0,  0.0,  1.2},
        {"豆腐",     "other",      76,  8.1,  1.9,  3.7},
        {"牛奶",     "dairy",      54,  3.0,  4.9,  3.2},
        {"盐",       "condiment",   0,  0.0,  0.0,  0.0},
        {"食用油",   "condiment", 899,  0.0,  0.0, 99.9},
    };

    const QString now = utcNow();
    for (const auto &item : items) {
        Ingredient i;
        i.id               = generateUuid();
        i.name             = QString::fromUtf8(item.n);
        i.unit             = QStringLiteral("g");
        i.calories_per_100 = item.cal;
        i.protein_per_100  = item.pro;
        i.carbs_per_100    = item.carb;
        i.fat_per_100      = item.fat;
        i.category         = QString::fromUtf8(item.c);
        i.is_system        = true;
        i.created_at       = now;
        i.updated_at       = now;
        repo_->save(i);
    }
    qDebug() << "[IngredientService] Seeded" << sizeof(items)/sizeof(items[0]) << "ingredients";
}



QVariantMap IngredientService::toMap(const Ingredient &i) const {
    QVariantMap m;
    m[QStringLiteral("id")]               = i.id;
    m[QStringLiteral("name")]             = i.name;
    m[QStringLiteral("unit")]             = i.unit;
    m[QStringLiteral("caloriesPer100")]   = i.calories_per_100;
    m[QStringLiteral("proteinPer100")]    = i.protein_per_100;
    m[QStringLiteral("carbsPer100")]      = i.carbs_per_100;
    m[QStringLiteral("fatPer100")]        = i.fat_per_100;
    m[QStringLiteral("category")]         = i.category;
    return m;
}

QVariantList IngredientService::getAll() {
    QVariantList list;
    for (const auto &i : repo_->getAll()) list.append(toMap(i));
    return list;
}

QVariantList IngredientService::getByCategory(const QString &category) {
    QVariantList list;
    for (const auto &i : repo_->getByCategory(category)) list.append(toMap(i));
    return list;
}

QVariantList IngredientService::searchByName(const QString &keyword) {
    QVariantList list;
    for (const auto &i : repo_->searchByName(keyword)) list.append(toMap(i));
    return list;
}

QVariantMap IngredientService::getById(const QString &id) {
    auto opt = repo_->getById(id);
    return opt.has_value() ? toMap(opt.value()) : QVariantMap();
}

} // namespace smart_diet
