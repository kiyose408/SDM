#pragma once
#include <QObject>
#include <QVariantList>
#include <QVariantMap>
#include "base_service.h"

namespace smart_diet {
class TagRepository;

class TagService : public BaseService {
    Q_OBJECT
public:
    explicit TagService(TagRepository *repo, QObject *parent = nullptr);

    Q_INVOKABLE QVariantList getAll();
    Q_INVOKABLE QVariantList getByRecipe(const QString &recipeId);
    Q_INVOKABLE QVariantList getTagIdsByRecipe(const QString &recipeId);
    Q_INVOKABLE bool setRecipeTags(const QString &recipeId, const QVariantList &tagIds);

private:
    void seedTags();
    TagRepository *repo_;
};

} // namespace smart_diet
