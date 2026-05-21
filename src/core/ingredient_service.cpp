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
        // 蔬菜 (vegetable)
        {"西红柿","vegetable",20,1.0,3.5,0.3},{"土豆","vegetable",81,2.0,17.5,0.1},
        {"青椒","vegetable",22,1.0,4.6,0.2},{"胡萝卜","vegetable",37,1.0,8.8,0.2},
        {"洋葱","vegetable",40,1.1,9.3,0.1},{"白菜","vegetable",13,1.5,2.2,0.2},
        {"黄瓜","vegetable",16,0.7,2.9,0.1},{"茄子","vegetable",21,1.1,4.9,0.2},
        {"菠菜","vegetable",23,2.9,3.6,0.4},{"西兰花","vegetable",34,2.8,6.6,0.4},
        {"生菜","vegetable",13,1.3,2.0,0.3},{"芹菜","vegetable",14,0.8,3.0,0.1},
        {"冬瓜","vegetable",11,0.4,2.6,0.1},{"南瓜","vegetable",22,0.7,5.3,0.1},
        {"豆芽","vegetable",18,2.1,2.5,0.2},{"莲藕","vegetable",73,1.9,16.4,0.2},
        {"玉米","vegetable",112,4.0,22.8,1.2},{"蘑菇","vegetable",20,2.7,2.0,0.3},
        {"木耳","vegetable",21,1.5,3.8,0.2},{"山药","vegetable",57,1.9,12.4,0.1},
        {"姜","vegetable",41,1.3,7.8,0.7},{"蒜","vegetable",126,4.5,27.9,0.2},
        {"大葱","vegetable",30,1.7,5.2,0.3},{"香菜","vegetable",23,1.8,3.0,0.5},
        {"辣椒","vegetable",38,1.1,8.7,0.3},

        // 肉禽蛋奶 (meat/dairy)
        {"鸡蛋","meat",144,13.3,1.5,9.5},{"猪瘦肉","meat",143,20.3,1.5,6.2},
        {"猪五花肉","meat",395,13.2,2.4,37.0},{"猪排骨","meat",264,18.3,0.0,20.4},
        {"牛肉","meat",125,19.9,1.9,4.2},{"牛腩","meat",160,17.9,0.2,9.2},
        {"羊肉","meat",203,19.0,0.0,14.1},{"鸡胸肉","meat",133,31.0,0.0,1.2},
        {"鸡腿肉","meat",181,16.0,0.0,13.0},{"鸡翅","meat",194,17.4,4.6,11.8},
        {"鸭肉","meat",240,15.5,0.2,19.7},{"牛奶","dairy",54,3.0,4.9,3.2},
        {"酸奶","dairy",72,2.5,9.3,2.7},{"奶酪","dairy",328,25.7,3.5,23.5},

        // 水产海鲜 (seafood)
        {"虾仁","seafood",48,12.0,0.0,0.6},{"草鱼","seafood",113,16.6,0.0,5.2},
        {"带鱼","seafood",127,17.7,0.0,4.9},{"鲈鱼","seafood",105,18.6,0.0,3.4},
        {"三文鱼","seafood",139,17.2,0.0,7.8},{"蛤蜊","seafood",62,10.1,2.1,1.5},

        // 主食谷物 (staple)
        {"大米","staple",116,2.6,25.9,0.3},{"面粉","staple",366,10.3,76.1,1.1},
        {"面条","staple",286,8.5,58.0,1.1},{"馒头","staple",223,7.0,44.2,1.1},
        {"面包","staple",312,8.3,58.1,5.1},{"红薯","staple",86,1.6,20.1,0.1},
        {"小米","staple",358,9.0,73.5,3.1},{"燕麦","staple",367,13.5,61.6,6.8},

        // 豆制品 (other)
        {"豆腐","other",76,8.1,1.9,3.7},{"豆皮","other",409,44.6,12.2,17.4},
        {"腐竹","other",459,44.6,20.3,21.7},

        // 调味品 (condiment)
        {"盐","condiment",0,0.0,0.0,0.0},{"食用油","condiment",899,0.0,0.0,99.9},
        {"酱油","condiment",53,5.6,5.5,0.1},{"醋","condiment",18,0.4,3.6,0.0},
        {"糖","condiment",400,0.0,99.9,0.0},{"料酒","condiment",50,0.5,1.0,0.0},
        {"豆瓣酱","condiment",78,3.0,12.9,1.5},{"番茄酱","condiment",81,1.7,16.5,0.4},
        {"蚝油","condiment",90,2.0,18.0,0.5},{"生抽","condiment",33,5.0,2.0,0.0},
        {"老抽","condiment",83,6.0,14.0,0.2},{"淀粉","condiment",346,1.2,85.0,0.1},
        {"花椒","condiment",258,6.7,37.8,8.9},{"八角","condiment",281,3.8,46.3,5.9},
        {"辣椒粉","condiment",282,15.0,46.0,10.0},
        {"黑胡椒","condiment",251,10.4,38.7,3.3},
        {"橄榄油","condiment",884,0.0,0.0,100.0},
        {"香油","condiment",898,0.0,0.0,99.7},
        {"柠檬","vegetable",29,1.1,9.3,0.3},
        {"花生","condiment",563,24.8,16.1,44.3},
        {"豆角","vegetable",34,2.5,4.8,0.3},
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
