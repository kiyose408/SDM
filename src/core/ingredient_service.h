#pragma once

#include <QObject>
#include <QVariantList>
#include <QVariantMap>
#include "base_service.h"

namespace smart_diet {

class IngredientRepository;

class IngredientService : public BaseService {
    Q_OBJECT
public:
    explicit IngredientService(IngredientRepository *repo, QObject *parent = nullptr);

    Q_INVOKABLE QVariantList getAll();
    Q_INVOKABLE QVariantList getByCategory(const QString &category);
    Q_INVOKABLE QVariantList searchByName(const QString &keyword);
    Q_INVOKABLE QVariantMap getById(const QString &id);

    /// 预置系统食材（仅首次运行）
    void seedSystemIngredients();

private:
    QVariantMap toMap(const struct Ingredient &i) const;
    IngredientRepository *repo_;
};

} // namespace smart_diet
