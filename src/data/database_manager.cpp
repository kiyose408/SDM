#include "database_manager.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QTextStream>
#include <QDebug>

namespace smart_diet {

// ============================================================================
// 单例
// ============================================================================

DatabaseManager &DatabaseManager::instance() {
    static DatabaseManager inst;
    return inst;
}

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent)
    , connectionName_(QStringLiteral("smartdiet_local"))
{
}

DatabaseManager::~DatabaseManager() {
    if (QSqlDatabase::contains(connectionName_)) {
        QSqlDatabase::database(connectionName_).close();
        QSqlDatabase::removeDatabase(connectionName_);
    }
}

// ============================================================================
// 初始化
// ============================================================================

bool DatabaseManager::initialize() {
    if (initialized_) {
        qDebug() << "[DatabaseManager] Already initialized, skipping.";
        return true;
    }

    // --- 1. 确定数据库文件路径 ---
    const QString appDataDir = QStandardPaths::writableLocation(
        QStandardPaths::AppDataLocation);
    QDir().mkpath(appDataDir);  // 确保目录存在
    dbPath_ = appDataDir + QStringLiteral("/smartdiet.db");

    qDebug() << "[DatabaseManager] DB path:" << dbPath_;

    // --- 2. 打开 SQLite 连接 ---
    QSqlDatabase db = QSqlDatabase::addDatabase(
        QStringLiteral("QSQLITE"), connectionName_);
    db.setDatabaseName(dbPath_);

    if (!db.open()) {
        qWarning() << "[DatabaseManager] Failed to open database:"
                    << db.lastError().text();
        return false;
    }

    // --- 3. 设置 pragma ---
    {
        QSqlQuery pragma(db);
        pragma.exec(QStringLiteral("PRAGMA foreign_keys = ON"));
        pragma.exec(QStringLiteral("PRAGMA journal_mode = WAL"));
    }

    // --- 4. 检查并创建表 ---
    {
        QSqlQuery query(db);
        query.exec(QStringLiteral(
            "SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='users'"));
        if (query.next() && query.value(0).toInt() == 0) {
            qDebug() << "[DatabaseManager] First run — creating tables...";
            if (!createTables()) {
                qWarning() << "[DatabaseManager] Table creation failed.";
                db.close();
                return false;
            }
            setSchemaVersion(CURRENT_SCHEMA_VERSION);
            qDebug() << "[DatabaseManager] Tables created successfully.";
        }
    }

    initialized_ = true;
    qDebug() << "[DatabaseManager] Initialization complete.";
    return true;
}

QSqlDatabase DatabaseManager::database() const {
    return QSqlDatabase::database(connectionName_);
}

// ============================================================================
// Schema 版本
// ============================================================================

int DatabaseManager::schemaVersion() const {
    if (!initialized_) return 0;
    QSqlQuery query(database());
    query.exec(QStringLiteral(
        "SELECT value FROM sqlite_meta WHERE key = 'schema_version'"));
    if (query.next())
        return query.value(0).toInt();
    return 0;
}

void DatabaseManager::setSchemaVersion(int version) {
    QSqlQuery query(database());
    query.exec(QStringLiteral(
        "CREATE TABLE IF NOT EXISTS sqlite_meta (key TEXT PRIMARY KEY, value TEXT)"));
    query.prepare(QStringLiteral(
        "INSERT OR REPLACE INTO sqlite_meta (key, value) VALUES ('schema_version', :ver)"));
    query.bindValue(QStringLiteral(":ver"), version);
    query.exec();
}

// ============================================================================
// DDL 执行
// ============================================================================

bool DatabaseManager::createTables() {
    return executeSqlFile(QStringLiteral(":/schema/sqlite_schema.sql"));
}

bool DatabaseManager::executeSqlFile(const QString &qrcPath) {
    QFile file(qrcPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "[DatabaseManager] Cannot open SQL file:" << qrcPath;
        return false;
    }

    QTextStream stream(&file);
    const QString sql = stream.readAll();
    file.close();

    // SQLite 不支持一次执行多条语句，需要逐条分割
    // 简单分割策略：按分号切分，跳过空语句和纯注释行
    const QStringList statements = sql.split(QLatin1Char(';'),
                                              Qt::SkipEmptyParts);

    QSqlDatabase db = database();
    int executed = 0;
    int errors = 0;

    for (const QString &raw : statements) {
        const QString trimmed = raw.trimmed();
        // 跳过空语句块
        if (trimmed.isEmpty())
            continue;

        // 跳过纯注释块（去掉所有 -- 行后无实际 SQL 内容）
        QStringList lines = trimmed.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
        QStringList sqlLines;
        for (const QString &line : lines) {
            if (!line.trimmed().startsWith(QLatin1String("--")))
                sqlLines.append(line);
        }
        if (sqlLines.isEmpty())
            continue;

        QSqlQuery query(db);
        if (!query.exec(trimmed)) {
            qWarning() << "[DatabaseManager] SQL error:" << query.lastError().text()
                        << "\n  Statement:" << trimmed.left(120);
            errors++;
        } else {
            executed++;
        }
    }

    qDebug() << "[DatabaseManager] SQL execution:"
             << executed << "ok," << errors << "errors";
    return errors == 0;
}

} // namespace smart_diet
