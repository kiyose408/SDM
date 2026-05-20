#pragma once

#include <QObject>
#include <QString>

namespace smart_diet {

/**
 * @brief 会话管理器 — 当前登录用户状态
 *
 * 存储登录后用户 ID/昵称，通过 QSettings 持久化。
 * 阶段 2.2.4 将 QSettings 替换为 OS Keychain。
 */
class SessionManager : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString userId READ userId NOTIFY sessionChanged)
    Q_PROPERTY(QString nickname READ nickname NOTIFY sessionChanged)
    Q_PROPERTY(bool loggedIn READ isLoggedIn NOTIFY sessionChanged)

public:
    explicit SessionManager(QObject *parent = nullptr);

    /// 是否已登录
    bool isLoggedIn() const;

    /// 当前用户 ID
    QString userId() const { return userId_; }

    /// 当前用户昵称
    QString nickname() const { return nickname_; }

    /// 登录成功后调用
    Q_INVOKABLE void setSession(const QString &userId, const QString &nickname);

    /// 登出
    Q_INVOKABLE void clearSession();

    /// 从持久化存储恢复会话（启动时调用）
    bool restoreSession();

signals:
    void sessionChanged();
    void loggedIn();
    void loggedOut();

private:
    void saveToSettings();
    void clearSettings();

    QString userId_;
    QString nickname_;
    bool    loggedIn_ = false;
};

} // namespace smart_diet
