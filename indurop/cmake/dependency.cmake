
include(${CMAKE_CURRENT_LIST_DIR}/preinstall_dependency.cmake)

# rapidjson
preinstall_dependency(RapidJSON
	${CMAKE_SOURCE_DIR}/dependency/rapidjson
	CONFIG_ARGS
		"-DRAPIDJSON_BUILD_DOC=OFF"
		"-DRAPIDJSON_BUILD_EXAMPLES=OFF"
		"-DRAPIDJSON_BUILD_TESTS=OFF"
		"-DRAPIDJSON_BUILD_CXX11=OFF"
)
set(RapidJSON_DIR ${RapidJSON_ROOT}/cmake)

# bicomc
preinstall_dependency(BiCOMC
	${CMAKE_SOURCE_DIR}/dependency/bicomc
	CONFIG_ARGS
		"-DBiCOMC_BUILD_EXAMPLE=OFF"
)
set(BiCOMC_DIR ${BiCOMC_ROOT}/lib/cmake/BiCOMC)
