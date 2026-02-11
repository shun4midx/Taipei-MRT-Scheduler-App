cmake_minimum_required(VERSION 3.22.1)

project("mrt")

add_subdirectory(TaipeiMRT)

# JNI glue library
add_library(
        mrt
        SHARED
        native-lib.cpp
)

# Include your headers
target_include_directories(mrt PRIVATE
        TaipeiMRT
)

# Link your MRT engine
target_link_libraries(
        mrt
        taipei_mrt
        android
        log
)
