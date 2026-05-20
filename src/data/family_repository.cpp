#include "family_repository.h"
#include "database_manager.h"
#include "orm_helper.h"
#include "utils/time_utils.h"

#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>

namespace smart_diet {

FamilyRepository::FamilyRepository(QObject *parent)
    : QObject(parent)
    , db_(DatabaseManager::instance().database())
{
}

Family FamilyRepository::mapRow(const QSqlQuery &query) {
    Family f;
    f.id                     = OrmHelper::readString(query, QStringLiteral("id"));
    f.name                   = OrmHelper::readString(query, QStringLiteral("name"));
    f.invite_code            = OrmHelper::readString(query, QStringLiteral("invite_code"));
    f.invite_code_expires_at = OrmHelper::readString(query, QStringLiteral("invite_code_expires_at"));
    f.creator_id             = OrmHelper::readString(query, QStringLiteral("creator_id"));
    f.is_deleted             = OrmHelper::readBool(query, QStringLiteral("is_deleted"));
    f.version                = OrmHelper::readInt(query, QStringLiteral("version"));
    f.created_at             = OrmHelper::readString(query, QStringLiteral("created_at"));
    f.updated_at             = OrmHelper::readString(query, QStringLiteral("updated_at"));
    return f;
}

std::optional<Family> FamilyRepository::getById(const QString &uuid) {
    QSqlQuery query(db_);
    query.prepare(QStringLiteral("SELECT * FROM families WHERE id = :id AND is_deleted = 0"));
    OrmHelper::bindId(query, uuid);
    if (!query.exec() || !query.next()) return std::nullopt;
    return mapRow(query);
}

QList<Family> FamilyRepository::getAll() {
    QList<Family> list;
    QSqlQuery query(db_);
    query.prepare(QStringLiteral("SELECT * FROM families WHERE is_deleted = 0 ORDER BY created_at DESC"));
    if (!query.exec()) return list;
    while (query.next()) list.append(mapRow(query));
    return list;
}

bool FamilyRepository::save(const Family &entity) {
    QSqlQuery query(db_);
    query.prepare(QStringLiteral(
        "INSERT INTO families (id, name, invite_code, invite_code_expires_at, creator_id,"
        " is_deleted, version, created_at, updated_at)"
        " VALUES (:id, :name, :invite_code, :invite_code_expires_at, :creator_id,"
        " :is_deleted, :version, :created_at, :updated_at)"));
    query.bindValue(QStringLiteral(":id"), entity.id);
    query.bindValue(QStringLiteral(":name"), entity.name);
    query.bindValue(QStringLiteral(":invite_code"), entity.invite_code);
    query.bindValue(QStringLiteral(":invite_code_expires_at"),
                    entity.invite_code_expires_at.isEmpty() ? QVariant() : entity.invite_code_expires_at);
    query.bindValue(QStringLiteral(":creator_id"), entity.creator_id);
    query.bindValue(QStringLiteral(":is_deleted"), entity.is_deleted ? 1 : 0);
    query.bindValue(QStringLiteral(":version"), entity.version);
    query.bindValue(QStringLiteral(":created_at"), entity.created_at);
    query.bindValue(QStringLiteral(":updated_at"), entity.updated_at);
    if (!query.exec()) { qWarning() << "[FamilyRepository] save failed:" << query.lastError().text(); return false; }
    emit familyInserted(entity.id);
    return true;
}

bool FamilyRepository::update(const Family &entity) {
    QSqlQuery query(db_);
    query.prepare(QStringLiteral(
        "UPDATE families SET name=:name, invite_code=:invite_code,"
        " invite_code_expires_at=:invite_code_expires_at,"
        " version=version+1, updated_at=:updated_at WHERE id=:id"));
    OrmHelper::bindId(query, entity.id);
    query.bindValue(QStringLiteral(":name"), entity.name);
    query.bindValue(QStringLiteral(":invite_code"), entity.invite_code);
    query.bindValue(QStringLiteral(":invite_code_expires_at"),
                    entity.invite_code_expires_at.isEmpty() ? QVariant() : entity.invite_code_expires_at);
    query.bindValue(QStringLiteral(":updated_at"), entity.updated_at);
    if (!query.exec()) { qWarning() << "[FamilyRepository] update failed:" << query.lastError().text(); return false; }
    emit familyUpdated(entity.id);
    return true;
}

bool FamilyRepository::softDelete(const QString &uuid) {
    QSqlQuery query(db_);
    query.prepare(QStringLiteral("UPDATE families SET is_deleted=1, updated_at=:now WHERE id=:id"));
    OrmHelper::bindId(query, uuid);
    query.bindValue(QStringLiteral(":now"), utcNow());
    return query.exec();
}

bool FamilyRepository::updateWithVersion(const Family &entity, int expectedVersion) {
    QSqlQuery query(db_);
    query.prepare(QStringLiteral(
        "UPDATE families SET name=:name, invite_code=:invite_code,"
        " version=version+1, updated_at=:updated_at"
        " WHERE id=:id AND version=:version"));
    OrmHelper::bindIdAndVersion(query, entity.id, expectedVersion);
    query.bindValue(QStringLiteral(":name"), entity.name);
    query.bindValue(QStringLiteral(":invite_code"), entity.invite_code);
    query.bindValue(QStringLiteral(":updated_at"), entity.updated_at);
    if (!query.exec()) return false;
    if (query.numRowsAffected() == 0) return false;
    emit familyUpdated(entity.id);
    return true;
}

std::optional<Family> FamilyRepository::getByInviteCode(const QString &code) {
    QSqlQuery query(db_);
    query.prepare(QStringLiteral("SELECT * FROM families WHERE invite_code=:code AND is_deleted=0"));
    query.bindValue(QStringLiteral(":code"), code);
    if (!query.exec() || !query.next()) return std::nullopt;
    return mapRow(query);
}

QList<Family> FamilyRepository::getByCreator(const QString &userId) {
    QList<Family> list;
    QSqlQuery query(db_);
    query.prepare(QStringLiteral(
        "SELECT * FROM families WHERE creator_id=:uid AND is_deleted=0 ORDER BY created_at DESC"));
    query.bindValue(QStringLiteral(":uid"), userId);
    if (!query.exec()) return list;
    while (query.next()) list.append(mapRow(query));
    return list;
}

} // namespace smart_diet
