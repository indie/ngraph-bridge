# ******************************************************************************
# Copyright 2017-2019 Intel Corporation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ******************************************************************************

include_directories(${GTEST_INCLUDE_DIR})
include_directories(${TensorFlow_INCLUDE_DIR})
include_directories(${TensorFlow_INCLUDE_DIR}/external/nsync/public)

# Get this location from the CMAKE variables passed during the cmake time
if (NOT DEFINED TF_SRC_DIR)
   message(
       FATAL_ERROR
       "TensorFlow source directory.Use -DTF_SRC_DIR=<tf source directory>"
    )
endif()


include_directories(${TensorFlow_SRC_DIR})

# Files that are generated during TF build are here
include_directories(${TensorFlow_SRC_DIR}/bazel-genfiles)

add_library(tensorflow_cc_lib SHARED IMPORTED)
set_target_properties(
    tensorflow_cc_lib
    PROPERTIES IMPORTED_LOCATION
    ${TensorFlow_SRC_DIR}/bazel-bin/tensorflow/libtensorflow_cc.so.1
)

set(SRC
    main.cpp
    graph_exec.cpp
    tf_exec.cpp
    padding.cpp
    conversions.cpp
    encapsulate_op/encapsulate_op_test.cc
    graph_rewrites/assign_clusters.cc
    graph_rewrites/deadness_test.cc
    graph_rewrites/backend_manager_test.cc
    graph_rewrites/encapsulate_clusters_test.cc
    graph_rewrites/disable_ops_test.cc
    graph_rewrites/mark_for_clustering_test.cc
    test_index_library.cpp
    test_utilities.cpp
    test_math_ops.cpp
    test_nn_ops.cpp
    test_array_ops.cpp
    test_variable_ops.cpp
    test_momentum_op.cpp
    opexecuter.cpp
)

if(NGRAPH_TF_ENABLE_VARIABLES_AND_OPTIMIZERS)
    list(APPEND SRC graph_rewrites/capture_variables.cc)
    list(APPEND SRC graph_rewrites/enter_in_catalog_test.cc)
    list(APPEND SRC graph_rewrites/remove_ngraphassigns.cc)
    list(APPEND SRC graph_rewrites/test_replace_optimizer.cpp)
endif()

if(NGRAPH_TF_USE_GRAPPLER_OPTIMIZER)
    list(APPEND SRC graph_rewrites/config_for_grappler_test.cc)
    list(APPEND SRC graph_rewrites/partial_shapes_test.cc)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

message(STATUS "Compile Flags: ${CMAKE_CXX_FLAGS}")
message(STATUS "Shared Link Flags: ${CMAKE_SHARED_LINKER_FLAGS}")

add_executable(gtest_ngtf ${SRC})
message(STATUS "TensorFlow_SRC_DIR: ${TensorFlow_SRC_DIR}")

# The following custom commands are used to create symlinks for various
# input files needed by the unit test
add_custom_command(
    TARGET gtest_ngtf
    PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_CURRENT_SOURCE_DIR}/test_axpy.pbtxt
        ${CMAKE_CURRENT_BINARY_DIR}/test_axpy.pbtxt
    COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_CURRENT_SOURCE_DIR}/test_axpy_launchop.pbtxt
        ${CMAKE_CURRENT_BINARY_DIR}/test_axpy_launchop.pbtxt
)

if (APPLE)
    set(NGRAPH_TF_CXX11_ABI 0)
endif()

if(NGRAPH_BRIDGE_STATIC_LIB_ENABLE)
    target_link_libraries(gtest_ngtf
        -Wl,--whole-archive
            ngraph_bridge_static
        -Wl,--no-whole-archive
        ngraph_lib
        lib_cpu_backend_static
        lib_interpreter_backend_static 
        ngraph_lib
        dl
        libgtest
        pthread
        ${TensorFlow_FRAMEWORK_LIBRARY}
        tensorflow_cc_lib
    )
else()
    target_link_libraries(
        gtest_ngtf
        ngraph_bridge
        ngraph_lib
        libgtest
        pthread
        ${TensorFlow_FRAMEWORK_LIBRARY}
        tensorflow_cc_lib
    )
endif()

if (NGRAPH_PLAIDML_ENABLE)
    target_link_libraries(gtest_ngtf ${PLAIDML_LIBRARIES})
endif()

add_subdirectory(python)
add_subdirectory(python/bfloat16)
add_subdirectory(model_level_tests)

if (DEFINED NGRAPH_TF_INSTALL_PREFIX)
    set(CMAKE_INSTALL_PREFIX ${NGRAPH_TF_INSTALL_PREFIX})
else()
    set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/../install/")
endif()

# First install the libngraph_bridge.so and headers
install(TARGETS gtest_ngtf DESTINATION ${CMAKE_INSTALL_PREFIX}/test)  
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/test_axpy.pbtxt DESTINATION ${CMAKE_INSTALL_PREFIX}/test)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/test_axpy_launchop.pbtxt DESTINATION ${CMAKE_INSTALL_PREFIX}/test)
install(
    DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/python 
    DESTINATION ${CMAKE_INSTALL_PREFIX}/test
    FILES_MATCHING PATTERN "*.py"
)
install(
    DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/python
    DESTINATION ${CMAKE_INSTALL_PREFIX}/test
    FILES_MATCHING PATTERN "*.pbtxt"
)
install(
    DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/python
    DESTINATION ${CMAKE_INSTALL_PREFIX}/test
    FILES_MATCHING PATTERN "*.pb"
)

install(
    DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/model_level_tests
    DESTINATION ${CMAKE_INSTALL_PREFIX}/test
)
