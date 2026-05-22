#include "fridge_service.h"
#include "data/inventory_repository.h"
#include "data/ingredient_repository.h"
#include "data/database_manager.h"
#include "utils/uuid_utils.h"
#include "utils/time_utils.h"
#include <QSqlQuery>
#include <QDateTime>
#include <QDebug>

namespace smart_diet {

FridgeService::FridgeService(InventoryBatchRepository *ib, IngredientRepository *ir, QObject *parent)
    : BaseService(parent), ibRepo_(ib), ingRepo_(ir) {}

QVariantList FridgeService::getStock(const QString &familyId) {
    // 按食材聚合（同食材多批次合并）
    struct AggEntry {
        double qty = 0;
        QString unit;
        QString earliestDate;
    };
    QMap<QString, AggEntry> agg;

    auto batches = ibRepo_->getFamilyInventory(familyId);
    for (const auto &b : batches) {
        agg[b.ingredient_id].qty += b.batch_quantity;
        agg[b.ingredient_id].unit = b.unit;
        if (agg[b.ingredient_id].earliestDate.isEmpty() || b.purchase_date < agg[b.ingredient_id].earliestDate)
            agg[b.ingredient_id].earliestDate = b.purchase_date;
    }

    QVariantList list;
    for (auto it = agg.begin(); it != agg.end(); ++it) {
        auto ing = ingRepo_->getById(it.key());
        QVariantMap item;
        item[QStringLiteral("ingredientId")] = it.key();
        item[QStringLiteral("name")]         = ing.has_value() ? ing->name : it.key();
        item[QStringLiteral("category")]     = ing.has_value() ? ing->category : QStringLiteral("other");
        item[QStringLiteral("quantity")]     = it->qty;
        item[QStringLiteral("unit")]         = it->unit;
        item[QStringLiteral("purchaseDate")] = it->earliestDate;
        list.append(item);
    }
    return list;
}

bool FridgeService::addItem(const QString &familyId, const QString &ingredientId,
                             double quantity, const QString &unit,
                             const QString &expiryDate, const QString &addedBy) {
    InventoryBatch b;
    b.id = generateUuid();
    b.family_id     = familyId;
    b.ingredient_id = ingredientId;
    b.batch_quantity = quantity;
    b.unit          = unit.isEmpty() ? QStringLiteral("g") : unit;
    b.expiry_date   = expiryDate;
    b.purchase_date = QDateTime::currentDateTimeUtc().toString(Qt::ISODate).left(10);
    b.source        = QStringLiteral("manual_add");
    b.added_by      = addedBy;
    b.created_at    = utcNow();
    b.updated_at    = b.created_at;
    return ibRepo_->save(b);
}

bool FridgeService::deductBatch(const QString &batchId, double amount, const QString &operatorId, const QString &reason) {
    auto batch = ibRepo_->getBatchWithName(batchId);
    if (batch.batch_quantity <= 0) return false;

    double newQty = batch.batch_quantity - amount;
    if (newQty < 0) newQty = 0;
    batch.batch_quantity = newQty;
    batch.updated_at = utcNow();
    bool ok = ibRepo_->update(batch);

    // 记录日志
    if (ok) {
        QSqlQuery lq(DatabaseManager::instance().database());
        lq.prepare("INSERT INTO inventory_logs (id, family_id, batch_id, ingredient_id, operation, quantity_change, quantity_before, quantity_after, reason, operator_id, created_at, updated_at) VALUES (:id,:fid,:bid,:iid,:op,:chg,:bef,:aft,:rsn,:oid,:ca,:ua)");
        lq.bindValue(":id",  generateUuid());
        lq.bindValue(":fid", batch.family_id);
        lq.bindValue(":bid", batchId);
        lq.bindValue(":iid", batch.ingredient_id);
        lq.bindValue(":op",  "deduct");
        lq.bindValue(":chg", -amount);
        lq.bindValue(":bef", batch.batch_quantity + amount);
        lq.bindValue(":aft", newQty);
        lq.bindValue(":rsn", reason);
        lq.bindValue(":oid", operatorId);
        lq.bindValue(":ca",  utcNow());
        lq.bindValue(":ua",  utcNow());
        lq.exec();
    }
    return ok;
}

bool FridgeService::deductByFifo(const QString &familyId, const QString &ingredientId,
                                  double totalNeeded, const QString &operatorId, const QString &reason) {
    auto batches = ibRepo_->getByFamilyAndIngredient(familyId, ingredientId);
    double remaining = totalNeeded;
    for (const auto &b : batches) {
        if (remaining <= 0) break;
        double take = qMin(remaining, b.batch_quantity);
        if (take > 0) deductBatch(b.id, take, operatorId, reason);
        remaining -= take;
    }
    return remaining <= 0;
}

bool FridgeService::calibrateStock(const QString &familyId, const QString &ingredientId,
                                     double newTotal, const QString &operatorId) {
    double currentTotal = 0;
    auto batches = ibRepo_->getByFamilyAndIngredient(familyId, ingredientId);
    for (const auto &b : batches) currentTotal += b.batch_quantity;

    if (qAbs(currentTotal - newTotal) < 0.001) return true;

    auto ing = ingRepo_->getById(ingredientId);
    QString unit = ing.has_value() && !ing->unit.isEmpty() ? ing->unit : QStringLiteral("g");

    // 清空所有旧批次
    for (const auto &b : batches) ibRepo_->softDelete(b.id);

    // 新建校准批次
    if (newTotal > 0) {
        InventoryBatch nb;
        nb.id = generateUuid();
        nb.family_id       = familyId;
        nb.ingredient_id   = ingredientId;
        nb.batch_quantity   = newTotal;
        nb.unit            = unit;
        nb.purchase_date   = QDateTime::currentDateTimeUtc().toString(Qt::ISODate).left(10);
        nb.source          = QStringLiteral("calibration");
        nb.added_by        = operatorId;
        nb.created_at      = utcNow();
        nb.updated_at      = nb.created_at;
        ibRepo_->save(nb);
    }

    // 记录日志
    QSqlQuery lq(DatabaseManager::instance().database());
    lq.prepare("INSERT INTO inventory_logs (id, family_id, batch_id, ingredient_id, operation, quantity_change, quantity_before, quantity_after, reason, operator_id, created_at, updated_at) VALUES (:id,:fid,:bid,:iid,:op,:chg,:bef,:aft,:rsn,:oid,:ca,:ua)");
    lq.bindValue(":id",  generateUuid());
    lq.bindValue(":fid", familyId);
    lq.bindValue(":bid", QString());
    lq.bindValue(":iid", ingredientId);
    lq.bindValue(":op",  "calibrate");
    lq.bindValue(":chg", newTotal - currentTotal);
    lq.bindValue(":bef", currentTotal);
    lq.bindValue(":aft", newTotal);
    lq.bindValue(":rsn", QStringLiteral("manual_calibration"));
    lq.bindValue(":oid", operatorId);
    lq.bindValue(":ca",  utcNow());
    lq.bindValue(":ua",  utcNow());
    lq.exec();

    return true;
}

int FridgeService::importFromPurchase(const QString &familyId, const QString &addedBy) {
    int count = 0;
    QSqlQuery q(DatabaseManager::instance().database());
    q.prepare("SELECT sli.ingredient_id, sli.required_quantity, i.unit FROM shopping_list_items sli JOIN ingredients i ON sli.ingredient_id=i.id WHERE sli.family_id=:fid AND sli.is_purchased=0");
    q.bindValue(":fid", familyId);
    if (!q.exec()) return 0;
    while (q.next()) {
        QString iid = q.value(0).toString();
        double qty  = q.value(1).toDouble();
        QString unit = q.value(2).toString();
        addItem(familyId, iid, qty, unit, QDateTime::currentDateTimeUtc().toString(Qt::ISODate).left(10), addedBy);
        count++;
    }
    // 标记已购买
    QSqlQuery uq(DatabaseManager::instance().database());
    uq.prepare("UPDATE shopping_list_items SET is_purchased=1, purchased_at=:pa WHERE family_id=:fid AND is_purchased=0");
    uq.bindValue(":pa", utcNow());
    uq.bindValue(":fid", familyId);
    uq.exec();
    return count;
}

} // namespace smart_diet
