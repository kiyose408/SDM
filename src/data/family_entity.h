#pragma once

#include <QString>

namespace smart_diet {

/**
 * @brief 家庭实体（对应 families 表）
 *
 * 字段定义与 004.数据库设计文档 §4.2 严格一致。
 */
struct Family {
    QString id;
    QString name;                    // 默认 "{创建者昵称}的家"
    QString invite_code;             // 6 位字母数字
    QString invite_code_expires_at;  // NULL = 永不过期
    QString creator_id;              // FK → users.id
    bool    is_deleted = false;
    int     version    = 1;
    QString created_at;
    QString updated_at;
};

/**
 * @brief 家庭成员实体（对应 family_members 表）
 *
 * 字段定义与 004.数据库设计文档 §4.3 严格一致。
 */
struct FamilyMember {
    QString id;
    QString family_id;               // FK → families.id
    QString user_id;                  // FK → users.id
    QString role = QStringLiteral("member");  // owner / member
    bool    can_manage_inventory = false;
    bool    can_confirm_menu     = false;
    QString joined_at;
    int     version    = 1;
    QString updated_at;
};

} // namespace smart_diet
