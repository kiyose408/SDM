#include "recipe_repository.h"
#include "database_manager.h"
#include "orm_helper.h"
#include "utils/time_utils.h"
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>

namespace smart_diet {

// ================================================================
// RecipeRepository
// ================================================================
RecipeRepository::RecipeRepository(QObject *parent)
    : QObject(parent), db_(DatabaseManager::instance().database()) {}

Recipe RecipeRepository::mapRow(const QSqlQuery &query) {
    Recipe r;
    r.id              = OrmHelper::readString(query, QStringLiteral("id"));
    r.name            = OrmHelper::readString(query, QStringLiteral("name"));
    r.cover_image_url = OrmHelper::readString(query, QStringLiteral("cover_image_url"));
    r.description     = OrmHelper::readString(query, QStringLiteral("description"));
    r.cooking_time    = OrmHelper::readInt(query, QStringLiteral("cooking_time"));
    r.servings        = OrmHelper::readInt(query, QStringLiteral("servings"));
    r.meal_type       = OrmHelper::readString(query, QStringLiteral("meal_type"));
    r.total_calories  = OrmHelper::readReal(query, QStringLiteral("total_calories"));
    r.total_protein   = OrmHelper::readReal(query, QStringLiteral("total_protein"));
    r.total_carbs     = OrmHelper::readReal(query, QStringLiteral("total_carbs"));
    r.total_fat       = OrmHelper::readReal(query, QStringLiteral("total_fat"));
    r.is_system       = OrmHelper::readBool(query, QStringLiteral("is_system"));
    r.creator_id      = OrmHelper::readString(query, QStringLiteral("creator_id"));
    r.family_id       = OrmHelper::readString(query, QStringLiteral("family_id"));
    r.popularity      = OrmHelper::readInt(query, QStringLiteral("popularity"));
    r.is_deleted      = OrmHelper::readBool(query, QStringLiteral("is_deleted"));
    r.version         = OrmHelper::readInt(query, QStringLiteral("version"));
    r.created_at      = OrmHelper::readString(query, QStringLiteral("created_at"));
    r.updated_at      = OrmHelper::readString(query, QStringLiteral("updated_at"));
    return r;
}

std::optional<Recipe> RecipeRepository::getById(const QString &uuid) {
    QSqlQuery query(db_);
    query.prepare(QStringLiteral("SELECT * FROM recipes WHERE id=:id AND is_deleted=0"));
    OrmHelper::bindId(query, uuid);
    if (!query.exec() || !query.next()) return std::nullopt;
    return mapRow(query);
}

QList<Recipe> RecipeRepository::getAll() {
    QList<Recipe> list;
    QSqlQuery query(db_);
    query.prepare(QStringLiteral("SELECT * FROM recipes WHERE is_deleted=0 ORDER BY popularity DESC"));
    if (!query.exec()) return list;
    while (query.next()) list.append(mapRow(query));
    return list;
}

bool RecipeRepository::save(const Recipe &e) {
    QSqlQuery q(db_);
    q.prepare(QStringLiteral(
        "INSERT INTO recipes (id,name,cover_image_url,description,cooking_time,servings,"
        "meal_type,total_calories,total_protein,total_carbs,total_fat,"
        "is_system,creator_id,family_id,popularity,is_deleted,version,created_at,updated_at)"
        " VALUES (:id,:n,:img,:d,:ct,:s,:mt,:tc,:tp,:tcb,:tf,:is,:ci,:fi,:p,:dl,:v,:ca,:ua)"));
    q.bindValue(QStringLiteral(":id"), e.id); q.bindValue(QStringLiteral(":n"), e.name);
    q.bindValue(QStringLiteral(":img"), e.cover_image_url.isEmpty()?QVariant():e.cover_image_url);
    q.bindValue(QStringLiteral(":d"), e.description.isEmpty()?QVariant():e.description);
    q.bindValue(QStringLiteral(":ct"), e.cooking_time); q.bindValue(QStringLiteral(":s"), e.servings);
    q.bindValue(QStringLiteral(":mt"), e.meal_type); q.bindValue(QStringLiteral(":tc"), e.total_calories);
    q.bindValue(QStringLiteral(":tp"), e.total_protein); q.bindValue(QStringLiteral(":tcb"), e.total_carbs);
    q.bindValue(QStringLiteral(":tf"), e.total_fat); q.bindValue(QStringLiteral(":is"), e.is_system?1:0);
    q.bindValue(QStringLiteral(":ci"), e.creator_id.isEmpty()?QVariant():e.creator_id);
    q.bindValue(QStringLiteral(":fi"), e.family_id.isEmpty()?QVariant():e.family_id);
    q.bindValue(QStringLiteral(":p"), e.popularity); q.bindValue(QStringLiteral(":dl"), e.is_deleted?1:0);
    q.bindValue(QStringLiteral(":v"), e.version); q.bindValue(QStringLiteral(":ca"), e.created_at);
    q.bindValue(QStringLiteral(":ua"), e.updated_at);
    if (!q.exec()) { qWarning()<<"[RecipeRepo] save failed:"<<q.lastError().text(); return false; }
    emit recipeInserted(e.id); return true;
}

bool RecipeRepository::update(const Recipe &e) {
    QSqlQuery q(db_);
    q.prepare(QStringLiteral("UPDATE recipes SET name=:n,description=:d,cooking_time=:ct,servings=:s,"
        "meal_type=:mt,total_calories=:tc,total_protein=:tp,total_carbs=:tcb,total_fat=:tf,"
        "version=version+1,updated_at=:ua WHERE id=:id"));
    OrmHelper::bindId(q, e.id); q.bindValue(QStringLiteral(":n"), e.name);
    q.bindValue(QStringLiteral(":d"), e.description.isEmpty()?QVariant():e.description);
    q.bindValue(QStringLiteral(":ct"), e.cooking_time); q.bindValue(QStringLiteral(":s"), e.servings);
    q.bindValue(QStringLiteral(":mt"), e.meal_type);
    q.bindValue(QStringLiteral(":tc"), e.total_calories); q.bindValue(QStringLiteral(":tp"), e.total_protein);
    q.bindValue(QStringLiteral(":tcb"), e.total_carbs); q.bindValue(QStringLiteral(":tf"), e.total_fat);
    q.bindValue(QStringLiteral(":ua"), e.updated_at);
    if (!q.exec()) { qWarning()<<"[RecipeRepo] update failed:"<<q.lastError().text(); return false; }
    emit recipeUpdated(e.id); return true;
}

bool RecipeRepository::softDelete(const QString &uuid) {
    QSqlQuery q(db_);
    q.prepare(QStringLiteral("UPDATE recipes SET is_deleted=1,updated_at=:now WHERE id=:id"));
    OrmHelper::bindId(q, uuid); q.bindValue(QStringLiteral(":now"), utcNow());
    return q.exec();
}

bool RecipeRepository::updateWithVersion(const Recipe &e, int ev) {
    QSqlQuery q(db_);
    q.prepare(QStringLiteral("UPDATE recipes SET name=:n,version=version+1,updated_at=:ua WHERE id=:id AND version=:version"));
    OrmHelper::bindIdAndVersion(q, e.id, ev);
    q.bindValue(QStringLiteral(":n"), e.name); q.bindValue(QStringLiteral(":ua"), e.updated_at);
    if (!q.exec()||q.numRowsAffected()==0) return false;
    emit recipeUpdated(e.id); return true;
}

QList<Recipe> RecipeRepository::getByFamily(const QString &familyId) {
    QList<Recipe> list; QSqlQuery q(db_);
    q.prepare(QStringLiteral("SELECT * FROM recipes WHERE (family_id=:fid OR is_system=1) AND is_deleted=0 ORDER BY popularity DESC"));
    q.bindValue(QStringLiteral(":fid"), familyId);
    if (!q.exec()) return list;
    while (q.next()) list.append(mapRow(q));
    return list;
}

QList<Recipe> RecipeRepository::searchByName(const QString &keyword) {
    QList<Recipe> list; QSqlQuery q(db_);
    q.prepare(QStringLiteral("SELECT * FROM recipes WHERE name LIKE :kw AND is_deleted=0 ORDER BY popularity DESC"));
    q.bindValue(QStringLiteral(":kw"), QStringLiteral("%%1%").arg(keyword));
    if (!q.exec()) return list;
    while (q.next()) list.append(mapRow(q));
    return list;
}

QList<Recipe> RecipeRepository::getByMealType(const QString &mealType) {
    QList<Recipe> list; QSqlQuery q(db_);
    q.prepare(QStringLiteral("SELECT * FROM recipes WHERE (meal_type=:mt OR meal_type='any') AND is_deleted=0 ORDER BY popularity DESC"));
    q.bindValue(QStringLiteral(":mt"), mealType);
    if (!q.exec()) return list;
    while (q.next()) list.append(mapRow(q));
    return list;
}

// ================================================================
// RecipeIngredientRepository
// ================================================================
RecipeIngredientRepository::RecipeIngredientRepository(QObject *parent)
    : QObject(parent), db_(DatabaseManager::instance().database()) {}

RecipeIngredient RecipeIngredientRepository::mapRow(const QSqlQuery &query) {
    RecipeIngredient ri;
    ri.id            = OrmHelper::readString(query, QStringLiteral("id"));
    ri.recipe_id     = OrmHelper::readString(query, QStringLiteral("recipe_id"));
    ri.ingredient_id = OrmHelper::readString(query, QStringLiteral("ingredient_id"));
    ri.amount        = OrmHelper::readReal(query, QStringLiteral("amount"));
    ri.created_at    = OrmHelper::readString(query, QStringLiteral("created_at"));
    return ri;
}

bool RecipeIngredientRepository::addIngredient(const RecipeIngredient &ri) {
    QSqlQuery q(db_);
    q.prepare(QStringLiteral("INSERT INTO recipe_ingredients (id,recipe_id,ingredient_id,amount,created_at) VALUES (:id,:rid,:iid,:a,:ca)"));
    q.bindValue(QStringLiteral(":id"), ri.id); q.bindValue(QStringLiteral(":rid"), ri.recipe_id);
    q.bindValue(QStringLiteral(":iid"), ri.ingredient_id); q.bindValue(QStringLiteral(":a"), ri.amount);
    q.bindValue(QStringLiteral(":ca"), ri.created_at);
    return q.exec();
}

QList<RecipeIngredient> RecipeIngredientRepository::getByRecipe(const QString &recipeId) {
    QList<RecipeIngredient> list; QSqlQuery q(db_);
    q.prepare(QStringLiteral("SELECT * FROM recipe_ingredients WHERE recipe_id=:rid"));
    q.bindValue(QStringLiteral(":rid"), recipeId);
    if (!q.exec()) return list;
    while (q.next()) list.append(mapRow(q));
    return list;
}

bool RecipeIngredientRepository::removeByRecipe(const QString &recipeId) {
    QSqlQuery q(db_);
    q.prepare(QStringLiteral("DELETE FROM recipe_ingredients WHERE recipe_id=:rid"));
    q.bindValue(QStringLiteral(":rid"), recipeId);
    return q.exec();
}

} // namespace smart_diet
