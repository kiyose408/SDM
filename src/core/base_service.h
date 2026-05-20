#pragma once

#include <QObject>
#include <QString>

namespace smart_diet {

/**
 * @brief Service 层抽象基类
 *
 * 所有业务服务类继承此类，提供统一的错误信号和权限校验入口。
 *
 * 设计约束（来自 001.概要设计文档）：
 * - Service 层不直接访问数据库（通过 Repository 接口）
 * - Service 层不直接操作 UI（通过信号通知 QML）
 * - Service 层持有 Repository 指针（依赖注入，不 new）
 */
class BaseService : public QObject {
    Q_OBJECT

public:
    explicit BaseService(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~BaseService() = default;

signals:
    /**
     * @brief 统一错误信号（QML 端统一监听并弹出 Toast/提示）
     * @param message    用户可读的错误描述（中文）
     * @param errorCode  错误码，遵循 005.接口设计文档 错误码体系：
     *                   1000-1999 认证与权限错误
     *                   2000-2999 参数校验错误
     *                   3000-3999 业务逻辑错误
     *                   4000-4999 数据冲突错误
     *                   5000-5999 服务端/内部错误
     */
    void errorOccurred(const QString &message, int errorCode);

    /**
     * @brief 操作完成信号（非错误场景的成功通知）
     * @param message  成功描述（中文）
     */
    void operationCompleted(const QString &message);

protected:
    /**
     * @brief 权限校验辅助方法
     * @param userId       操作用户 UUID
     * @param familyId     目标家庭 UUID
     * @param requiredRole 所需角色（"owner" / "chef" / "member"）
     * @return 是否拥有此权限
     *
     * 角色层级（来自 004.数据库设计文档 v1.6）：
     * - owner  户主，拥有全部权限
     * - chef   掌勺人，可核销库存、管理当日菜单
     * - member 普通成员，仅可浏览和添加心愿
     */
    virtual bool verifyPermission(const QString &userId,
                                   const QString &familyId,
                                   const QString &requiredRole);
};

// 默认实现：基类不做权限校验，派生类按需覆写
inline bool BaseService::verifyPermission(const QString & /*userId*/,
                                           const QString & /*familyId*/,
                                           const QString & /*requiredRole*/) {
    return true;  // 默认放行，派生类覆写实现真实校验
}

} // namespace smart_diet
