
📄 README.md (SmartDietManager 定制版)

🥗 SmartDietManager

智能饮食管家 · 让每一餐都心中有数

SmartDietManager 是一款跨平台桌面应用，旨在通过 AI 算法与家庭协作机制，解决现代家庭“吃什么难决策”、“营养摄入不透明”以及“食材浪费严重”等核心痛点。从菜单规划、智能采购、食材入库到烹饪核销，实现全流程闭环管理。

📋 目录

项目概览
核心功能
技术架构
开发计划
贡献指南

📊 项目概览

本项目基于 Qt/QML 框架开发，采用单人开发模式，旨在构建一款集个性化膳食推荐、家庭共享冰箱管理、智能采购与烹饪核销于一体的 App。

项目类型：跨平台桌面应用 (支持 Windows/macOS/Linux)
开发人员：1人
核心目标：
  实现家庭饮食数据的多端实时同步 (承诺  扣减库存 -> 生成消费记录 -> 全员同步

离线模式
无网络时自动加载本地 SQLite 数据，支持全功能操作，网络恢复后静默同步

🚀 开发与部署

环境准备
开发工具：Qt Creator 6.x
编译器：支持 C++17 的编译器 (MSVC/GCC/Clang)
依赖库：PostgreSQL 驱动, Qt SQL, Qt Network

项目结构
text
src/
├── ui/               # QML 视图与组件
├── core/             # C++ 业务逻辑与 Service 层
├── data/             # Repository 与数据库操作
├── network/          # WebSocket 与 HTTP 客户端
└── utils/            # 工具类 (加密、OCR封装等)

启动步骤
安装 Qt 6.x 环境
初始化本地 SQLite 数据库 (执行 schema.sql)
配置云端 PostgreSQL 连接参数 (开发环境配置)
使用 Qt Creator 打开项目并构建运行

📅 项目开发计划

根据《项目开发计划 v1.1》，当前预计总工时约 500+ 小时。
阶段   里程碑   预计耗时 (现实估算)
Phase 1   基础框架搭建   ~7 天

Phase 2   用户认证系统   ~5 天

Phase 3   家庭管理模块   ~5.5 天

Phase 4   菜谱与食材管理   ~4.5 天

Phase 5   每日菜单管理   ~5 天

Phase 6   冰箱库存管理   ~5.5 天

Phase 7   采购清单与核销   ~5.5 天

Phase 8   WebSocket 实时同步   ~9 天 (高风险)

Phase 9   测试与优化   ~9 天

Phase 10   发布与打包   ~7.5 天

📅 预计完成时间：约 3.5 - 4 个月 (含缓冲期)

🔐 安全与隐私

密码存储：使用 Argon2id 算法进行哈希加密 (当前最佳实践)
通信安全：全链路 HTTPS/WSS 加密，证书固定 (Certificate Pinning)
数据隔离：严格的 RBAC 权限模型，确保家庭数据仅家庭成员可见
隐私合规：符合《个人信息保护法》，敏感数据脱敏展示

💡 贡献与沟通

由于是单人开发项目，暂不接受外部代码贡献。如有建议，欢迎通过 Issues 提出。
