#pragma once

#include <QObject>
#include <QSqlDatabase>
#include <QList>
#include <optional>

#include "base_repository.h"
#include "user_entity.h"

namespace smart_diet {

/**
 * @brief 用户数据访问层
 *
 * 实现 BaseRepository<User> 的全部 CRUD 接口，
 * 额外提供 login_id 查询（用于登录认证）。
 *
 * 多重继承：QObject（元对象系统）+ BaseRepository<User>（接口契约）
 */
class UserRepository : public QObject, public BaseRepository<User> {
    Q_OBJECT

public:
    explicit UserRepository(QObject *parent = nullptr);

    // ===== BaseRepository<User> 接口实现 =====
    std::optional<User> getById(const QString &uuid) override;
    QList<User> getAll() override;
    bool save(const User &entity) override;
    bool update(const User &entity) override;
    bool softDelete(const QString &uuid) override;
    bool updateWithVersion(const User &entity, int expectedVersion) override;

    // ===== User 特有查询 =====
    /// 按 login_id 查找（登录用）
    std::optional<User> getByLoginId(const QString &loginId);

    /// 检查 login_id 是否已被占用
    bool existsByLoginId(const QString &loginId);

signals:
    void userInserted(const QString &userId);
    void userUpdated(const QString &userId);
    void userDeleted(const QString &userId);

private:
    /// 从 QSqlQuery 当前行映射为 User 实体
    static User mapRow(const QSqlQuery &query);

    QSqlDatabase db_;
};

} // namespace smart_diet
