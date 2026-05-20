#include "family_service.h"
#include "data/family_repository.h"
#include "data/family_member_repository.h"
#include "utils/uuid_utils.h"
#include "utils/time_utils.h"

#include <QRandomGenerator>
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

} // namespace smart_diet
