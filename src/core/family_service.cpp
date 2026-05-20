#include "family_service.h"
#include "data/family_repository.h"
#include "data/family_member_repository.h"
#include "utils/uuid_utils.h"
#include "utils/time_utils.h"

#include <QRandomGenerator>
#include <QSqlQuery>
#include <QDebug>

namespace smart_diet {

FamilyService::FamilyService(FamilyRepository *familyRepo,
                             FamilyMemberRepository *memberRepo,
                             QObject *parent)
    : BaseService(parent)
    , familyRepo_(familyRepo)
    , memberRepo_(memberRepo)
{
}

QVariantMap FamilyService::createFamily(const QString &creatorId,
                                         const QString &familyName)
{
    QVariantMap result;

    const QString now = utcNow();

    // 创建家庭
    Family family;
    family.id          = generateUuid();
    family.name        = familyName.isEmpty()
                        ? QStringLiteral("我的家")
                        : familyName;
    family.invite_code = generateInviteCode();
    family.creator_id  = creatorId;
    family.created_at  = now;
    family.updated_at  = now;

    if (!familyRepo_->save(family)) {
        result[QStringLiteral("success")] = false;
        result[QStringLiteral("error")]   = QStringLiteral("创建家庭失败");
        return result;
    }

    // 创建者为户主
    FamilyMember member;
    member.id        = generateUuid();
    member.family_id = family.id;
    member.user_id   = creatorId;
    member.role      = QStringLiteral("owner");
    member.joined_at = now;
    member.updated_at = now;

    if (!memberRepo_->addMember(member)) {
        result[QStringLiteral("success")] = false;
        result[QStringLiteral("error")]   = QStringLiteral("添加户主失败");
        return result;
    }

    result[QStringLiteral("success")]    = true;
    result[QStringLiteral("familyId")]   = family.id;
    result[QStringLiteral("inviteCode")] = family.invite_code;

    qDebug() << "[FamilyService] Created family:" << family.id << family.invite_code;
    emit familyCreated(family.id);
    emit operationCompleted(QStringLiteral("家庭创建成功"));
    return result;
}

QVariantMap FamilyService::joinFamily(const QString &userId,
                                       const QString &inviteCode)
{
    QVariantMap result;

    const auto familyOpt = familyRepo_->getByInviteCode(inviteCode);
    if (!familyOpt.has_value()) {
        result[QStringLiteral("success")] = false;
        result[QStringLiteral("error")]   = QStringLiteral("邀请码无效，请核对后重试");
        emit errorOccurred(QStringLiteral("邀请码无效"), 3001);
        return result;
    }

    const Family &family = familyOpt.value();

    // 检查是否已是成员
    if (memberRepo_->getRelation(family.id, userId).has_value()) {
        result[QStringLiteral("success")] = false;
        result[QStringLiteral("error")]   = QStringLiteral("你已经是该家庭的成员");
        return result;
    }

    const QString now = utcNow();

    FamilyMember member;
    member.id        = generateUuid();
    member.family_id = family.id;
    member.user_id   = userId;
    member.role      = QStringLiteral("member");
    member.joined_at = now;
    member.updated_at = now;

    if (!memberRepo_->addMember(member)) {
        result[QStringLiteral("success")] = false;
        result[QStringLiteral("error")]   = QStringLiteral("加入家庭失败");
        return result;
    }

    result[QStringLiteral("success")]  = true;
    result[QStringLiteral("familyId")] = family.id;

    qDebug() << "[FamilyService] User" << userId << "joined family" << family.id;
    emit familyJoined(family.id, userId);
    emit operationCompleted(QStringLiteral("加入家庭成功"));
    return result;
}

QString FamilyService::generateInviteCode() {
    static const QString chars = QStringLiteral("ABCDEFGHJKLMNPQRSTUVWXYZ23456789");
    QString code;
    code.reserve(6);
    for (int i = 0; i < 6; ++i)
        code.append(chars.at(QRandomGenerator::global()->bounded(chars.size())));
    return code;
}

QVariantList FamilyService::getUserFamilies(const QString &userId) {
    QVariantList list;
    const auto members = memberRepo_->getByUser(userId);
    for (const auto &m : members) {
        const auto fam = familyRepo_->getById(m.family_id);
        if (fam.has_value()) {
            QVariantMap item;
            item[QStringLiteral("familyId")]   = fam->id;
            item[QStringLiteral("name")]       = fam->name;
            item[QStringLiteral("role")]       = m.role;
            item[QStringLiteral("inviteCode")] = fam->invite_code;
            list.append(item);
        }
    }
    return list;
}

QVariantList FamilyService::getMembers(const QString &familyId) {
    QVariantList list;
    const auto members = memberRepo_->getByFamily(familyId);
    for (const auto &m : members) {
        QVariantMap item;
        item[QStringLiteral("userId")]              = m.user_id;
        item[QStringLiteral("role")]                = m.role;
        item[QStringLiteral("canManageInventory")]  = m.can_manage_inventory;
        item[QStringLiteral("canConfirmMenu")]      = m.can_confirm_menu;
        item[QStringLiteral("joinedAt")]            = m.joined_at;
        list.append(item);
    }
    return list;
}

bool FamilyService::setMemberPermissions(const QString &familyId,
                                          const QString &userId,
                                          bool canManageInventory,
                                          bool canConfirmMenu) {
    auto memberOpt = memberRepo_->getRelation(familyId, userId);
    if (!memberOpt.has_value()) return false;

    FamilyMember m = memberOpt.value();
    m.can_manage_inventory = canManageInventory;
    m.can_confirm_menu     = canConfirmMenu;
    m.updated_at           = utcNow();

    // 简单用 addMember 的 INSERT OR REPLACE 不适用，需要用 update
    // 直接在 repo 里用 SQL UPDATE
    QSqlQuery query(memberRepo_->database());
    query.prepare(QStringLiteral(
        "UPDATE family_members SET can_manage_inventory=:inv,"
        " can_confirm_menu=:menu, version=version+1, updated_at=:now"
        " WHERE family_id=:fid AND user_id=:uid"));
    query.bindValue(QStringLiteral(":inv"), canManageInventory ? 1 : 0);
    query.bindValue(QStringLiteral(":menu"), canConfirmMenu ? 1 : 0);
    query.bindValue(QStringLiteral(":now"), m.updated_at);
    query.bindValue(QStringLiteral(":fid"), familyId);
    query.bindValue(QStringLiteral(":uid"), userId);
    return query.exec();
}

bool FamilyService::removeMember(const QString &familyId,
                                   const QString &userId,
                                   const QString &operatorId) {
    // 验证操作者是户主
    auto opOpt = memberRepo_->getRelation(familyId, operatorId);
    if (!opOpt.has_value() || opOpt->role != QStringLiteral("owner"))
        return false;
    // 不能移除自己
    if (userId == operatorId) return false;
    return memberRepo_->removeMember(familyId, userId);
}

QVariantMap FamilyService::transferOwnership(const QString &familyId,
                                              const QString &currentOwnerId,
                                              const QString &newOwnerId) {
    QVariantMap result;

    // 验证当前用户是户主
    auto ownerOpt = memberRepo_->getRelation(familyId, currentOwnerId);
    if (!ownerOpt.has_value() || ownerOpt->role != QStringLiteral("owner")) {
        result[QStringLiteral("success")] = false;
        result[QStringLiteral("error")]   = QStringLiteral("仅户主可移交权限");
        return result;
    }

    // 验证新户主是成员
    auto newOwnerOpt = memberRepo_->getRelation(familyId, newOwnerId);
    if (!newOwnerOpt.has_value()) {
        result[QStringLiteral("success")] = false;
        result[QStringLiteral("error")]   = QStringLiteral("该用户不是家庭成员");
        return result;
    }

    const QString now = utcNow();

    // 当前户主降为 member
    QSqlQuery q1(memberRepo_->database());
    q1.prepare(QStringLiteral(
        "UPDATE family_members SET role='member', updated_at=:now"
        " WHERE family_id=:fid AND user_id=:uid"));
    q1.bindValue(QStringLiteral(":now"), now);
    q1.bindValue(QStringLiteral(":fid"), familyId);
    q1.bindValue(QStringLiteral(":uid"), currentOwnerId);
    if (!q1.exec()) {
        result[QStringLiteral("success")] = false;
        result[QStringLiteral("error")]   = QStringLiteral("移交失败");
        return result;
    }

    // 新户主升为 owner
    QSqlQuery q2(memberRepo_->database());
    q2.prepare(QStringLiteral(
        "UPDATE family_members SET role='owner', updated_at=:now"
        " WHERE family_id=:fid AND user_id=:uid"));
    q2.bindValue(QStringLiteral(":now"), now);
    q2.bindValue(QStringLiteral(":fid"), familyId);
    q2.bindValue(QStringLiteral(":uid"), newOwnerId);
    q2.exec();

    result[QStringLiteral("success")] = true;
    emit operationCompleted(QStringLiteral("户主已移交"));
    return result;
}

} // namespace smart_diet
