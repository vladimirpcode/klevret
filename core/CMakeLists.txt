cmake_minimum_required(VERSION 3.21)

# ~~~ файлы исходного кода ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ >>>

# Получение всех файлов исходного кода
# Note: Многие считают GLOB плохой практикой, хотя я не вижу ничего плохого.
#   Если всё-таки будет необходимо, то можно переделать на указание вручную.
file(GLOB KLEVRET_CORE_SOURCE_FILES
    "${KLEVRET_CORE_PATH}/src/*.cpp"
    # Note: Так же видел, что в исходном конфиге заголовочные файлы не указывались.
    #   Честно говоря первый раз вижу подобное, но если нужно, то заголовочные файлы можно убрать.
    "${KLEVRET_CORE_PATH}/src/*.hpp"
    "${KLEVRET_COMMON_PATH}/src/*.hpp"
    "${KLEVRET_COMMON_PATH}/src/*.cpp"
    "${KLEVRET_CONFIG_PATH}/src/*.hpp"
    "${KLEVRET_CONFIG_PATH}/src/*.cpp"
)

# ~~~ core-server target ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ >>>

# Переменная требуется для того, чтобы в случае изменения названия таргента,
#   не пришлось вручную менять во всех местах упоминания.
set(KLEVRET_CORE_TARGET "core-server")
add_executable(${KLEVRET_CORE_TARGET} ${KLEVRET_CORE_SOURCE_FILES})

# Опционально. Указание что в src так же нужно искать заголовочные файлы.
#   Используется в случаях с поддиректориями в файлах исходного кода, чтобы
#   указывать `#include <Subdir/File.hpp>`, а не `#include "../Subdir/File.hpp"`
target_include_directories(${KLEVRET_CORE_TARGET}
    PRIVATE "${KLEVRET_CORE_PATH}/src/"
    "${KLEVRET_COMMON_PATH}/src/"
    "${KLEVRET_CONFIG_PATH}/src/"
)

# Указание стандарта
target_compile_features(${KLEVRET_CORE_TARGET} PRIVATE cxx_std_23)

# Опции компиляции
target_compile_options(${KLEVRET_CORE_TARGET}
    PRIVATE ${KLEVRET_COMMON_COMPILE_OPTIONS}
)

# Остальные параметры
set_target_properties(${KLEVRET_CORE_TARGET} PROPERTIES
    CXX_EXTENSIONS OFF
    POSITION_INDEPENDENT_CODE ON
)

# ~~~ core-server зависимости ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ >>>

target_link_libraries(${KLEVRET_CORE_TARGET}
    PRIVATE
        Boost::asio
        Boost::algorithm
        Boost::property_tree
        Boost::json
        #common
)

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ >>>
