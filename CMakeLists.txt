cmake_minimum_required(VERSION 3.16)

project(SmartDietManager VERSION 0.1 LANGUAGES C CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ================================================================
# Argon2 密码哈希库（third_party）
# ================================================================
set(ARGON2_SOURCES
    third_party/argon2/argon2.c
    third_party/argon2/core.c
    third_party/argon2/blake2/blake2b.c
    third_party/argon2/encoding.c
    third_party/argon2/thread.c
    third_party/argon2/ref.c
)
add_library(argon2 STATIC ${ARGON2_SOURCES})
target_include_directories(argon2 PUBLIC
    third_party/argon2
    third_party/argon2/blake2
)
if(MSVC)
    target_compile_definitions(argon2 PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

# MSVC 环境下强制使用 msvc2022_64 版本的 Qt（避免误用 mingw_64 导致 ABI 不兼容）
if(MSVC)
    set(CMAKE_PREFIX_PATH "D:/Qt/6.9.0/msvc2022_64" CACHE PATH "Qt MSVC installation path")
endif()

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Quick QuickControls2 Sql Network WebSockets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Quick QuickControls2 Sql Network WebSockets LinguistTools)

set(TS_FILES SmartDietManager_zh_CN.ts)


set(PROJECT_SOURCES
    src/app/main.cpp
    src/core/base_service.h
    src/core/registration_validator.h
    src/core/registration_validator.cpp
    src/core/tdee_calculator.h
    src/core/tdee_calculator.cpp
    src/core/auth_service.h
    src/core/auth_service.cpp
    src/core/session_manager.h
    src/core/session_manager.cpp
    src/core/family_service.h
    src/core/family_service.cpp
    src/core/ingredient_service.h
    src/core/ingredient_service.cpp
    src/core/recipe_service.h
    src/core/recipe_service.cpp
    src/data/base_repository.h
    src/data/database_manager.h
    src/data/database_manager.cpp
    src/network/sync_manager.h
    src/utils/constants.h
    src/utils/uuid_utils.h
    src/utils/time_utils.h
    src/utils/argon2_hasher.h
    src/utils/argon2_hasher.cpp
    src/data/orm_helper.h
    src/data/user_entity.h
    src/data/user_repository.h
    src/data/user_repository.cpp
    src/data/family_entity.h
    src/data/family_repository.h
    src/data/family_repository.cpp
    src/data/family_member_repository.h
    src/data/family_member_repository.cpp
    src/data/ingredient_entity.h
    src/data/ingredient_repository.h
    src/data/ingredient_repository.cpp
    src/data/recipe_entity.h
    src/data/recipe_repository.h
    src/data/recipe_repository.cpp
    resources/translations/SmartDietManager_zh_CN.ts
    resources.qrc
)
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(SmartDietManager
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET SmartDietManager APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
# 解释：这行代码告诉编译器，去 src 目录下查找头文件
    target_include_directories(SmartDietManager PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )
# =========================
    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(SmartDietManager SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(SmartDietManager
            ${PROJECT_SOURCES}
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

target_link_libraries(SmartDietManager PRIVATE argon2
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::QuickControls2
    Qt${QT_VERSION_MAJOR}::Sql
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::WebSockets
)

# MSVC 编译器必要配置：C++17 标准检测 + 中文注释 UTF-8 支持
if(MSVC)
    target_compile_options(SmartDietManager PRIVATE /Zc:__cplusplus /utf-8)
endif()

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.SmartDietManager)
endif()
set_target_properties(SmartDietManager PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS SmartDietManager
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(SmartDietManager)
endif()
