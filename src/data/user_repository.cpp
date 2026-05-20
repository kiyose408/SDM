#include "user_repository.h"
#include "database_manager.h"
#include "orm_helper.h"
#include "utils/uuid_utils.h"
#include "utils/time_utils.h"

#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>

namespace smart_diet {

// ============================================================================
// 构造
// ============================================================================

UserRepository::UserRepository(QObject *parent)
    : QObject(parent)
    , db_(DatabaseManager::instance().database())
{
}

// ============================================================================
// 行映射
// ============================================================================

User UserRepository::mapRow(const QSqlQuery &query) {
    User u;
    u.id            = OrmHelper::readString(query, QStringLiteral("id"));
    u.login_id      = OrmHelper::readString(query, QStringLiteral("login_id"));
    u.nickname      = OrmHelper::readString(query, QStringLiteral("nickname"));
    u.avatar_url    = OrmHelper::readString(query, QStringLiteral("avatar_url"));
    u.gender        = OrmHelper::readInt(query, QStringLiteral("gender"));
    u.height        = OrmHelper::readReal(query, QStringLiteral("height"));
    u.weight        = OrmHelper::readReal(query, QStringLiteral("weight"));
    u.birth_date    = OrmHelper::readString(query, QStringLiteral("birth_date"));
    u.diet_goal     = OrmHelper::readString(query, QStringLiteral("diet_goal"));
    u.bmr           = OrmHelper::readReal(query, QStringLiteral("bmr"));
    u.tdee          = OrmHelper::readReal(query, QStringLiteral("tdee"));
    u.password_hash = OrmHelper::readString(query, QStringLiteral("password_hash"));
    u.is_deleted    = OrmHelper::readBool(query, QStringLiteral("is_deleted"));
    u.version       = OrmHelper::readInt(query, QStringLiteral("version"));
    u.created_at    = OrmHelper::readString(query, QStringLiteral("created_at"));
    u.updated_at    = OrmHelper::readString(query, QStringLiteral("updated_at"));
    return u;
}

// ============================================================================
// CRUD
// ============================================================================

std::optional<User> UserRepository::getById(const QString &uuid) {
    QSqlQuery query(db_);
    query.prepare(QStringLiteral(
        "SELECT * FROM users WHERE id = :id AND is_deleted = 0"));
    OrmHelper::bindId(query, uuid);

    if (!query.exec()) {
        qWarning() << "[UserRepository] getById failed:" << query.lastError().text();
        return std::nullopt;
    }
    if (!query.next()) return std::nullopt;
    return mapRow(query);
}

QList<User> UserRepository::getAll() {
    QList<User> list;
    QSqlQuery query(db_);
    query.prepare(QStringLiteral(
        "SELECT * FROM users WHERE is_deleted = 0 ORDER BY created_at DESC"));

    if (!query.exec()) {
        qWarning() << "[UserRepository] getAll failed:" << query.lastError().text();
        return list;
    }
    while (query.next()) {
        list.append(mapRow(query));
    }
    return list;
}

bool UserRepository::save(const User &entity) {
    QSqlQuery query(db_);
    query.prepare(QStringLiteral(
        "INSERT INTO users ("
        "  id, login_id, nickname, avatar_url, gender,"
        "  height, weight, birth_date, diet_goal,"
        "  bmr, tdee, password_hash,"
        "  is_deleted, version, created_at, updated_at"
        ") VALUES ("
        "  :id, :login_id, :nickname, :avatar_url, :gender,"
        "  :height, :weight, :birth_date, :diet_goal,"
        "  :bmr, :tdee, :password_hash,"
        "  :is_deleted, :version, :created_at, :updated_at"
        ")"));

    query.bindValue(QStringLiteral(":id"),            entity.id);
    query.bindValue(QStringLiteral(":login_id"),      entity.login_id);
    query.bindValue(QStringLiteral(":nickname"),      entity.nickname);
    query.bindValue(QStringLiteral(":avatar_url"),    entity.avatar_url.isEmpty() ? QVariant() : entity.avatar_url);
    query.bindValue(QStringLiteral(":gender"),        entity.gender);
    query.bindValue(QStringLiteral(":height"),        entity.height > 0 ? entity.height : QVariant());
    query.bindValue(QStringLiteral(":weight"),        entity.weight > 0 ? entity.weight : QVariant());
    query.bindValue(QStringLiteral(":birth_date"),    entity.birth_date.isEmpty() ? QVariant() : entity.birth_date);
    query.bindValue(QStringLiteral(":diet_goal"),     entity.diet_goal);
    query.bindValue(QStringLiteral(":bmr"),           entity.bmr > 0 ? entity.bmr : QVariant());
    query.bindValue(QStringLiteral(":tdee"),          entity.tdee > 0 ? entity.tdee : QVariant());
    query.bindValue(QStringLiteral(":password_hash"), entity.password_hash);
    query.bindValue(QStringLiteral(":is_deleted"),    entity.is_deleted ? 1 : 0);
    query.bindValue(QStringLiteral(":version"),       entity.version);
    query.bindValue(QStringLiteral(":created_at"),    entity.created_at);
    query.bindValue(QStringLiteral(":updated_at"),    entity.updated_at);

    if (!query.exec()) {
        qWarning() << "[UserRepository] save failed:" << query.lastError().text();
        return false;
    }
    emit userInserted(entity.id);
    return true;
}

bool UserRepository::update(const User &entity) {
    QSqlQuery query(db_);
    query.prepare(QStringLiteral(
        "UPDATE users SET"
        "  nickname   = :nickname,"
        "  avatar_url = :avatar_url,"
        "  gender     = :gender,"
        "  height     = :height,"
        "  weight     = :weight,"
        "  birth_date = :birth_date,"
        "  diet_goal  = :diet_goal,"
        "  bmr        = :bmr,"
        "  tdee       = :tdee,"
        "  password_hash = :password_hash,"
        "  version    = version + 1,"
        "  updated_at = :updated_at"
        " WHERE id = :id"));

    OrmHelper::bindId(query, entity.id);
    query.bindValue(QStringLiteral(":nickname"),      entity.nickname);
    query.bindValue(QStringLiteral(":avatar_url"),    entity.avatar_url.isEmpty() ? QVariant() : entity.avatar_url);
    query.bindValue(QStringLiteral(":gender"),        entity.gender);
    query.bindValue(QStringLiteral(":height"),        entity.height > 0 ? entity.height : QVariant());
    query.bindValue(QStringLiteral(":weight"),        entity.weight > 0 ? entity.weight : QVariant());
    query.bindValue(QStringLiteral(":birth_date"),    entity.birth_date.isEmpty() ? QVariant() : entity.birth_date);
    query.bindValue(QStringLiteral(":diet_goal"),     entity.diet_goal);
    query.bindValue(QStringLiteral(":bmr"),           entity.bmr > 0 ? entity.bmr : QVariant());
    query.bindValue(QStringLiteral(":tdee"),          entity.tdee > 0 ? entity.tdee : QVariant());
    query.bindValue(QStringLiteral(":password_hash"), entity.password_hash);
    query.bindValue(QStringLiteral(":updated_at"),    entity.updated_at);

    if (!query.exec()) {
        qWarning() << "[UserRepository] update failed:" << query.lastError().text();
        return false;
    }
    emit userUpdated(entity.id);
    return true;
}

bool UserRepository::softDelete(const QString &uuid) {
    QSqlQuery query(db_);
    query.prepare(QStringLiteral(
        "UPDATE users SET is_deleted = 1, updated_at = :now WHERE id = :id"));
    OrmHelper::bindId(query, uuid);
    query.bindValue(QStringLiteral(":now"), utcNow());

    if (!query.exec()) {
        qWarning() << "[UserRepository] softDelete failed:" << query.lastError().text();
        return false;
    }
    emit userDeleted(uuid);
    return true;
}

bool UserRepository::updateWithVersion(const User &entity, int expectedVersion) {
    QSqlQuery query(db_);
    query.prepare(QStringLiteral(
        "UPDATE users SET"
        "  nickname   = :nickname,"
        "  avatar_url = :avatar_url,"
        "  gender     = :gender,"
        "  height     = :height,"
        "  weight     = :weight,"
        "  birth_date = :birth_date,"
        "  diet_goal  = :diet_goal,"
        "  bmr        = :bmr,"
        "  tdee       = :tdee,"
        "  password_hash = :password_hash,"
        "  version    = version + 1,"
        "  updated_at = :updated_at"
        " WHERE id = :id AND version = :version"));

    OrmHelper::bindIdAndVersion(query, entity.id, expectedVersion);
    query.bindValue(QStringLiteral(":nickname"),      entity.nickname);
    query.bindValue(QStringLiteral(":avatar_url"),    entity.avatar_url.isEmpty() ? QVariant() : entity.avatar_url);
    query.bindValue(QStringLiteral(":gender"),        entity.gender);
    query.bindValue(QStringLiteral(":height"),        entity.height > 0 ? entity.height : QVariant());
    query.bindValue(QStringLiteral(":weight"),        entity.weight > 0 ? entity.weight : QVariant());
    query.bindValue(QStringLiteral(":birth_date"),    entity.birth_date.isEmpty() ? QVariant() : entity.birth_date);
    query.bindValue(QStringLiteral(":diet_goal"),     entity.diet_goal);
    query.bindValue(QStringLiteral(":bmr"),           entity.bmr > 0 ? entity.bmr : QVariant());
    query.bindValue(QStringLiteral(":tdee"),          entity.tdee > 0 ? entity.tdee : QVariant());
    query.bindValue(QStringLiteral(":password_hash"), entity.password_hash);
    query.bindValue(QStringLiteral(":updated_at"),    entity.updated_at);

    if (!query.exec()) {
        qWarning() << "[UserRepository] updateWithVersion failed:" << query.lastError().text();
        return false;
    }

    // version 不匹配 → affected rows = 0 → 冲突
    if (query.numRowsAffected() == 0) {
        qDebug() << "[UserRepository] Version conflict on" << entity.id;
        return false;
    }

    emit userUpdated(entity.id);
    return true;
}

// ============================================================================
// User 特有
// ============================================================================

std::optional<User> UserRepository::getByLoginId(const QString &loginId) {
    QSqlQuery query(db_);
    query.prepare(QStringLiteral(
        "SELECT * FROM users WHERE login_id = :login_id AND is_deleted = 0"));
    query.bindValue(QStringLiteral(":login_id"), loginId);

    if (!query.exec()) {
        qWarning() << "[UserRepository] getByLoginId failed:" << query.lastError().text();
        return std::nullopt;
    }
    if (!query.next()) return std::nullopt;
    return mapRow(query);
}

bool UserRepository::existsByLoginId(const QString &loginId) {
    QSqlQuery query(db_);
    query.prepare(QStringLiteral(
        "SELECT 1 FROM users WHERE login_id = :login_id AND is_deleted = 0"));
    query.bindValue(QStringLiteral(":login_id"), loginId);

    if (!query.exec()) {
        qWarning() << "[UserRepository] existsByLoginId failed:" << query.lastError().text();
        return false;
    }
    return query.next();
}

} // namespace smart_diet
