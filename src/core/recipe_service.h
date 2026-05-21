#pragma once
#include <QObject>
#include <QVariantList>
#include <QVariantMap>
#include "base_service.h"

namespace smart_diet {
class RecipeRepository;
class RecipeIngredientRepository;
class IngredientRepository;
class RecipeFavoriteRepository;

class RecipeService : public BaseService {
    Q_OBJECT
public:
    explicit RecipeService(RecipeRepository *r, RecipeIngredientRepository *ri,
                           IngredientRepository *ir, RecipeFavoriteRepository *fr,
                           QObject *parent = nullptr);

    Q_INVOKABLE QVariantList getAll();
    Q_INVOKABLE QVariantList getByFamily(const QString &familyId);
    Q_INVOKABLE QVariantList searchByName(const QString &keyword);
    Q_INVOKABLE QVariantMap getById(const QString &id);
    Q_INVOKABLE QVariantList getIngredients(const QString &recipeId);

    /// 创建菜谱，返回 { success, recipeId }
    Q_INVOKABLE QVariantMap createRecipe(const QVariantMap &data, const QVariantList &ingredients,
                                          const QString &creatorId, const QString &familyId);
    /// 更新菜谱
    Q_INVOKABLE bool updateRecipe(const QString &recipeId, const QVariantMap &data,
                                   const QVariantList &ingredients);

    Q_INVOKABLE bool deleteRecipe(const QString &recipeId);

    Q_INVOKABLE bool toggleFavorite(const QString &userId, const QString &recipeId);
    Q_INVOKABLE bool isFavorited(const QString &userId, const QString &recipeId);
    Q_INVOKABLE QVariantList getFavorites(const QString &userId);

private:
    void seedSystemRecipes();
    QVariantMap toMap(const struct Recipe &r) const;
    RecipeRepository *repo_;
    RecipeIngredientRepository *riRepo_;
    IngredientRepository *ingRepo_;
    RecipeFavoriteRepository *favRepo_ = nullptr;
};
} // namespace smart_diet
