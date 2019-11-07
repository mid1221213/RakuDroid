
cmake_minimum_required(VERSION 3.4.1)

add_library(native-lib SHARED native-lib.cpp)

add_library(rakudroid SHARED IMPORTED)

set(SHARED_LIB_PATH ${PROJECT_SOURCE_DIR}/../jniLibs/${ANDROID_ABI})

set_target_properties(rakudroid PROPERTIES IMPORTED_LOCATION ${SHARED_LIB_PATH}/librakudroid.so)

find_library(log-lib log)

target_link_libraries(
        native-lib
		${SHARED_LIB_PATH}/librakudroid.so
        ${log-lib}
        )
