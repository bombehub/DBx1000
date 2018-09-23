cmake_minimum_required(VERSION 2.8)
project(Dbx1000)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "-pg -g ${CMAKE_CXX_FLAGS}")
SET(LINK_FLAGS "-pg ${LINK_FLAGS}")
add_definitions(-D NOGRAPHITE=1)

set(SOURCE_FILES
        benchmarks/test.h
        benchmarks/test_txn.cpp
        benchmarks/test_wl.cpp
        benchmarks/tpcc.h
        benchmarks/tpcc_const.h
        benchmarks/tpcc_helper.cpp
        benchmarks/tpcc_helper.h
        benchmarks/tpcc_query.cpp
        benchmarks/tpcc_query.h
        benchmarks/tpcc_txn.cpp
        benchmarks/tpcc_wl.cpp
        benchmarks/ycsb.h
        benchmarks/ycsb_query.cpp
        benchmarks/ycsb_query.h
        benchmarks/ycsb_txn.cpp
        benchmarks/ycsb_wl.cpp
        concurrency_control/dl_detect.cpp
        concurrency_control/dl_detect.h
        concurrency_control/hekaton.cpp
        concurrency_control/occ.cpp
        concurrency_control/occ.h
        concurrency_control/plock.cpp
        concurrency_control/plock.h
        concurrency_control/row_hekaton.cpp
        concurrency_control/row_hekaton.h
        concurrency_control/row_lock.cpp
        concurrency_control/row_lock.h
        concurrency_control/row_mvcc.cpp
        concurrency_control/row_mvcc.h
        concurrency_control/row_occ.cpp
        concurrency_control/row_occ.h
        concurrency_control/row_silo.cpp
        concurrency_control/row_silo.h
        concurrency_control/row_tictoc.cpp
        concurrency_control/row_tictoc.h
        concurrency_control/row_ts.cpp
        concurrency_control/row_ts.h
        concurrency_control/row_vll.cpp
        concurrency_control/row_vll.h
        concurrency_control/silo.cpp
        concurrency_control/tictoc.cpp
        storage/catalog.cpp
        storage/catalog.h
        storage/index_base.h
        storage/index_btree.cpp
        storage/index_btree.h
        storage/index_hash.cpp
        storage/index_hash.h
        storage/row.cpp
        storage/row.h
        storage/table.cpp
        storage/table.h
        system/global.cpp
        system/global.h
        system/helper.cpp
        system/helper.h
        system/main.cpp
        system/manager.cpp
        system/manager.h
        system/mem_alloc.cpp
        system/mem_alloc.h
        system/parser.cpp
        system/query.cpp
        system/query.h
        system/stats.cpp
        system/stats.h
        system/thread.cpp
        system/thread.h
        system/txn.cpp
        system/txn.h
        system/wl.cpp
        system/wl.h
        config-std.h
        config.cpp
        config.h)

add_executable(Dbx1000 ${SOURCE_FILES})
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR} ./benchmarks/ ./concurrency_control/ ./storage/ ./system/)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})


find_package(Threads)
target_link_libraries(Dbx1000 ${CMAKE_THREAD_LIBS_INIT})

#include_directories(/usr/include/glib-2.0 /usr/lib/x86_64-linux-gnu/glib-2.0/include)
#find_package(PkgConfig REQUIRED)
#pkg_check_modules(GLIB_PKG glib-2.0)
#include_directories(${GLIB_PKG_INCLUDE_DIRS})
#target_link_libraries (Dbx1000 glib-2.0)
