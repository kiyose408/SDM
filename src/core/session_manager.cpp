#include "session_manager.h"

#include <QSettings>
#include <QDebug>

namespace smart_diet {

SessionManager::SessionManager(QObject *parent)
    : QObject(parent)
{
}

bool SessionManager::isLoggedIn() const {
    return loggedIn_ && !userId_.isEmpty();
}

void SessionManager::setSession(const QString &userId, const QString &nickname) {
    userId_   = userId;
    nickname_ = nickname;
    loggedIn_ = true;
    saveToSettings();
    qDebug() << "[SessionManager] Session set:" << userId << nickname;
    emit sessionChanged();
    emit loggedIn();
}

void SessionManager::clearSession() {
    userId_   = QString();
    nickname_ = QString();
    loggedIn_ = false;
    clearSettings();
    qDebug() << "[SessionManager] Session cleared";
    emit sessionChanged();
    emit loggedOut();
}

bool SessionManager::restoreSession() {
    QSettings settings(QStringLiteral("SmartDiet"), QStringLiteral("Session"));
    const QString uid  = settings.value(QStringLiteral("userId")).toString();
    const QString nick = settings.value(QStringLiteral("nickname")).toString();

    if (uid.isEmpty()) {
        qDebug() << "[SessionManager] No saved session";
        return false;
    }

    userId_   = uid;
    nickname_ = nick;
    loggedIn_ = true;
    qDebug() << "[SessionManager] Session restored:" << uid << nick;
    emit sessionChanged();
    return true;
}

void SessionManager::saveToSettings() {
    QSettings settings(QStringLiteral("SmartDiet"), QStringLiteral("Session"));
    settings.setValue(QStringLiteral("userId"),   userId_);
    settings.setValue(QStringLiteral("nickname"), nickname_);
}

void SessionManager::clearSettings() {
    QSettings settings(QStringLiteral("SmartDiet"), QStringLiteral("Session"));
    settings.remove(QStringLiteral("userId"));
    settings.remove(QStringLiteral("nickname"));
}

} // namespace smart_diet
