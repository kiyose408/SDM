#pragma once

#include <QObject>
#include <QVariantMap>
#include <QString>

namespace smart_diet {

/**
 * @brief 注册表单校验器
 *
 * 校验规则（来源：003.详细设计文档 §2 + 004.数据库设计文档 users 表约束）：
 * - 昵称：必填，非空
 * - 身高：必填，50–250 cm
 * - 体重：必填，20–500 kg
 * - 年龄：必填，1–150 岁
 * - 性别：必须选择（0=未选 → 无效）
 * - 饮食目标：必须选择（-1=未选 → 无效）
 */
class RegistrationValidator : public QObject {
    Q_OBJECT

public:
    explicit RegistrationValidator(QObject *parent = nullptr);

    /**
     * @brief 校验注册表单
     * @param profile 表单字段 { "nickname", "height", "weight", "age", "gender", "dietGoal" }
     * @return { "valid": bool, "errors": { "field": "中文提示" } }
     *
     * QML 用法：
     *   var result = validator.validate({ nickname: "小明", height: 175, ... });
     *   if (!result.valid) console.log(result.errors.nickname);
     */
    Q_INVOKABLE QVariantMap validate(const QVariantMap &profile) const;

private:
    /// 校验必填且非空字符串
    static bool isNotEmpty(const QVariant &v);
    /// 校验数值在 [min, max] 范围内
    static bool inRange(const QVariant &v, double min, double max);
    /// 校验正整数
    static bool isPositiveInt(const QVariant &v);
};

} // namespace smart_diet
