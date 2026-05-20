#pragma once

#include <QObject>
#include <QSqlDatabase>
#include <QList>
#include <optional>
#include "family_entity.h"

namespace smart_diet {

class FamilyMemberRepository : public QObject {
    Q_OBJECT

public:
    explicit FamilyMemberRepository(QObject *parent = nullptr);

    /// 添加成员到家庭
    bool addMember(const FamilyMember &member);
    /// 移除成员
    bool removeMember(const QString &familyId, const QString &userId);
    /// 获取家庭所有成员
    QList<FamilyMember> getByFamily(const QString &familyId);
    /// 获取用户所属家庭列表
    QList<FamilyMember> getByUser(const QString &userId);
    /// 查找特定关系
    std::optional<FamilyMember> getRelation(const QString &familyId, const QString &userId);
    /// 数据库句柄（供 Service 层直接执行 SQL）
    QSqlDatabase database() const { return db_; }

signals:
    void memberAdded(const QString &familyId, const QString &userId);
    void memberRemoved(const QString &familyId, const QString &userId);

private:
    static FamilyMember mapRow(const QSqlQuery &query);
    QSqlDatabase db_;
};

} // namespace smart_diet
