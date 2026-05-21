#pragma once

#include <QObject>
#include <QVariantMap>
#include <QString>
#include <optional>
#include "base_service.h"
#include "data/user_entity.h"

namespace smart_diet {

class UserRepository;
class RegistrationValidator;
class TdeeCalculator;

/**
 * @brief 认证服务 — 注册、登录、Token 管理
 *
 * 实现 BaseService 的错误/成功信号体系。
 * 持有 UserRepository（通过构造注入）。
 *
 * 注册流程：校验 → 算 BMR/TDEE → 哈希密码 → 保存 User → 发信号
 */
class AuthService : public BaseService {
    Q_OBJECT

public:
    explicit AuthService(UserRepository *userRepo, QObject *parent = nullptr);

    /**
     * @brief 注册新用户
     * @param loginId   登录标识（手机号/邮箱）
     * @param password  明文密码
     * @param profile   表单数据 { nickname, gender, height, weight, age, dietGoal }
     * @return { success: bool, userId: "...", bmr: ..., tdee: ..., goalTdee: ... }
     *
     * 流程：
     *  1. 校验表单
     *  2. 检查 login_id 是否已被占用
     *  3. 计算 BMR / TDEE
     *  4. 哈希密码
     *  5. 构建 User → UserRepository.save()
     *  6. 发信号 userRegistered
     */
    Q_INVOKABLE QVariantMap registerUser(const QString &loginId,
                                          const QString &password,
                                          const QVariantMap &profile);

    /**
     * @brief 登录
     * @param loginId   登录标识
     * @param password  明文密码
     * @return { success: bool, userId: "...", nickname: "..." }
     *
     * 流程：查用户 → 验密码哈希 → 生成 Token → 返回结果
     */
    Q_INVOKABLE QVariantMap login(const QString &loginId,
                                   const QString &password);

    /**
     * @brief 修改密码（需验证旧密码）
     * @param loginId      登录标识
     * @param oldPassword  旧密码（验证身份）
     * @param newPassword  新密码
     * @return { success: bool, error: "..." }
     *
     * 安全约束：必须提供旧密码才能修改，防止未授权篡改。
     * "忘记密码"远程重置留到阶段 8 服务端上线后实现。
     */
    Q_INVOKABLE double getUserTdee(const QString &userId);

    Q_INVOKABLE QVariantMap resetPassword(const QString &loginId,
                                           const QString &oldPassword,
                                           const QString &newPassword);

signals:
    void userRegistered(const QString &userId);
    void userLoggedIn(const QString &userId);

private:
    UserRepository       *userRepo_ = nullptr;
    RegistrationValidator *validator_ = nullptr;
    TdeeCalculator        *tdeeCalc_  = nullptr;
};

} // namespace smart_diet
