cmake_minimum_required(VERSION 3.21)

# ~~~ проект ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ >>>

# Инициализация проекта
project("klevret" LANGUAGES CXX)

# Запрет на сборку в корневой директории
if(${CMAKE_CURRENT_BINARY_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    message(FATAL_ERROR
        "\nIn-source сборка запрещена.builds isn't allowed."
        "\nСоздайте build-директорию выполните сборку оттуда."
        "\nНе забудьте удалить CMakeCache.txt."
    )
endif()

# ~~~ опции ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ >>>

# Компоненты
option(KLEVRET_BUILD_CORE "Сборка ядра" ON)
option(KLEVRET_BUILD_DHCP "Сборка dhcp" ON)
option(KLEVRET_BUILD_CLI "Сборка консоли" ON)
option(KLEVRET_BUILD_COMMON "Сборка общих библиотек" OFF) # хардкодом собираются
option(KLEVRET_BUILD_CONFIG "Сборка конфига" OFF) # хардкодом собираются

# Тесты
option(KLEVRET_BUILD_TESTS "Сборка тестов" ON)

# ~~~ глобальные настройки ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ >>>

# Получение информации о типе генератора
get_property(IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

# Установка типа сборки по-умолчанию
if(NOT CMAKE_BUILD_TYPE AND NOT IS_MULTI_CONFIG)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
endif()

# Указание возможных типов конфигурации для multi-config генераторов
if(IS_MULTI_CONFIG)
    set(CMAKE_CONFIGURATION_TYPES "Release;Debug" CACHE STRING "" FORCE)
endif()

# Включение экспорта команд компиляции (используется допустим для clangd)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "" FORCE)

# Указание общих опций компиляции
list(APPEND KLEVRET_COMMON_COMPILE_OPTIONS -Wall -Wextra -Wpedantic)

# ~~~ подготовка общих зависимостей ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ >>>

include(FetchContent)

# googletest
message(STATUS "Загрузка 'google/googletest.git'")
FetchContent_Declare("googletest"
    GIT_REPOSITORY "https://github.com/google/googletest.git"
    GIT_TAG "v1.16.0"
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
    EXCLUDE_FROM_ALL
)
set(INSTALL_GTEST FALSE CACHE BOOL "" FORCE)
FetchContent_MakeAvailable("googletest")

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/PrepareBoostModule.cmake")

PrepareBoostModule("asio" "1.87.0")
PrepareBoostModule("algorithm" "1.87.0")
PrepareBoostModule("property_tree" "1.87.0")
PrepareBoostModule("json" "1.87.0")

# ~~~ добавление компонентов ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ >>>

# Указание путей до корня компонентов (будет использовано в локальных конфигах)
set(KLEVRET_CORE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/core")
set(KLEVRET_DHCP_PATH "${CMAKE_CURRENT_SOURCE_DIR}/dhcp")
set(KLEVRET_CLI_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cli")
set(KLEVRET_COMMON_PATH "${CMAKE_CURRENT_SOURCE_DIR}/common")
set(KLEVRET_CONFIG_PATH "${CMAKE_CURRENT_SOURCE_DIR}/config")

# core
if(${KLEVRET_BUILD_CORE})
    add_subdirectory(${KLEVRET_CORE_PATH})
endif()

# dhcp
if(${KLEVRET_BUILD_DHCP})
    add_subdirectory(${KLEVRET_DHCP_PATH})
endif()

# cli
if(${KLEVRET_BUILD_CLI})
    add_subdirectory(${KLEVRET_CLI_PATH})
endif()

# common
if(${KLEVRET_BUILD_COMMON})
    #add_subdirectory(${KLEVRET_COMMON_PATH})
endif()

#

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ >>>
