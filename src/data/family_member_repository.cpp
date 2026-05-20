#include "family_member_repository.h"
#include "database_manager.h"
#include "orm_helper.h"
#include "utils/time_utils.h"

#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>

namespace smart_diet {

FamilyMemberRepository::FamilyMemberRepository(QObject *parent)
    : QObject(parent)
    , db_(DatabaseManager::instance().database())
{
}

FamilyMember FamilyMemberRepository::mapRow(const QSqlQuery &query) {
    FamilyMember m;
    m.id                    = OrmHelper::readString(query, QStringLiteral("id"));
    m.family_id             = OrmHelper::readString(query, QStringLiteral("family_id"));
    m.user_id               = OrmHelper::readString(query, QStringLiteral("user_id"));
    m.role                  = OrmHelper::readString(query, QStringLiteral("role"));
    m.can_manage_inventory  = OrmHelper::readBool(query, QStringLiteral("can_manage_inventory"));
    m.can_confirm_menu      = OrmHelper::readBool(query, QStringLiteral("can_confirm_menu"));
    m.joined_at             = OrmHelper::readString(query, QStringLiteral("joined_at"));
    m.version               = OrmHelper::readInt(query, QStringLiteral("version"));
    m.updated_at            = OrmHelper::readString(query, QStringLiteral("updated_at"));
    return m;
}

bool FamilyMemberRepository::addMember(const FamilyMember &member) {
    QSqlQuery query(db_);
    query.prepare(QStringLiteral(
        "INSERT INTO family_members (id, family_id, user_id, role, can_manage_inventory,"
        " can_confirm_menu, joined_at, version, updated_at)"
        " VALUES (:id, :family_id, :user_id, :role, :can_manage_inventory,"
        " :can_confirm_menu, :joined_at, :version, :updated_at)"));
    query.bindValue(QStringLiteral(":id"), member.id);
    query.bindValue(QStringLiteral(":family_id"), member.family_id);
    query.bindValue(QStringLiteral(":user_id"), member.user_id);
    query.bindValue(QStringLiteral(":role"), member.role);
    query.bindValue(QStringLiteral(":can_manage_inventory"), member.can_manage_inventory ? 1 : 0);
    query.bindValue(QStringLiteral(":can_confirm_menu"), member.can_confirm_menu ? 1 : 0);
    query.bindValue(QStringLiteral(":joined_at"), member.joined_at);
    query.bindValue(QStringLiteral(":version"), member.version);
    query.bindValue(QStringLiteral(":updated_at"), member.updated_at);
    if (!query.exec()) {
        qWarning() << "[FamilyMemberRepo] addMember failed:" << query.lastError().text();
        return false;
    }
    emit memberAdded(member.family_id, member.user_id);
    return true;
}

bool FamilyMemberRepository::removeMember(const QString &familyId, const QString &userId) {
    QSqlQuery query(db_);
    query.prepare(QStringLiteral(
        "DELETE FROM family_members WHERE family_id=:fid AND user_id=:uid"));
    query.bindValue(QStringLiteral(":fid"), familyId);
    query.bindValue(QStringLiteral(":uid"), userId);
    if (!query.exec()) return false;
    emit memberRemoved(familyId, userId);
    return true;
}

QList<FamilyMember> FamilyMemberRepository::getByFamily(const QString &familyId) {
    QList<FamilyMember> list;
    QSqlQuery query(db_);
    query.prepare(QStringLiteral(
        "SELECT * FROM family_members WHERE family_id=:fid ORDER BY joined_at ASC"));
    query.bindValue(QStringLiteral(":fid"), familyId);
    if (!query.exec()) return list;
    while (query.next()) list.append(mapRow(query));
    return list;
}

QList<FamilyMember> FamilyMemberRepository::getByUser(const QString &userId) {
    QList<FamilyMember> list;
    QSqlQuery query(db_);
    query.prepare(QStringLiteral(
        "SELECT * FROM family_members WHERE user_id=:uid ORDER BY joined_at DESC"));
    query.bindValue(QStringLiteral(":uid"), userId);
    if (!query.exec()) return list;
    while (query.next()) list.append(mapRow(query));
    return list;
}

std::optional<FamilyMember> FamilyMemberRepository::getRelation(
    const QString &familyId, const QString &userId) {
    QSqlQuery query(db_);
    query.prepare(QStringLiteral(
        "SELECT * FROM family_members WHERE family_id=:fid AND user_id=:uid"));
    query.bindValue(QStringLiteral(":fid"), familyId);
    query.bindValue(QStringLiteral(":uid"), userId);
    if (!query.exec() || !query.next()) return std::nullopt;
    return mapRow(query);
}

} // namespace smart_diet
