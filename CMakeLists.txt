cmake_minimum_required(VERSION 3.16)
project(RecordingPlugin VERSION "0.8.4")

set(CMAKE_CXX_STANDARD 17)

set(UnityDir Unity)
set(SrcDir Src)
set(ExternalDir External)
set(TestDir Test)
set(ConvertDir Convert)
set(AnalysisTestDir AnalysisTest)
set(AnalysisDir Analysis)
set(UtilsDir Utils)

configure_file(Config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/${SrcDir}/Config.h @ONLY)

if(ANDROID_BUILD)
    set( UNITY_PLUGINS_DIR "C:/Users/Anton-Lammert/Desktop/ImmersiveStudyAnalyzer/Assets/Plugins/Android/ARM64" )
else()
    set( UNITY_PLUGINS_DIR "C:/Users/Anton-Lammert/Desktop/ImmersiveStudyAnalyzer/Assets/Plugins/Windows" )
endif()

if(NOT WORKFLOW)
    if(EXISTS ${UNITY_PLUGINS_DIR})
        set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${UNITY_PLUGINS_DIR} )
        set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${UNITY_PLUGINS_DIR} )
        set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${UNITY_PLUGINS_DIR} )
    endif()
endif()

    # find all source files
file(GLOB_RECURSE SRC_FILES
        ${SrcDir}/*.h.in
        ${SrcDir}/*.cpp
        ${SrcDir}/*.h
        ${UnityDir}/*.cpp
        ${UnityDir}/*.h
        External/AudioFile/*.h
        External/ThreadPool/*.hpp
        )

file(GLOB_RECURSE TEST_FILES
        ${SrcDir}/*.h.in
        ${SrcDir}/*.cpp
        ${SrcDir}/*.h
        ${UnityDir}/*.cpp
        ${UnityDir}/*.h
        ${TestDir}/Test.cpp
        ${UtilsDir}/*.cpp
        ${UtilsDir}/*.h
        External/AudioFile/*.h
        External/ThreadPool/*.hpp
        )

file(GLOB_RECURSE CONVERT_FILES
        ${SrcDir}/*.h.in
        ${SrcDir}/*.cpp
        ${SrcDir}/*.h
        ${UnityDir}/*.cpp
        ${UnityDir}/*.h
        ${ConvertDir}/Convert.cpp
        ${UtilsDir}/*.cpp
        ${UtilsDir}/*.h
        External/AudioFile/*.h
        External/ThreadPool/*.hpp
        )

file(GLOB_RECURSE CONVERT_BATCH_FILES
        ${SrcDir}/*.h.in
        ${SrcDir}/*.cpp
        ${SrcDir}/*.h
        ${UnityDir}/*.cpp
        ${UnityDir}/*.h
        ${ConvertDir}/ConvertBatch.cpp
        ${UtilsDir}/*.cpp
        ${UtilsDir}/*.h
        External/AudioFile/*.h
        External/ThreadPool/*.hpp
        )

file(GLOB_RECURSE ANALYSIS_TEST_FILES
        ${SrcDir}/*.h.in
        ${SrcDir}/*.cpp
        ${SrcDir}/*.h
        ${UnityDir}/*.cpp
        ${UnityDir}/*.h
        ${AnalysisTestDir}/*.cpp
        ${AnalysisTestDir}/*.h
        External/AudioFile/*.h
        External/ThreadPool/*.hpp
        )


file(GLOB_RECURSE GAZE_ANALYSIS_FILES
        ${SrcDir}/*.h.in
        ${SrcDir}/*.cpp
        ${SrcDir}/*.h
        ${UnityDir}/*.cpp
        ${UnityDir}/*.h
        #${AnalysisDir}/*.cpp
        #${AnalysisDir}/*.h
        External/AudioFile/*.h
        External/ThreadPool/*.hpp
)


include_directories(${SrcDir} ${UnityDir})

add_subdirectory(External/glm EXCLUDE_FROM_ALL)
add_subdirectory(External/zlib)

include_directories(${CMAKE_SOURCE_DIR}/External/zlib ${CMAKE_BINARY_DIR}/External/zlib)

if(NOT ANDROID_BUILD)
    add_subdirectory(External/Catch2)

    set(TEST_LIBRARIES PRIVATE glm Catch2::Catch2 zlibstatic)

    add_executable(tests ${TEST_FILES})
    target_link_libraries(tests ${TEST_LIBRARIES})

    
    add_executable(convertBatch ${CONVERT_BATCH_FILES})
    target_link_libraries(convertBatch ${TEST_LIBRARIES})

    add_executable(convert ${CONVERT_FILES})
    target_link_libraries(convert ${TEST_LIBRARIES})

    add_executable(analysis_tests ${ANALYSIS_TEST_FILES})
    target_link_libraries(analysis_tests ${TEST_LIBRARIES})

    add_executable(gaze_analysis ${AnalysisDir}/GazeAnalysis.cpp ${GAZE_ANALYSIS_FILES})
    target_link_libraries(gaze_analysis ${TEST_LIBRARIES})

    add_executable(get_transforms_for_gaze_analysis ${AnalysisDir}/GetTransformsForGazeAnalysis.cpp ${GAZE_ANALYSIS_FILES})
    target_link_libraries(get_transforms_for_gaze_analysis ${TEST_LIBRARIES})

endif()

set(COMMON_LIBRARIES PRIVATE glm zlibstatic)

# create a shared library (.dll/.so)
add_library(${PROJECT_NAME} SHARED ${SRC_FILES})
target_link_libraries(${PROJECT_NAME} ${COMMON_LIBRARIES})