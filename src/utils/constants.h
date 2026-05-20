#pragma once

#include <QString>

// ============================================================================
// 智能家庭饮食管家 — 全局常量、枚举、配置
// 来源：003.详细设计文档 / 004.数据库设计文档 / 005.接口设计文档
// 规则：所有"魔法数字"一律在该文件中定义符号常量，业务代码禁止硬编码
// ============================================================================

namespace smart_diet {

// ============================================================
// 主题色系（来自 003.详细设计文档 §1 色彩体系）
// ============================================================
namespace ThemeColor {
    constexpr auto PRIMARY      = "#64C288";  // 暖柔青柠绿（主色）
    constexpr auto SECONDARY    = "#FF9A5A";  // 活力橙
    constexpr auto DANGER       = "#E86B6B";  // 豆沙红（预警/错误）
    constexpr auto ACCENT       = "#F9D07B";  // 暖杏黄
    constexpr auto INFO         = "#72B5E4";  // 浅湖蓝
    constexpr auto TEXT_PRIMARY = "#333333";  // 主文字深灰
    constexpr auto TEXT_HINT    = "#999999";  // 辅助文字浅灰
    constexpr auto BG_PAGE      = "#F5F5F5";  // 页面背景
    constexpr auto BG_CARD      = "#FFFFFF";  // 卡片背景纯白
}

// ============================================================
// 枚举定义
// ============================================================

/// 饮食目标（对应 users.diet_goal）
enum class DietGoal {
    WEIGHT_LOSS  = 0,  // 健康减脂
    MUSCLE_GAIN  = 1,  // 增肌塑形
    MAINTENANCE  = 2,  // 日常维持
    GATHERING    = 3,  // 聚餐模式
    CHEAT_DAY    = 4   // 放纵餐
};

/// 性别（对应 users.gender）
enum class Gender {
    MALE   = 0,
    FEMALE = 1
};

/// 家庭角色（对应 family_members.role，权限层级 owner > chef > member）
enum class FamilyRole {
    OWNER  = 0,  // 户主：全部权限
    CHEF   = 1,  // 掌勺人：可核销库存
    MEMBER = 2   // 普通成员：浏览 + 心愿
};

/// 库存批次状态
enum class InventoryStatus {
    FRESH    = 0,  // 新鲜
    EXPIRING = 1,  // 临期（≤3 天）
    EXPIRED  = 2   // 过期
};

/// 菜单时段
enum class MealSlot {
    BREAKFAST = 0,
    LUNCH     = 1,
    DINNER    = 2
};

// ============================================================
// 业务常量
// ============================================================

/// TDEE 活动系数（Mifflin-St Jeor 公式配套）
namespace ActivityFactor {
    constexpr double SEDENTARY       = 1.200;  // 久坐不动
    constexpr double LIGHTLY_ACTIVE  = 1.375;  // 轻度活动（1-3 天/周）
    constexpr double MODERATELY      = 1.550;  // 中等活动（3-5 天/周）— 默认值
    constexpr double VERY_ACTIVE     = 1.725;  // 高度活动（6-7 天/周）
    constexpr double EXTRA_ACTIVE    = 1.900;  // 极高活动（体力劳动 + 运动）
}

/// 饮食模式 → TDEE 摄入倍率
namespace DietMultiplier {
    constexpr double WEIGHT_LOSS = 0.80;   // 减脂：80% TDEE
    constexpr double MUSCLE_GAIN = 1.15;   // 增肌：115% TDEE
    constexpr double MAINTENANCE = 1.00;   // 维持：100% TDEE
    constexpr double GATHERING   = 1.30;   // 聚餐：130% TDEE
    constexpr double CHEAT_DAY   = 1.20;   // 放纵餐：120% TDEE
}

/// 默认宏量营养素供能比（碳水 : 蛋白质 : 脂肪）
namespace MacroRatio {
    constexpr double CARBS   = 0.50;  // 碳水 50%
    constexpr double PROTEIN = 0.30;  // 蛋白质 30%
    constexpr double FAT     = 0.20;  // 脂肪 20%
}

/// 临期预警阈值
namespace ExpiryThreshold {
    constexpr int WARNING_DAYS = 3;  // 距过期 ≤3 天触发临期预警
}

/// WebSocket 同步策略
namespace SyncConfig {
    constexpr int MAX_RETRY_COUNT       = 5;
    constexpr int BASE_BACKOFF_MS       = 1000;   // 起始退避 1s
    constexpr int MAX_BACKOFF_MS        = 16000;  // 退避上限 16s
    constexpr int HEARTBEAT_INTERVAL_S  = 30;     // 心跳间隔 30s
    constexpr int SYNC_TARGET_MS        = 2000;   // 同步延迟目标 <2s
}

/// API 分页
namespace Pagination {
    constexpr int DEFAULT_PAGE      = 1;
    constexpr int DEFAULT_PAGE_SIZE = 20;
    constexpr int MAX_PAGE_SIZE     = 100;
}

/// 家庭邀请码
namespace InviteCode {
    constexpr int LENGTH       = 6;   // 6 位数字
    constexpr int EXPIRY_HOURS = 24;  // 24 小时后过期
}

} // namespace smart_diet
