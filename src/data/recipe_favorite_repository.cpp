#include "recipe_favorite_repository.h"
#include "database_manager.h"
#include "orm_helper.h"
#include "utils/uuid_utils.h"
#include "utils/time_utils.h"
#include <QSqlQuery>

namespace smart_diet {

RecipeFavoriteRepository::RecipeFavoriteRepository(QObject *parent)
    : QObject(parent), db_(DatabaseManager::instance().database()) {}

bool RecipeFavoriteRepository::addFavorite(const QString &userId, const QString &recipeId) {
    QSqlQuery q(db_);
    q.prepare(QStringLiteral("INSERT OR IGNORE INTO recipe_favorites (id,user_id,recipe_id,created_at) VALUES (:id,:uid,:rid,:ca)"));
    q.bindValue(QStringLiteral(":id"), generateUuid());
    q.bindValue(QStringLiteral(":uid"), userId);
    q.bindValue(QStringLiteral(":rid"), recipeId);
    q.bindValue(QStringLiteral(":ca"), utcNow());
    return q.exec();
}

bool RecipeFavoriteRepository::removeFavorite(const QString &userId, const QString &recipeId) {
    QSqlQuery q(db_);
    q.prepare(QStringLiteral("DELETE FROM recipe_favorites WHERE user_id=:uid AND recipe_id=:rid"));
    q.bindValue(QStringLiteral(":uid"), userId);
    q.bindValue(QStringLiteral(":rid"), recipeId);
    return q.exec();
}

bool RecipeFavoriteRepository::isFavorited(const QString &userId, const QString &recipeId) {
    QSqlQuery q(db_);
    q.prepare(QStringLiteral("SELECT 1 FROM recipe_favorites WHERE user_id=:uid AND recipe_id=:rid"));
    q.bindValue(QStringLiteral(":uid"), userId);
    q.bindValue(QStringLiteral(":rid"), recipeId);
    return q.exec() && q.next();
}

QList<QString> RecipeFavoriteRepository::getFavoriteRecipeIds(const QString &userId) {
    QList<QString> list;
    QSqlQuery q(db_);
    q.prepare(QStringLiteral("SELECT recipe_id FROM recipe_favorites WHERE user_id=:uid"));
    q.bindValue(QStringLiteral(":uid"), userId);
    if (!q.exec()) return list;
    while (q.next()) list.append(q.value(0).toString());
    return list;
}

} // namespace smart_diet
