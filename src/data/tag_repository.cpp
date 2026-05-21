#include "tag_repository.h"
#include "tag_entity.h"
#include "database_manager.h"
#include "orm_helper.h"
#include "utils/uuid_utils.h"
#include "utils/time_utils.h"
#include <QSqlQuery>
#include <QDebug>

namespace smart_diet {

TagRepository::TagRepository(QObject *parent)
    : QObject(parent), db_(DatabaseManager::instance().database()) {}

Tag TagRepository::mapRow(const QSqlQuery &query) {
    Tag t;
    t.id         = OrmHelper::readString(query, QStringLiteral("id"));
    t.name       = OrmHelper::readString(query, QStringLiteral("name"));
    t.type       = OrmHelper::readString(query, QStringLiteral("type"));
    t.created_at = OrmHelper::readString(query, QStringLiteral("created_at"));
    return t;
}

QList<Tag> TagRepository::getAll() {
    QList<Tag> list;
    QSqlQuery q(db_);
    q.prepare(QStringLiteral("SELECT * FROM tags ORDER BY type, name"));
    if (!q.exec()) return list;
    while (q.next()) list.append(mapRow(q));
    return list;
}

QList<Tag> TagRepository::getByRecipe(const QString &recipeId) {
    QList<Tag> list;
    QSqlQuery q(db_);
    q.prepare(QStringLiteral("SELECT t.* FROM tags t JOIN recipe_tag_relations r ON t.id=r.tag_id WHERE r.recipe_id=:rid"));
    q.bindValue(QStringLiteral(":rid"), recipeId);
    if (!q.exec()) return list;
    while (q.next()) list.append(mapRow(q));
    return list;
}

QList<QString> TagRepository::getTagIdsByRecipe(const QString &recipeId) {
    QList<QString> ids;
    QSqlQuery q(db_);
    q.prepare(QStringLiteral("SELECT tag_id FROM recipe_tag_relations WHERE recipe_id=:rid"));
    q.bindValue(QStringLiteral(":rid"), recipeId);
    if (!q.exec()) return ids;
    while (q.next()) ids.append(q.value(0).toString());
    return ids;
}

bool TagRepository::setRecipeTags(const QString &recipeId, const QList<QString> &tagIds) {
    QSqlQuery q(db_);
    q.prepare(QStringLiteral("DELETE FROM recipe_tag_relations WHERE recipe_id=:rid"));
    q.bindValue(QStringLiteral(":rid"), recipeId);
    q.exec();

    const QString now = utcNow();
    for (const auto &tid : tagIds) {
        QSqlQuery ins(db_);
        ins.prepare(QStringLiteral("INSERT INTO recipe_tag_relations (id,recipe_id,tag_id,created_at) VALUES (:id,:rid,:tid,:ca)"));
        ins.bindValue(QStringLiteral(":id"), generateUuid());
        ins.bindValue(QStringLiteral(":rid"), recipeId);
        ins.bindValue(QStringLiteral(":tid"), tid);
        ins.bindValue(QStringLiteral(":ca"), now);
        if (!ins.exec()) return false;
    }
    return true;
}

} // namespace smart_diet
