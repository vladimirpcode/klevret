cmake_minimum_required(VERSION 3.21)

# ~~~ файлы исходного кода ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ >>>

# Получение всех файлов исходного кода
# Note: Многие считают GLOB плохой практикой, хотя я не вижу ничего плохого.
#   Если всё-таки будет необходимо, то можно переделать на указание вручную.
file(GLOB KLEVRET_CLI_SOURCE_FILES
    "${KLEVRET_CLI_PATH}/src/*.cpp"
    # Note: Так же видел, что в исходном конфиге заголовочные файлы не указывались.
    #   Честно говоря первый раз вижу подобное, но если нужно, то заголовочные файлы можно убрать.
    "${KLEVRET_CLI_PATH}/src/*.hpp"
    "${KLEVRET_COMMON_PATH}/src/*.hpp"
    "${KLEVRET_COMMON_PATH}/src/*.cpp"
    "${KLEVRET_CONFIG_PATH}/src/*.hpp"
    "${KLEVRET_CONFIG_PATH}/src/*.cpp"
)

# ~~~ cli target ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ >>>

# Переменная требуется для того, чтобы в случае изменения названия таргента,
#   не пришлось вручную менять во всех местах упоминания.
set(KLEVRET_CLI_TARGET "cli")
add_executable(${KLEVRET_CLI_TARGET} ${KLEVRET_CLI_SOURCE_FILES})

# Опционально. Указание что в src так же нужно искать заголовочные файлы.
#   Используется в случаях с поддиректориями в файлах исходного кода, чтобы
#   указывать `#include <Subdir/File.hpp>`, а не `#include "../Subdir/File.hpp"`
target_include_directories(${KLEVRET_CLI_TARGET}
    PRIVATE "${KLEVRET_CLI_PATH}/src/"
    "${KLEVRET_CONFIG_PATH}/src/"
    "${KLEVRET_COMMON_PATH}/src/"
)

# Указание стандарта
target_compile_features(${KLEVRET_CLI_TARGET} PRIVATE cxx_std_23)

# Опции компиляции
target_compile_options(${KLEVRET_CLI_TARGET}
    PRIVATE ${KLEVRET_COMMON_COMPILE_OPTIONS}
)

# Остальные параметры
set_target_properties(${KLEVRET_CLI_TARGET} PROPERTIES
    CXX_EXTENSIONS OFF
    POSITION_INDEPENDENT_CODE ON
)

target_link_libraries(${KLEVRET_CLI_TARGET}
    PRIVATE
        Boost::asio
        Boost::algorithm
        Boost::property_tree
        Boost::json
)

# ~~~ тесты ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ >>>

if(KLEVRET_BUILD_TESTS)
    add_subdirectory(tests)
endif()

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ >>>
