#include "inventory_repository.h"
#include "data/orm_helper.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QDateTime>
#include <QDebug>

namespace smart_diet {

InventoryBatchRepository::InventoryBatchRepository(const QSqlDatabase &db, QObject *parent)
    : QObject(parent), db_(db) {}

InventoryBatch InventoryBatchRepository::mapRow(const QSqlQuery &query) {
    InventoryBatch b;
    b.id               = OrmHelper::readString(query, QStringLiteral("id"));
    b.family_id        = OrmHelper::readString(query, QStringLiteral("family_id"));
    b.ingredient_id    = OrmHelper::readString(query, QStringLiteral("ingredient_id"));
    b.batch_quantity   = OrmHelper::readReal(query, QStringLiteral("batch_quantity"));
    b.unit             = OrmHelper::readString(query, QStringLiteral("unit"));
    b.expiry_date      = OrmHelper::readString(query, QStringLiteral("expiry_date"));
    b.purchase_date    = OrmHelper::readString(query, QStringLiteral("purchase_date"));
    b.source           = OrmHelper::readString(query, QStringLiteral("source"));
    b.added_by         = OrmHelper::readString(query, QStringLiteral("added_by"));
    b.is_deleted       = OrmHelper::readBool(query, QStringLiteral("is_deleted"));
    b.version          = OrmHelper::readInt(query, QStringLiteral("version"));
    b.created_at       = OrmHelper::readString(query, QStringLiteral("created_at"));
    b.updated_at       = OrmHelper::readString(query, QStringLiteral("updated_at"));
    b.ingredient_name  = OrmHelper::readString(query, QStringLiteral("ingredient_name"));
    b.ingredient_category = OrmHelper::readString(query, QStringLiteral("ingredient_category"));
    return b;
}

std::optional<InventoryBatch> InventoryBatchRepository::getById(const QString &id) {
    QSqlQuery q(db_);
    q.prepare("SELECT id, family_id, ingredient_id, batch_quantity, unit, expiry_date, purchase_date, source, added_by, is_deleted, version, created_at, updated_at FROM inventory_batches WHERE id=:id");
    q.bindValue(":id", id);
    if (q.exec() && q.next()) return mapRow(q);
    return std::nullopt;
}

QList<InventoryBatch> InventoryBatchRepository::getAll() {
    QList<InventoryBatch> list;
    QSqlQuery q(db_);
    q.exec("SELECT * FROM inventory_batches WHERE is_deleted=0 ORDER BY expiry_date ASC");
    while (q.next()) list.append(mapRow(q));
    return list;
}

bool InventoryBatchRepository::save(const InventoryBatch &entity) {
    QSqlQuery q(db_);
    q.prepare("INSERT INTO inventory_batches (id, family_id, ingredient_id, batch_quantity, unit, expiry_date, purchase_date, source, added_by, is_deleted, version, created_at, updated_at) VALUES (:id,:fid,:iid,:qty,:unit,:exp,:pur,:src,:by,:del,:ver,:ca,:ua)");
    q.bindValue(":id",  entity.id);
    q.bindValue(":fid", entity.family_id);
    q.bindValue(":iid", entity.ingredient_id);
    q.bindValue(":qty", entity.batch_quantity);
    q.bindValue(":unit", entity.unit);
    q.bindValue(":exp", entity.expiry_date.isEmpty() ? QVariant() : entity.expiry_date);
    q.bindValue(":pur", entity.purchase_date.isEmpty() ? QVariant() : entity.purchase_date);
    q.bindValue(":src", entity.source);
    q.bindValue(":by",  entity.added_by.isEmpty() ? QVariant() : entity.added_by);
    q.bindValue(":del", entity.is_deleted ? 1 : 0);
    q.bindValue(":ver", entity.version);
    q.bindValue(":ca",  entity.created_at);
    q.bindValue(":ua",  entity.updated_at);
    if (!q.exec()) { qWarning() << "[InventoryBatchRepo] save failed:" << q.lastError().text(); return false; }
    return true;
}

bool InventoryBatchRepository::update(const InventoryBatch &entity) {
    QSqlQuery q(db_);
    q.prepare("UPDATE inventory_batches SET family_id=:fid, ingredient_id=:iid, batch_quantity=:qty, unit=:unit, expiry_date=:exp, purchase_date=:pur, source=:src, added_by=:by, is_deleted=:del, version=version+1, updated_at=:ua WHERE id=:id");
    q.bindValue(":fid", entity.family_id);
    q.bindValue(":iid", entity.ingredient_id);
    q.bindValue(":qty", entity.batch_quantity);
    q.bindValue(":unit", entity.unit);
    q.bindValue(":exp", entity.expiry_date.isEmpty() ? QVariant() : entity.expiry_date);
    q.bindValue(":pur", entity.purchase_date.isEmpty() ? QVariant() : entity.purchase_date);
    q.bindValue(":src", entity.source);
    q.bindValue(":by",  entity.added_by.isEmpty() ? QVariant() : entity.added_by);
    q.bindValue(":del", entity.is_deleted ? 1 : 0);
    q.bindValue(":ua",  entity.updated_at);
    q.bindValue(":id",  entity.id);
    if (!q.exec()) { qWarning() << "[InventoryBatchRepo] update failed:" << q.lastError().text(); return false; }
    return true;
}

bool InventoryBatchRepository::softDelete(const QString &id) {
    QSqlQuery q(db_);
    q.prepare("UPDATE inventory_batches SET is_deleted=1, updated_at=:ua WHERE id=:id");
    q.bindValue(":ua", QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    q.bindValue(":id", id);
    return q.exec();
}

bool InventoryBatchRepository::updateWithVersion(const InventoryBatch &entity, int expectedVersion) {
    QSqlQuery q(db_);
    q.prepare("UPDATE inventory_batches SET batch_quantity=:qty, expiry_date=:exp, updated_at=:ua, version=version+1 WHERE id=:id AND version=:ver");
    q.bindValue(":qty", entity.batch_quantity);
    q.bindValue(":exp", entity.expiry_date.isEmpty() ? QVariant() : entity.expiry_date);
    q.bindValue(":ua",  entity.updated_at);
    q.bindValue(":id",  entity.id);
    q.bindValue(":ver", expectedVersion);
    return q.exec() && q.numRowsAffected() > 0;
}

QList<InventoryBatch> InventoryBatchRepository::getByFamilyAndIngredient(const QString &familyId, const QString &ingredientId) {
    QList<InventoryBatch> list;
    QSqlQuery q(db_);
    q.prepare("SELECT * FROM inventory_batches WHERE family_id=:fid AND ingredient_id=:iid AND is_deleted=0 ORDER BY expiry_date ASC");
    q.bindValue(":fid", familyId);
    q.bindValue(":iid", ingredientId);
    if (q.exec()) while (q.next()) list.append(mapRow(q));
    return list;
}

QList<InventoryBatch> InventoryBatchRepository::getFamilyInventory(const QString &familyId) {
    QList<InventoryBatch> list;
    QSqlQuery q(db_);
    q.prepare("SELECT ib.*, i.name AS ingredient_name, i.category AS ingredient_category FROM inventory_batches ib JOIN ingredients i ON ib.ingredient_id=i.id WHERE ib.family_id=:fid AND ib.is_deleted=0 ORDER BY i.category, ib.expiry_date ASC");
    q.bindValue(":fid", familyId);
    if (q.exec()) while (q.next()) list.append(mapRow(q));
    return list;
}

InventoryBatch InventoryBatchRepository::getBatchWithName(const QString &batchId) {
    InventoryBatch b;
    QSqlQuery q(db_);
    q.prepare("SELECT ib.*, i.name AS ingredient_name, i.category AS ingredient_category FROM inventory_batches ib JOIN ingredients i ON ib.ingredient_id=i.id WHERE ib.id=:id");
    q.bindValue(":id", batchId);
    if (q.exec() && q.next()) b = mapRow(q);
    return b;
}

} // namespace smart_diet
