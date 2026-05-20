#include "auth_service.h"
#include "registration_validator.h"
#include "tdee_calculator.h"
#include "data/user_repository.h"
#include "data/database_manager.h"
#include "utils/uuid_utils.h"
#include "utils/time_utils.h"

#include "utils/argon2_hasher.h"
#include <QCryptographicHash>
#include <QDebug>

namespace smart_diet {

AuthService::AuthService(UserRepository *userRepo, QObject *parent)
    : BaseService(parent)
    , userRepo_(userRepo)
    , validator_(new RegistrationValidator(this))
    , tdeeCalc_(new TdeeCalculator(this))
{
}

QVariantMap AuthService::registerUser(const QString &loginId,
                                       const QString &password,
                                       const QVariantMap &profile)
{
    QVariantMap result;

    // --- 1. 校验 ---
    const QVariantMap validation = validator_->validate(profile);
    if (!validation.value(QStringLiteral("valid")).toBool()) {
        result[QStringLiteral("success")] = false;
        result[QStringLiteral("error")]   = QStringLiteral("表单校验未通过");
        result[QStringLiteral("errors")]  = validation.value(QStringLiteral("errors"));
        emit errorOccurred(QStringLiteral("表单校验未通过"), 2001);
        return result;
    }

    // --- 2. 检查 login_id 唯一性 ---
    if (userRepo_->existsByLoginId(loginId)) {
        result[QStringLiteral("success")] = false;
        result[QStringLiteral("error")]   = QStringLiteral("该账号已被注册");
        emit errorOccurred(QStringLiteral("该账号已被注册"), 4002);
        return result;
    }

    // --- 3. 计算 BMR / TDEE ---
    const int    gender    = profile.value(QStringLiteral("gender")).toInt();
    const double weight    = profile.value(QStringLiteral("weight")).toDouble();
    const double heightCm  = profile.value(QStringLiteral("height")).toDouble();
    const int    age       = profile.value(QStringLiteral("age")).toInt();
    const int    dietGoal  = profile.value(QStringLiteral("dietGoal")).toInt();

    const double bmr  = tdeeCalc_->calculateBmr(gender, weight, heightCm, age);
    const double tdee = tdeeCalc_->calculateTdee(bmr);
    const double goalTdee = tdeeCalc_->applyDietGoal(tdee, dietGoal);

    // --- 4. 构建 User ---
    User user;
    user.id            = generateUuid();
    user.login_id      = loginId;
    user.nickname      = profile.value(QStringLiteral("nickname")).toString().trimmed();
    user.gender        = gender;
    user.height        = heightCm;
    user.weight        = weight;
    user.birth_date    = QString();  // 未采集出生日期，留空
    switch (dietGoal) {
        case 0: user.diet_goal = QStringLiteral("weight_loss"); break;
        case 1: user.diet_goal = QStringLiteral("muscle_gain"); break;
        default: user.diet_goal = QStringLiteral("maintenance"); break;
    }
    user.bmr           = bmr;
    user.tdee          = goalTdee;
    user.password_hash = Argon2Hasher::hash(password);
    user.created_at    = utcNow();
    user.updated_at    = user.created_at;

    // --- 5. 保存 ---
    if (!userRepo_->save(user)) {
        result[QStringLiteral("success")] = false;
        result[QStringLiteral("error")]   = QStringLiteral("保存用户失败");
        emit errorOccurred(QStringLiteral("保存用户失败"), 5001);
        return result;
    }

    // --- 6. 成功 ---
    result[QStringLiteral("success")]   = true;
    result[QStringLiteral("userId")]    = user.id;
    result[QStringLiteral("bmr")]       = bmr;
    result[QStringLiteral("tdee")]      = tdee;
    result[QStringLiteral("goalTdee")]  = goalTdee;

    qDebug() << "[AuthService] User registered:" << user.id << user.nickname;
    emit userRegistered(user.id);
    emit operationCompleted(QStringLiteral("注册成功"));
    return result;
}

QVariantMap AuthService::login(const QString &loginId,
                                  const QString &password)
{
    QVariantMap result;

    // --- 1. 查找用户 ---
    const auto userOpt = userRepo_->getByLoginId(loginId);
    if (!userOpt.has_value()) {
        result[QStringLiteral("success")] = false;
        result[QStringLiteral("error")]   = QStringLiteral("账号不存在");
        emit errorOccurred(QStringLiteral("账号不存在"), 1001);
        return result;
    }

    const User &user = userOpt.value();

    // --- 2. 验证密码 ---
    bool passwordOk = false;

    // 兼容旧 SHA-256 密码（$argon2id$ 前缀即新格式）
    if (user.password_hash.startsWith(QStringLiteral("$argon2id$"))) {
        passwordOk = Argon2Hasher::verify(password, user.password_hash);
    } else {
        // 旧格式：SHA-256 hex 字符串
        const QString oldHash = QString::fromUtf8(
            QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex());
        passwordOk = (oldHash == user.password_hash);
        if (passwordOk) {
            // 自动升级为 Argon2id
            User updated = user;
            updated.password_hash = Argon2Hasher::hash(password);
            userRepo_->update(updated);
            qDebug() << "[AuthService] Upgraded password to Argon2id for:" << user.id;
        }
    }

    if (!passwordOk) {
        result[QStringLiteral("success")] = false;
        result[QStringLiteral("error")]   = QStringLiteral("密码错误");
        emit errorOccurred(QStringLiteral("密码错误"), 1001);
        return result;
    }

    // --- 3. 成功 ---
    result[QStringLiteral("success")]  = true;
    result[QStringLiteral("userId")]   = user.id;
    result[QStringLiteral("nickname")] = user.nickname;

    qDebug() << "[AuthService] User logged in:" << user.id << user.nickname;
    emit userLoggedIn(user.id);
    emit operationCompleted(QStringLiteral("登录成功"));
    return result;
}

} // namespace smart_diet
