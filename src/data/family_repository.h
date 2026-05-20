#pragma once

#include <QObject>
#include <QSqlDatabase>
#include <QList>
#include <optional>

#include "base_repository.h"
#include "family_entity.h"

namespace smart_diet {

class FamilyRepository : public QObject, public BaseRepository<Family> {
    Q_OBJECT

public:
    explicit FamilyRepository(QObject *parent = nullptr);

    std::optional<Family> getById(const QString &uuid) override;
    QList<Family> getAll() override;
    bool save(const Family &entity) override;
    bool update(const Family &entity) override;
    bool softDelete(const QString &uuid) override;
    bool updateWithVersion(const Family &entity, int expectedVersion) override;

    /// 按邀请码查找
    std::optional<Family> getByInviteCode(const QString &code);
    /// 按创建者查找
    QList<Family> getByCreator(const QString &userId);

signals:
    void familyInserted(const QString &familyId);
    void familyUpdated(const QString &familyId);

private:
    static Family mapRow(const QSqlQuery &query);
    QSqlDatabase db_;
};

} // namespace smart_diet
