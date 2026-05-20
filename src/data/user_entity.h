#pragma once

#include <QString>

namespace smart_diet {

/**
 * @brief 用户实体（纯数据结构，对应 users 表）
 *
 * 字段定义与 004.数据库设计文档 §4.1 严格一致。
 * 实体不继承 QObject —— 它是纯数据载体，信号/槽由 Repository 层负责。
 */
struct User {
    QString id;
    QString login_id;         // 登录标识（手机号/邮箱/用户名）
    QString nickname;         // 昵称
    QString avatar_url;       // 头像路径/URL
    int     gender      = 0;  // 0=未设置, 1=男, 2=女
    double  height      = 0.0;
    double  weight      = 0.0;
    QString birth_date;       // YYYY-MM-DD
    QString diet_goal   = QStringLiteral("maintenance"); // weight_loss / muscle_gain / maintenance
    double  bmr         = 0.0;
    double  tdee        = 0.0;
    QString password_hash;    // Argon2id 哈希
    bool    is_deleted  = false;
    int     version     = 1;
    QString created_at;
    QString updated_at;
};

} // namespace smart_diet
