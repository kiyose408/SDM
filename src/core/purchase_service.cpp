#include "purchase_service.h"
#include "data/menu_repository.h"
#include "data/recipe_repository.h"
#include "data/ingredient_repository.h"
#include "data/database_manager.h"
#include "utils/uuid_utils.h"
#include "utils/time_utils.h"
#include <QSqlQuery>
#include <QMap>
#include <QDebug>

namespace smart_diet {

struct PurchaseItem {
    QString ingredientName;
    QString category;
    double  amount = 0;
    QString ingredientId;
};

PurchaseService::PurchaseService(DailyMenuRepository *dm, MenuItemRepository *mi,
                                 RecipeRepository *rr, RecipeIngredientRepository *ri,
                                 IngredientRepository *ir, QObject *parent)
    : BaseService(parent), dmRepo_(dm), miRepo_(mi), recipeRepo_(rr), riRepo_(ri), ingRepo_(ir) {}

QVariantList PurchaseService::getTodayPurchaseList(const QString &familyId, const QString &date) {
    // 先查冰箱现有库存（按食材汇总）
    QMap<QString, double> fridgeStock;
    QSqlQuery fs(DatabaseManager::instance().database());
    fs.prepare("SELECT ingredient_id, SUM(batch_quantity) FROM inventory_batches WHERE family_id=:fid AND is_deleted=0 GROUP BY ingredient_id");
    fs.bindValue(":fid", familyId);
    if (fs.exec()) while (fs.next()) fridgeStock[fs.value(0).toString()] = fs.value(1).toDouble();

    QMap<QString, PurchaseItem> aggregated;

    QStringList meals = {QStringLiteral("breakfast"), QStringLiteral("lunch"), QStringLiteral("dinner")};
    for (const auto &mt : meals) {
        auto menuOpt = dmRepo_->getByDate(familyId, date, mt);
        if (!menuOpt.has_value()) continue;

        auto items = miRepo_->getByMenu(menuOpt->id);
        for (const auto &mi : items) {
            auto recipe = recipeRepo_->getById(mi.recipe_id);
            if (!recipe.has_value()) continue;

            double sv = mi.servings_override > 0 ? mi.servings_override / 10.0 : 1.0;
            double defSv = recipe->servings > 0 ? recipe->servings : 2.0;
            double ratio = sv / (double)defSv;

            auto riList = riRepo_->getByRecipe(mi.recipe_id);
            for (const auto &ri : riList) {
                auto ing = ingRepo_->getById(ri.ingredient_id);
                if (!ing.has_value()) continue;

                double needed = ri.amount * ratio;
                if (needed < 0.1) continue;

                if (aggregated.contains(ing->name)) {
                    aggregated[ing->name].amount += needed;
                } else {
                    PurchaseItem pi;
                    pi.ingredientName = ing->name;
                    pi.category       = ing->category;
                    pi.amount         = needed;
                    pi.ingredientId   = ing->id;
                    aggregated[ing->name] = pi;
                }
            }
        }
    }

    // 减去冰箱已有库存
    for (auto it = aggregated.begin(); it != aggregated.end(); ++it) {
        double stock = fridgeStock.value(it->ingredientId);
        it->amount -= stock;
        if (it->amount < 0) it->amount = 0;
    }

    // 持久化到 shopping_list_items 表（覆盖当日）
    QSqlQuery del(DatabaseManager::instance().database());
    del.prepare("DELETE FROM shopping_list_items WHERE family_id=:fid");
    del.bindValue(":fid", familyId);
    del.exec();

    for (auto it = aggregated.begin(); it != aggregated.end(); ++it) {
        if (it->amount <= 0) continue;
        QSqlQuery inq(DatabaseManager::instance().database());
        inq.prepare("INSERT INTO shopping_list_items (id, family_id, ingredient_id, required_quantity, is_purchased, created_at, updated_at) VALUES (:id,:fid,:iid,:qty,0,:ca,:ua)");
        inq.bindValue(":id", generateUuid());
        inq.bindValue(":fid", familyId);
        inq.bindValue(":iid", it->ingredientId);
        inq.bindValue(":qty", it->amount);
        inq.bindValue(":ca", utcNow());
        inq.bindValue(":ua", utcNow());
        inq.exec();
    }

    // 按类别排序输出（返回实际需购买量 = 已扣除冰箱库存）
    QVariantList result;
    QStringList catOrder = {QStringLiteral("meat"), QStringLiteral("seafood"),
                             QStringLiteral("vegetable"), QStringLiteral("staple"),
                             QStringLiteral("dairy"), QStringLiteral("condiment"), QStringLiteral("other")};

    for (const auto &cat : catOrder) {
        for (auto it = aggregated.begin(); it != aggregated.end(); ++it) {
            if (it->category == cat && it->amount > 0) {
                QVariantMap m;
                m[QStringLiteral("name")]         = it->ingredientName;
                m[QStringLiteral("cat")]          = it->category;
                m[QStringLiteral("need")]         = it->amount;
                m[QStringLiteral("bought")]       = 0;
                m[QStringLiteral("ingredientId")] = it->ingredientId;
                result.append(m);
            }
        }
    }
    return result;
}

bool PurchaseService::confirmPurchase(const QString &familyId, const QString &userId,
                                       const QVariantList &items) {
    for (const auto &item : items) {
        QVariantMap m = item.toMap();
        QString ingredientId = m[QStringLiteral("ingredientId")].toString();
        double bought = m[QStringLiteral("bought")].toDouble();
        if (bought <= 0 || ingredientId.isEmpty()) continue;

        auto ing = ingRepo_->getById(ingredientId);
        if (!ing.has_value()) continue;

        QString now = utcNow();
        QString unit = ing->unit.isEmpty() ? QStringLiteral("g") : ing->unit;
        QString purDate = QDateTime::currentDateTimeUtc().toString(Qt::ISODate).left(10);

        QSqlQuery q(DatabaseManager::instance().database());
        q.prepare("INSERT INTO inventory_batches (id, family_id, ingredient_id, batch_quantity, unit, expiry_date, purchase_date, source, added_by, is_deleted, version, created_at, updated_at) VALUES (:id,:fid,:iid,:qty,:unit,:exp,:pur,:src,:by,0,1,:ca,:ua)");
        q.bindValue(":id",  generateUuid());
        q.bindValue(":fid", familyId);
        q.bindValue(":iid", ingredientId);
        q.bindValue(":qty", bought);
        q.bindValue(":unit", unit);
        q.bindValue(":exp", QVariant());
        q.bindValue(":pur", purDate);
        q.bindValue(":src", QStringLiteral("purchase_list_batch"));
        q.bindValue(":by",  userId);
        q.bindValue(":ca",  now);
        q.bindValue(":ua",  now);
        q.exec();
    }

    // 标记采购清单已购买
    QSqlQuery uq(DatabaseManager::instance().database());
    uq.prepare("UPDATE shopping_list_items SET is_purchased=1, purchased_at=:pa WHERE family_id=:fid AND is_purchased=0");
    uq.bindValue(":pa", utcNow());
    uq.bindValue(":fid", familyId);
    uq.exec();
    return true;
}

} // namespace smart_diet
