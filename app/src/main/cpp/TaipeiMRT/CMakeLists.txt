cmake_minimum_required(VERSION 3.10)

add_library(taipei_mrt STATIC)

target_sources(taipei_mrt PRIVATE
        prettify/prettify.cpp
        routing/routing.cpp
        utils/arrival_times.cpp
        utils/basic.cpp
        utils/cost.cpp
        utils/interchange.cpp
        utils/path_duration.cpp
)

target_include_directories(taipei_mrt PUBLIC
        prettify
        routing
        utils
)
