#pragma once
#include <QObject>
#include <QSqlDatabase>
#include <QList>
#include <QString>

namespace smart_diet {

class RecipeFavoriteRepository : public QObject {
    Q_OBJECT
public:
    explicit RecipeFavoriteRepository(QObject *parent = nullptr);

    bool addFavorite(const QString &userId, const QString &recipeId);
    bool removeFavorite(const QString &userId, const QString &recipeId);
    bool isFavorited(const QString &userId, const QString &recipeId);
    QList<QString> getFavoriteRecipeIds(const QString &userId);

private:
    QSqlDatabase db_;
};

} // namespace smart_diet
