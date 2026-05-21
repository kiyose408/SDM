#include "menu_repository.h"
#include "database_manager.h"
#include "orm_helper.h"
#include "utils/time_utils.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

namespace smart_diet {

// ================================================================
// DailyMenuRepository
// ================================================================
DailyMenuRepository::DailyMenuRepository(QObject *parent)
    : QObject(parent), db_(DatabaseManager::instance().database()) {}

DailyMenu DailyMenuRepository::mapRow(const QSqlQuery &query) {
    DailyMenu m;
    m.id           = OrmHelper::readString(query, QStringLiteral("id"));
    m.family_id    = OrmHelper::readString(query, QStringLiteral("family_id"));
    m.date         = OrmHelper::readString(query, QStringLiteral("date"));
    m.meal_type    = OrmHelper::readString(query, QStringLiteral("meal_type"));
    m.diner_count  = OrmHelper::readInt(query, QStringLiteral("diner_count"));
    m.diet_mode    = OrmHelper::readString(query, QStringLiteral("diet_mode"));
    m.is_confirmed = OrmHelper::readBool(query, QStringLiteral("is_confirmed"));
    m.confirmed_by = OrmHelper::readString(query, QStringLiteral("confirmed_by"));
    m.confirmed_at = OrmHelper::readString(query, QStringLiteral("confirmed_at"));
    m.chef_id      = OrmHelper::readString(query, QStringLiteral("chef_id"));
    m.is_deleted   = OrmHelper::readBool(query, QStringLiteral("is_deleted"));
    m.version      = OrmHelper::readInt(query, QStringLiteral("version"));
    m.created_at   = OrmHelper::readString(query, QStringLiteral("created_at"));
    m.updated_at   = OrmHelper::readString(query, QStringLiteral("updated_at"));
    return m;
}

std::optional<DailyMenu> DailyMenuRepository::getById(const QString &id) {
    QSqlQuery q(db_);
    q.prepare(QStringLiteral("SELECT * FROM daily_menus WHERE id=:id AND is_deleted=0"));
    OrmHelper::bindId(q, id);
    if (!q.exec() || !q.next()) return std::nullopt;
    return mapRow(q);
}

std::optional<DailyMenu> DailyMenuRepository::getByDate(const QString &familyId, const QString &date, const QString &mealType) {
    QSqlQuery q(db_);
    q.prepare(QStringLiteral("SELECT * FROM daily_menus WHERE family_id=:fid AND date=:d AND meal_type=:mt AND is_deleted=0"));
    q.bindValue(QStringLiteral(":fid"), familyId);
    q.bindValue(QStringLiteral(":d"), date);
    q.bindValue(QStringLiteral(":mt"), mealType);
    if (!q.exec() || !q.next()) return std::nullopt;
    return mapRow(q);
}

QList<DailyMenu> DailyMenuRepository::getByFamilyDate(const QString &familyId, const QString &date) {
    QList<DailyMenu> list;
    QSqlQuery q(db_);
    q.prepare(QStringLiteral("SELECT * FROM daily_menus WHERE family_id=:fid AND date=:d AND is_deleted=0 ORDER BY meal_type"));
    q.bindValue(QStringLiteral(":fid"), familyId);
    q.bindValue(QStringLiteral(":d"), date);
    if (!q.exec()) return list;
    while (q.next()) list.append(mapRow(q));
    return list;
}

bool DailyMenuRepository::save(const DailyMenu &m) {
    QSqlQuery q(db_);
    q.prepare(QStringLiteral("INSERT INTO daily_menus (id,family_id,date,meal_type,diner_count,diet_mode,is_confirmed,confirmed_by,confirmed_at,chef_id,is_deleted,version,created_at,updated_at) VALUES (:id,:fid,:d,:mt,:dc,:dm,:ic,:cb,:ca,:ci,:dl,:v,:cr,:ua)"));
    q.bindValue(QStringLiteral(":id"), m.id); q.bindValue(QStringLiteral(":fid"), m.family_id);
    q.bindValue(QStringLiteral(":d"), m.date); q.bindValue(QStringLiteral(":mt"), m.meal_type);
    q.bindValue(QStringLiteral(":dc"), m.diner_count); q.bindValue(QStringLiteral(":dm"), m.diet_mode);
    q.bindValue(QStringLiteral(":ic"), m.is_confirmed?1:0);
    q.bindValue(QStringLiteral(":cb"), m.confirmed_by.isEmpty()?QVariant():m.confirmed_by);
    q.bindValue(QStringLiteral(":ca"), m.confirmed_at.isEmpty()?QVariant():m.confirmed_at);
    q.bindValue(QStringLiteral(":ci"), m.chef_id.isEmpty()?QVariant():m.chef_id);
    q.bindValue(QStringLiteral(":dl"), m.is_deleted?1:0);
    q.bindValue(QStringLiteral(":v"), m.version); q.bindValue(QStringLiteral(":cr"), m.created_at);
    q.bindValue(QStringLiteral(":ua"), m.updated_at);
    if (!q.exec()) { qWarning()<<"[DailyMenuRepo] save failed:"<<q.lastError().text(); return false; }
    emit menuChanged(m.family_id, m.date); return true;
}

bool DailyMenuRepository::update(const DailyMenu &m) {
    QSqlQuery q(db_);
    q.prepare(QStringLiteral("UPDATE daily_menus SET diner_count=:dc,diet_mode=:dm,is_confirmed=:ic,chef_id=:ci,version=version+1,updated_at=:ua WHERE id=:id"));
    OrmHelper::bindId(q, m.id);
    q.bindValue(QStringLiteral(":dc"), m.diner_count); q.bindValue(QStringLiteral(":dm"), m.diet_mode);
    q.bindValue(QStringLiteral(":ic"), m.is_confirmed?1:0);
    q.bindValue(QStringLiteral(":ci"), m.chef_id.isEmpty()?QVariant():m.chef_id);
    q.bindValue(QStringLiteral(":ua"), m.updated_at);
    return q.exec();
}

// ================================================================
// MenuItemRepository
// ================================================================
MenuItemRepository::MenuItemRepository(QObject *parent)
    : QObject(parent), db_(DatabaseManager::instance().database()) {}

MenuItem MenuItemRepository::mapRow(const QSqlQuery &query) {
    MenuItem mi;
    mi.id                = OrmHelper::readString(query, QStringLiteral("id"));
    mi.menu_id           = OrmHelper::readString(query, QStringLiteral("menu_id"));
    mi.recipe_id         = OrmHelper::readString(query, QStringLiteral("recipe_id"));
    mi.is_locked         = OrmHelper::readBool(query, QStringLiteral("is_locked"));
    mi.servings_override = OrmHelper::readInt(query, QStringLiteral("servings_override"));
    mi.status            = OrmHelper::readString(query, QStringLiteral("status"));
    mi.sort_order        = OrmHelper::readInt(query, QStringLiteral("sort_order"));
    mi.version           = OrmHelper::readInt(query, QStringLiteral("version"));
    mi.created_at        = OrmHelper::readString(query, QStringLiteral("created_at"));
    mi.updated_at        = OrmHelper::readString(query, QStringLiteral("updated_at"));
    return mi;
}

QList<MenuItem> MenuItemRepository::getByMenu(const QString &menuId) {
    QList<MenuItem> list;
    QSqlQuery q(db_);
    q.prepare(QStringLiteral("SELECT * FROM menu_items WHERE menu_id=:mid ORDER BY sort_order"));
    q.bindValue(QStringLiteral(":mid"), menuId);
    if (!q.exec()) return list;
    while (q.next()) list.append(mapRow(q));
    return list;
}

bool MenuItemRepository::addItem(const MenuItem &item) {
    QSqlQuery q(db_);
    q.prepare(QStringLiteral("INSERT OR IGNORE INTO menu_items (id,menu_id,recipe_id,is_locked,servings_override,status,sort_order,version,created_at,updated_at) VALUES (:id,:mid,:rid,:il,:so,:st,:sr,:v,:cr,:ua)"));
    q.bindValue(QStringLiteral(":id"), item.id); q.bindValue(QStringLiteral(":mid"), item.menu_id);
    q.bindValue(QStringLiteral(":rid"), item.recipe_id); q.bindValue(QStringLiteral(":il"), item.is_locked?1:0);
    q.bindValue(QStringLiteral(":so"), item.servings_override); q.bindValue(QStringLiteral(":st"), item.status);
    q.bindValue(QStringLiteral(":sr"), item.sort_order); q.bindValue(QStringLiteral(":v"), item.version);
    q.bindValue(QStringLiteral(":cr"), item.created_at); q.bindValue(QStringLiteral(":ua"), item.updated_at);
    return q.exec();
}

bool MenuItemRepository::removeItem(const QString &menuId, const QString &recipeId) {
    QSqlQuery q(db_);
    q.prepare(QStringLiteral("DELETE FROM menu_items WHERE menu_id=:mid AND recipe_id=:rid"));
    q.bindValue(QStringLiteral(":mid"), menuId);
    q.bindValue(QStringLiteral(":rid"), recipeId);
    return q.exec();
}

bool MenuItemRepository::updateStatus(const QString &itemId, const QString &status) {
    QSqlQuery q(db_);
    q.prepare(QStringLiteral("UPDATE menu_items SET status=:st,updated_at=:ua WHERE id=:id"));
    q.bindValue(QStringLiteral(":st"), status);
    q.bindValue(QStringLiteral(":ua"), utcNow());
    OrmHelper::bindId(q, itemId);
    return q.exec();
}

bool MenuItemRepository::clearMenu(const QString &menuId) {
    QSqlQuery q(db_);
    q.prepare(QStringLiteral("DELETE FROM menu_items WHERE menu_id=:mid"));
    q.bindValue(QStringLiteral(":mid"), menuId);
    return q.exec();
}

bool MenuItemRepository::clearUnlocked(const QString &menuId) {
    QSqlQuery q(db_);
    q.prepare(QStringLiteral("DELETE FROM menu_items WHERE menu_id=:mid AND is_locked=0"));
    q.bindValue(QStringLiteral(":mid"), menuId);
    return q.exec();
}

} // namespace smart_diet
