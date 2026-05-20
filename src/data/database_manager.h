#pragma once

#include <QObject>
#include <QSqlDatabase>
#include <QString>

namespace smart_diet {

/**
 * @brief 数据库连接管理器（单例）
 *
 * 负责 SQLite 本地数据库的生命周期管理：
 * - 创建/打开数据库文件（路径由 QStandardPaths 决定）
 * - 首次启动时执行建表 DDL（从嵌入的 .qrc 资源读取）
 * - 提供 QSqlDatabase 句柄给 Repository 层
 * - 跟踪 schema 版本号，为将来数据库迁移预留接口
 *
 * 线程安全：QSqlDatabase 只能在创建它的线程中使用。
 * 当前项目为单线程架构（主线程），后续如需多线程需为每个线程创建独立连接。
 */
class DatabaseManager : public QObject {
    Q_OBJECT

public:
    /// 获取单例实例
    static DatabaseManager &instance();

    /// 初始化数据库：打开/创建文件 → 执行建表 DDL → 设置 pragma
    /// @return 是否初始化成功
    bool initialize();

    /// 获取数据库连接句柄
    QSqlDatabase database() const;

    /// 数据库是否已成功初始化
    bool isInitialized() const { return initialized_; }

    /// 当前 schema 版本号
    int schemaVersion() const;

private:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager() override;
    Q_DISABLE_COPY(DatabaseManager)

    /// 执行嵌入的建表 DDL 脚本
    bool createTables();

    /// 执行指定的 SQL 文件
    bool executeSqlFile(const QString &qrcPath);

    /// 写入 schema 版本号到数据库
    void setSchemaVersion(int version);

    QString     dbPath_;       // 数据库文件完整路径
    QString     connectionName_; // Qt 连接名
    bool        initialized_ = false;

    static constexpr int CURRENT_SCHEMA_VERSION = 1;
};

} // namespace smart_diet
