#pragma once

#include <QObject>
#include <QSqlDatabase>
#include <QList>
#include <optional>
#include "base_repository.h"
#include "recipe_entity.h"

namespace smart_diet {

class RecipeRepository : public QObject, public BaseRepository<Recipe> {
    Q_OBJECT
public:
    explicit RecipeRepository(QObject *parent = nullptr);

    std::optional<Recipe> getById(const QString &uuid) override;
    QList<Recipe> getAll() override;
    bool save(const Recipe &entity) override;
    bool update(const Recipe &entity) override;
    bool softDelete(const QString &uuid) override;
    bool updateWithVersion(const Recipe &entity, int expectedVersion) override;

    QList<Recipe> getByFamily(const QString &familyId);
    QList<Recipe> searchByName(const QString &keyword);
    QList<Recipe> getByMealType(const QString &mealType);

signals:
    void recipeInserted(const QString &id);
    void recipeUpdated(const QString &id);

private:
    static Recipe mapRow(const QSqlQuery &query);
    QSqlDatabase db_;
};

/**
 * @brief 菜谱食材关联 Repository
 */
class RecipeIngredientRepository : public QObject {
    Q_OBJECT
public:
    explicit RecipeIngredientRepository(QObject *parent = nullptr);

    bool addIngredient(const RecipeIngredient &ri);
    QList<RecipeIngredient> getByRecipe(const QString &recipeId);
    bool removeByRecipe(const QString &recipeId);
    QSqlDatabase database() const { return db_; }

private:
    static RecipeIngredient mapRow(const QSqlQuery &query);
    QSqlDatabase db_;
};

} // namespace smart_diet
