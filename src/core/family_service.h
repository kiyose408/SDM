#pragma once

#include <QObject>
#include <QVariantMap>
#include <QString>
#include "base_service.h"

namespace smart_diet {

class FamilyRepository;
class FamilyMemberRepository;

class FamilyService : public BaseService {
    Q_OBJECT

public:
    explicit FamilyService(FamilyRepository *familyRepo,
                           FamilyMemberRepository *memberRepo,
                           QObject *parent = nullptr);

    /**
     * @brief 创建新家庭
     * @param creatorId  创建者 UUID
     * @param familyName 家庭名称（空则用默认）
     * @return { success, familyId, inviteCode }
     */
    Q_INVOKABLE QVariantMap createFamily(const QString &creatorId,
                                          const QString &familyName = {});

    /**
     * @brief 通过邀请码加入家庭
     * @param userId     加入者 UUID
     * @param inviteCode 6 位邀请码
     * @return { success, familyId, error }
     */
    Q_INVOKABLE QVariantMap joinFamily(const QString &userId,
                                        const QString &inviteCode);

signals:
    void familyCreated(const QString &familyId);
    void familyJoined(const QString &familyId, const QString &userId);

private:
    static QString generateInviteCode();

    FamilyRepository       *familyRepo_ = nullptr;
    FamilyMemberRepository *memberRepo_ = nullptr;
};

} // namespace smart_diet
