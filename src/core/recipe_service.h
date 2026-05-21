#pragma once
#include <QObject>
#include <QVariantList>
#include <QVariantMap>
#include "base_service.h"

namespace smart_diet {
class RecipeRepository;
class RecipeIngredientRepository;
class IngredientRepository;

class RecipeService : public BaseService {
    Q_OBJECT
public:
    explicit RecipeService(RecipeRepository *r, RecipeIngredientRepository *ri,
                           IngredientRepository *ir, QObject *parent = nullptr);

    Q_INVOKABLE QVariantList getAll();
    Q_INVOKABLE QVariantList getByFamily(const QString &familyId);
    Q_INVOKABLE QVariantMap getById(const QString &id);
    Q_INVOKABLE QVariantList getIngredients(const QString &recipeId);

private:
    void seedSystemRecipes();
    QVariantMap toMap(const struct Recipe &r) const;
    RecipeRepository *repo_;
    RecipeIngredientRepository *riRepo_;
    IngredientRepository *ingRepo_;
};
} // namespace smart_diet
