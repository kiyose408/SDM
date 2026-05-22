#include "consumption_service.h"
#include "data/recipe_repository.h"
#include "data/ingredient_repository.h"
#include "data/inventory_repository.h"
#include "data/database_manager.h"
#include "data/menu_repository.h"
#include "utils/uuid_utils.h"
#include "utils/time_utils.h"
#include <QSqlQuery>
#include <QDateTime>
#include <QDebug>

namespace smart_diet {

ConsumptionService::ConsumptionService(MenuItemRepository *mi, RecipeRepository *rr,
                                         RecipeIngredientRepository *ri, IngredientRepository *ir,
                                         InventoryBatchRepository *ib, QObject *parent)
    : BaseService(parent), miRepo_(mi), recipeRepo_(rr), riRepo_(ri), ingRepo_(ir), ibRepo_(ib) {}

// 辅助：按 menu_item ID 查询单条记录
static QVariantMap queryMenuItem(const QString &itemId) {
    QVariantMap m;
    QSqlQuery q(DatabaseManager::instance().database());
    q.prepare("SELECT * FROM menu_items WHERE id=:id");
    q.bindValue(":id", itemId);
    if (q.exec() && q.next()) {
        m["id"] = q.value("id").toString();
        m["menu_id"] = q.value("menu_id").toString();
        m["recipe_id"] = q.value("recipe_id").toString();
        m["servings_override"] = q.value("servings_override").toInt();
        m["status"] = q.value("status").toString();
    }
    return m;
}

bool ConsumptionService::consumeRecipe(const QString &menuItemId, const QString &familyId,
                                        double servings, const QString &operatorId) {
    auto mi = queryMenuItem(menuItemId);
    if (mi.isEmpty() || mi["status"].toString() == QStringLiteral("completed")) return false;

    QString recipeId = mi["recipe_id"].toString();
    auto recipe = recipeRepo_->getById(recipeId);
    if (!recipe.has_value()) return false;

    double defSv = recipe->servings > 0 ? recipe->servings : 2.0;
    double ratio = servings / defSv;

    QString batchId = generateUuid();
    QString reason = QStringLiteral("cooking:") + batchId + QStringLiteral(":") + QString::number(servings, 'f', 1);
    QString now = utcNow();

    auto riList = riRepo_->getByRecipe(recipeId);
    bool allOk = true;

    for (const auto &ri : riList) {
        auto ing = ingRepo_->getById(ri.ingredient_id);
        if (!ing.has_value()) continue;
        double needed = ri.amount * ratio;
        if (needed < 0.01) continue;

        auto batches = ibRepo_->getByFamilyAndIngredient(familyId, ri.ingredient_id);
        double remaining = needed;
        for (const auto &b : batches) {
            if (remaining <= 0) break;
            double take = qMin(remaining, b.batch_quantity);
            if (take > 0) {
                auto batch = ibRepo_->getById(b.id);
                if (!batch.has_value()) continue;
                double before = batch->batch_quantity;
                batch->batch_quantity -= take;
                batch->updated_at = now;
                ibRepo_->update(*batch);

                QSqlQuery lq(DatabaseManager::instance().database());
                lq.prepare("INSERT INTO inventory_logs (id, family_id, batch_id, ingredient_id, operation, quantity_change, quantity_before, quantity_after, reason, related_menu_id, operator_id, snapshot_recipe_name, snapshot_ingredient_name, snapshot_servings, created_at, updated_at) VALUES (:id,:fid,:bid,:iid,'deduct',:chg,:bef,:aft,:rsn,:mid,:oid,:rn,:in,:sv,:ca,:ua)");
                lq.bindValue(":id",  generateUuid());
                lq.bindValue(":fid", familyId);
                lq.bindValue(":bid", b.id);
                lq.bindValue(":iid", ri.ingredient_id);
                lq.bindValue(":chg", -take);
                lq.bindValue(":bef", before);
                lq.bindValue(":aft", batch->batch_quantity);
                lq.bindValue(":rsn", reason);
                lq.bindValue(":mid", menuItemId);
                lq.bindValue(":oid", operatorId);
                lq.bindValue(":rn",  recipe->name);
                lq.bindValue(":in",  ing->name);
                lq.bindValue(":sv",  servings);
                lq.bindValue(":ca",  now);
                lq.bindValue(":ua",  now);
                if (!lq.exec()) { qWarning() << "[Consumption] log insert failed"; allOk = false; }
            }
            remaining -= take;
        }
        if (remaining > 0) allOk = false;
    }

    if (allOk) miRepo_->updateStatus(menuItemId, QStringLiteral("completed"));

    return allOk;
}

bool ConsumptionService::undoConsume(const QString &menuItemId, const QString &operatorId) {
    auto mi = queryMenuItem(menuItemId);
    if (mi.isEmpty() || mi["status"].toString() != QStringLiteral("completed")) return false;

    QSqlQuery fq(DatabaseManager::instance().database());
    fq.prepare("SELECT reason FROM inventory_logs WHERE related_menu_id=:mid AND operation='deduct' AND reason LIKE 'cooking:%' ORDER BY created_at DESC LIMIT 1");
    fq.bindValue(":mid", menuItemId);
    if (!fq.exec() || !fq.next()) return false;

    QString reason = fq.value(0).toString();
    QString now = utcNow();

    QSqlQuery lq(DatabaseManager::instance().database());
    lq.prepare("SELECT family_id, batch_id, ingredient_id, quantity_change FROM inventory_logs WHERE related_menu_id=:mid AND reason=:rsn AND operation='deduct'");
    lq.bindValue(":mid", menuItemId);
    lq.bindValue(":rsn", reason);
    if (!lq.exec()) return false;

    while (lq.next()) {
        QString famId  = lq.value(0).toString();
        QString batchId = lq.value(1).toString();
        QString ingId  = lq.value(2).toString();
        double deducted = -lq.value(3).toDouble();

        auto batch = ibRepo_->getById(batchId);
        if (!batch.has_value()) continue;

        double before = batch->batch_quantity;
        batch->batch_quantity += deducted;
        batch->updated_at = now;
        ibRepo_->update(*batch);

        QSqlQuery ulq(DatabaseManager::instance().database());
        ulq.prepare("INSERT INTO inventory_logs (id, family_id, batch_id, ingredient_id, operation, quantity_change, quantity_before, quantity_after, reason, related_menu_id, operator_id, created_at, updated_at) VALUES (:id,:fid,:bid,:iid,'undo',:chg,:bef,:aft,:rsn,:mid,:oid,:ca,:ua)");
        ulq.bindValue(":id",  generateUuid());
        ulq.bindValue(":fid", famId);
        ulq.bindValue(":bid", batchId);
        ulq.bindValue(":iid", ingId);
        ulq.bindValue(":chg", deducted);
        ulq.bindValue(":bef", before);
        ulq.bindValue(":aft", batch->batch_quantity);
        ulq.bindValue(":rsn", QStringLiteral("undo:") + reason);
        ulq.bindValue(":mid", menuItemId);
        ulq.bindValue(":oid", operatorId);
        ulq.bindValue(":ca",  now);
        ulq.bindValue(":ua",  now);
        ulq.exec();
    }

    miRepo_->updateStatus(menuItemId, QStringLiteral("pending"));
    return true;
}

QVariantList ConsumptionService::getConsumptionHistory(const QString &familyId, int limit) {
    QVariantList result;
    QSqlQuery q(DatabaseManager::instance().database());
    q.prepare("SELECT DISTINCT related_menu_id, snapshot_recipe_name, snapshot_servings, operator_id, created_at FROM inventory_logs WHERE family_id=:fid AND operation='deduct' AND reason LIKE 'cooking:%' ORDER BY created_at DESC LIMIT :lim");
    q.bindValue(":fid", familyId);
    q.bindValue(":lim", limit);
    if (!q.exec()) return result;

    while (q.next()) {
        QString menuItemId = q.value(0).toString();
        QSqlQuery det(DatabaseManager::instance().database());
        det.prepare("SELECT reason FROM inventory_logs WHERE related_menu_id=:mid AND operation='deduct' ORDER BY created_at DESC LIMIT 1");
        det.bindValue(":mid", menuItemId);
        if (!det.exec() || !det.next()) continue;
        QString reasonTag = det.value(0).toString();

        QSqlQuery det2(DatabaseManager::instance().database());
        det2.prepare("SELECT snapshot_ingredient_name, ABS(quantity_change) FROM inventory_logs WHERE related_menu_id=:mid AND reason=:rsn AND operation='deduct'");
        det2.bindValue(":mid", menuItemId);
        det2.bindValue(":rsn", reasonTag);
        QStringList details;
        if (det2.exec()) while (det2.next())
            details.append(det2.value(0).toString() + " " + QString::number(det2.value(1).toDouble(), 'f', 0) + "g");

        QVariantMap m;
        m[QStringLiteral("menuItemId")] = menuItemId;
        m[QStringLiteral("recipeName")] = q.value(1).toString();
        m[QStringLiteral("servings")]  = q.value(2).toDouble();
        m[QStringLiteral("operatorId")] = q.value(3).toString();
        m[QStringLiteral("createdAt")]  = q.value(4).toString();
        m[QStringLiteral("details")]    = details.join(QStringLiteral(", "));
        result.append(m);
    }
    return result;
}

} // namespace smart_diet
