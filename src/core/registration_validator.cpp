#include "registration_validator.h"

namespace smart_diet {

RegistrationValidator::RegistrationValidator(QObject *parent)
    : QObject(parent)
{
}

QVariantMap RegistrationValidator::validate(const QVariantMap &profile) const
{
    QVariantMap errors;
    bool valid = true;

    // --- 昵称 ---
    const QString nickname = profile.value(QStringLiteral("nickname")).toString().trimmed();
    if (nickname.isEmpty()) {
        errors[QStringLiteral("nickname")] = QStringLiteral("请输入您的昵称");
        valid = false;
    }

    // --- 身高 (cm) ---
    const QVariant h = profile.value(QStringLiteral("height"));
    if (!isNotEmpty(h) || !inRange(h, 50.0, 250.0)) {
        errors[QStringLiteral("height")] = QStringLiteral("请输入有效身高 (50-250 cm)");
        valid = false;
    }

    // --- 体重 (kg) ---
    const QVariant w = profile.value(QStringLiteral("weight"));
    if (!isNotEmpty(w) || !inRange(w, 20.0, 500.0)) {
        errors[QStringLiteral("weight")] = QStringLiteral("请输入有效体重 (20-500 kg)");
        valid = false;
    }

    // --- 年龄 ---
    const QVariant a = profile.value(QStringLiteral("age"));
    if (!isNotEmpty(a) || !inRange(a, 1.0, 150.0)) {
        errors[QStringLiteral("age")] = QStringLiteral("请输入有效年龄 (1-150 岁)");
        valid = false;
    }

    // --- 性别 ---
    const int gender = profile.value(QStringLiteral("gender"), 0).toInt();
    if (gender < 1 || gender > 2) {
        errors[QStringLiteral("gender")] = QStringLiteral("请选择您的性别");
        valid = false;
    }

    // --- 饮食目标 ---
    const int dietGoal = profile.value(QStringLiteral("dietGoal"), -1).toInt();
    if (dietGoal < 0 || dietGoal > 2) {
        errors[QStringLiteral("dietGoal")] = QStringLiteral("请选择您的饮食目标");
        valid = false;
    }

    QVariantMap result;
    result[QStringLiteral("valid")]  = valid;
    result[QStringLiteral("errors")] = errors;
    return result;
}

bool RegistrationValidator::isNotEmpty(const QVariant &v) {
    return v.isValid() && !v.isNull();
}

bool RegistrationValidator::inRange(const QVariant &v, double min, double max) {
    if (!v.isValid() || v.isNull()) return false;
    bool ok = false;
    const double val = v.toDouble(&ok);
    return ok && val >= min && val <= max;
}

bool RegistrationValidator::isPositiveInt(const QVariant &v) {
    if (!v.isValid() || v.isNull()) return false;
    bool ok = false;
    const int val = v.toInt(&ok);
    return ok && val > 0;
}

} // namespace smart_diet
