#include "ingredient_repository.h"
#include "database_manager.h"
#include "orm_helper.h"
#include "utils/time_utils.h"

#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>

namespace smart_diet {

IngredientRepository::IngredientRepository(QObject *parent)
    : QObject(parent), db_(DatabaseManager::instance().database()) {}

Ingredient IngredientRepository::mapRow(const QSqlQuery &query) {
    Ingredient i;
    i.id                = OrmHelper::readString(query, QStringLiteral("id"));
    i.name              = OrmHelper::readString(query, QStringLiteral("name"));
    i.unit              = OrmHelper::readString(query, QStringLiteral("unit"));
    i.calories_per_100  = OrmHelper::readReal(query, QStringLiteral("calories_per_100"));
    i.protein_per_100   = OrmHelper::readReal(query, QStringLiteral("protein_per_100"));
    i.carbs_per_100     = OrmHelper::readReal(query, QStringLiteral("carbs_per_100"));
    i.fat_per_100       = OrmHelper::readReal(query, QStringLiteral("fat_per_100"));
    i.image_url         = OrmHelper::readString(query, QStringLiteral("image_url"));
    i.category          = OrmHelper::readString(query, QStringLiteral("category"));
    i.is_system         = OrmHelper::readBool(query, QStringLiteral("is_system"));
    i.creator_id        = OrmHelper::readString(query, QStringLiteral("creator_id"));
    i.version           = OrmHelper::readInt(query, QStringLiteral("version"));
    i.created_at        = OrmHelper::readString(query, QStringLiteral("created_at"));
    i.updated_at        = OrmHelper::readString(query, QStringLiteral("updated_at"));
    return i;
}

std::optional<Ingredient> IngredientRepository::getById(const QString &uuid) {
    QSqlQuery query(db_);
    query.prepare(QStringLiteral("SELECT * FROM ingredients WHERE id=:id"));
    OrmHelper::bindId(query, uuid);
    if (!query.exec() || !query.next()) return std::nullopt;
    return mapRow(query);
}

QList<Ingredient> IngredientRepository::getAll() {
    QList<Ingredient> list;
    QSqlQuery query(db_);
    query.prepare(QStringLiteral("SELECT * FROM ingredients ORDER BY name ASC"));
    if (!query.exec()) return list;
    while (query.next()) list.append(mapRow(query));
    return list;
}

bool IngredientRepository::save(const Ingredient &entity) {
    QSqlQuery query(db_);
    query.prepare(QStringLiteral(
        "INSERT INTO ingredients (id,name,unit,calories_per_100,protein_per_100,carbs_per_100,"
        "fat_per_100,image_url,category,is_system,creator_id,version,created_at,updated_at)"
        " VALUES (:id,:name,:unit,:cal,:pro,:carb,:fat,:img,:cat,:sys,:cid,:ver,:ca,:ua)"));
    query.bindValue(QStringLiteral(":id"), entity.id);
    query.bindValue(QStringLiteral(":name"), entity.name);
    query.bindValue(QStringLiteral(":unit"), entity.unit);
    query.bindValue(QStringLiteral(":cal"), entity.calories_per_100);
    query.bindValue(QStringLiteral(":pro"), entity.protein_per_100);
    query.bindValue(QStringLiteral(":carb"), entity.carbs_per_100);
    query.bindValue(QStringLiteral(":fat"), entity.fat_per_100);
    query.bindValue(QStringLiteral(":img"), entity.image_url.isEmpty() ? QVariant() : entity.image_url);
    query.bindValue(QStringLiteral(":cat"), entity.category);
    query.bindValue(QStringLiteral(":sys"), entity.is_system ? 1 : 0);
    query.bindValue(QStringLiteral(":cid"), entity.creator_id.isEmpty() ? QVariant() : entity.creator_id);
    query.bindValue(QStringLiteral(":ver"), entity.version);
    query.bindValue(QStringLiteral(":ca"), entity.created_at);
    query.bindValue(QStringLiteral(":ua"), entity.updated_at);
    if (!query.exec()) { qWarning() << "[IngredientRepo] save failed:" << query.lastError().text(); return false; }
    emit ingredientInserted(entity.id);
    return true;
}

bool IngredientRepository::update(const Ingredient &entity) {
    QSqlQuery query(db_);
    query.prepare(QStringLiteral(
        "UPDATE ingredients SET name=:name,unit=:unit,calories_per_100=:cal,"
        "protein_per_100=:pro,carbs_per_100=:carb,fat_per_100=:fat,"
        "image_url=:img,category=:cat,version=version+1,updated_at=:ua WHERE id=:id"));
    OrmHelper::bindId(query, entity.id);
    query.bindValue(QStringLiteral(":name"), entity.name);
    query.bindValue(QStringLiteral(":unit"), entity.unit);
    query.bindValue(QStringLiteral(":cal"), entity.calories_per_100);
    query.bindValue(QStringLiteral(":pro"), entity.protein_per_100);
    query.bindValue(QStringLiteral(":carb"), entity.carbs_per_100);
    query.bindValue(QStringLiteral(":fat"), entity.fat_per_100);
    query.bindValue(QStringLiteral(":img"), entity.image_url.isEmpty() ? QVariant() : entity.image_url);
    query.bindValue(QStringLiteral(":cat"), entity.category);
    query.bindValue(QStringLiteral(":ua"), entity.updated_at);
    if (!query.exec()) { qWarning() << "[IngredientRepo] update failed:" << query.lastError().text(); return false; }
    emit ingredientUpdated(entity.id);
    return true;
}

bool IngredientRepository::softDelete(const QString &uuid) {
    QSqlQuery query(db_);
    query.prepare(QStringLiteral("DELETE FROM ingredients WHERE id=:id"));
    OrmHelper::bindId(query, uuid);
    return query.exec();
}

bool IngredientRepository::updateWithVersion(const Ingredient &entity, int expectedVersion) {
    QSqlQuery query(db_);
    query.prepare(QStringLiteral(
        "UPDATE ingredients SET name=:name,version=version+1,updated_at=:ua"
        " WHERE id=:id AND version=:version"));
    OrmHelper::bindIdAndVersion(query, entity.id, expectedVersion);
    query.bindValue(QStringLiteral(":name"), entity.name);
    query.bindValue(QStringLiteral(":ua"), entity.updated_at);
    if (!query.exec() || query.numRowsAffected() == 0) return false;
    emit ingredientUpdated(entity.id);
    return true;
}

QList<Ingredient> IngredientRepository::getByCategory(const QString &category) {
    QList<Ingredient> list;
    QSqlQuery query(db_);
    query.prepare(QStringLiteral("SELECT * FROM ingredients WHERE category=:cat ORDER BY name ASC"));
    query.bindValue(QStringLiteral(":cat"), category);
    if (!query.exec()) return list;
    while (query.next()) list.append(mapRow(query));
    return list;
}

QList<Ingredient> IngredientRepository::searchByName(const QString &keyword) {
    QList<Ingredient> list;
    QSqlQuery query(db_);
    query.prepare(QStringLiteral("SELECT * FROM ingredients WHERE name LIKE :kw ORDER BY name ASC"));
    query.bindValue(QStringLiteral(":kw"), QStringLiteral("%%1%").arg(keyword));
    if (!query.exec()) return list;
    while (query.next()) list.append(mapRow(query));
    return list;
}

} // namespace smart_diet
