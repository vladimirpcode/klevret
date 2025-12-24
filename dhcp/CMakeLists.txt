cmake_minimum_required(VERSION 3.21)

# ~~~ файлы исходного кода ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ >>>

# Получение всех файлов исходного кода
# Note: Многие считают GLOB плохой практикой, хотя я не вижу ничего плохого.
#   Если всё-таки будет необходимо, то можно переделать на указание вручную.
file(GLOB KLEVRET_DHCP_SOURCE_FILES
    "${KLEVRET_DHCP_PATH}/src/*.cpp"
    # Note: Так же видел, что в исходном конфиге заголовочные файлы не указывались.
    #   Честно говоря первый раз вижу подобное, но если нужно, то заголовочные файлы можно убрать.
    "${KLEVRET_DHCP_PATH}/src/*.hpp"
    "${KLEVRET_COMMON_PATH}/src/*.hpp"
    "${KLEVRET_COMMON_PATH}/src/*.cpp"
    "${KLEVRET_CONFIG_PATH}/src/*.cpp"
    "${KLEVRET_CONFIG_PATH}/src/*.hpp"
)

# ~~~ dhcp-server target ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ >>>

# Переменная требуется для того, чтобы в случае изменения названия таргента,
#   не пришлось вручную менять во всех местах упоминания.
set(KLEVRET_DHCP_TARGET "dhcp-server")
add_executable(
    ${KLEVRET_DHCP_TARGET}
    ${KLEVRET_DHCP_SOURCE_FILES}
    #$<TARGET_OBJECTS:common>
)

target_sources(${KLEVRET_DHCP_TARGET} PUBLIC  ${KLEVRET_DHCP_SOURCE_FILES})

# Опционально. Указание что в src так же нужно искать заголовочные файлы.
#   Используется в случаях с поддиректориями в файлах исходного кода, чтобы
#   указывать `#include <Subdir/File.hpp>`, а не `#include "../Subdir/File.hpp"`
target_include_directories(${KLEVRET_DHCP_TARGET}
    PUBLIC "${KLEVRET_DHCP_PATH}/src/"
    "${KLEVRET_COMMON_PATH}/src/"
    "${KLEVRET_CONFIG_PATH}/src/"
)

# Указание стандарта
target_compile_features(${KLEVRET_DHCP_TARGET} PRIVATE cxx_std_23)

# Опции компиляции
target_compile_options(${KLEVRET_DHCP_TARGET}
    PRIVATE ${KLEVRET_COMMON_COMPILE_OPTIONS}
)

# Остальные параметры
set_target_properties(${KLEVRET_DHCP_TARGET} PROPERTIES
    CXX_EXTENSIONS OFF
    POSITION_INDEPENDENT_CODE ON
)

# ~~~ dhcp-server зависимости ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ >>>

target_link_libraries(${KLEVRET_DHCP_TARGET}
    PRIVATE
        Boost::asio
        Boost::algorithm
        Boost::property_tree
        Boost::json
        #common
)
# ~~~ тесты ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ >>>

if(KLEVRET_BUILD_TESTS)
    add_subdirectory(tests)
endif()

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ >>>
