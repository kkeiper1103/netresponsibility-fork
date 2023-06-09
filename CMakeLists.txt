cmake_minimum_required(VERSION 3.17)
project(net-responsibility)

set(CMAKE_CXX_STANDARD 20)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

# find package stuff
# !!this has got to be built with vcpkg!!
find_package(Poco REQUIRED COMPONENTS DataSQLite Net Util Zip)
find_package(PCAP REQUIRED)

set(SOURCES
    src/BootHistory.cpp src/Bypasses.cpp src/ConfigSubsystem.cpp src/Database.cpp src/Filter.cpp src/History.cpp
    src/MainApplication.cpp src/MyXml.cpp src/Options.cpp src/Plugin.cpp src/ReportBase.cpp
    src/ReportSubsystem.cpp src/Request.cpp src/Sniffer.cpp src/SnifferSubsystem.cpp src/SnifferThread.cpp
    src/Warnings.cpp)

add_executable(${PROJECT_NAME} main.cpp ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE include/ ${PCAP_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE
    Poco::Foundation Poco::DataSQLite Poco::Net Poco::Util Poco::Zip ${PCAP_LIBRARY}
)


add_library(report SHARED src/Report.cpp src/ReportBase.cpp)
target_include_directories(report PRIVATE include/ ${PCAP_INCLUDE_DIR})
target_link_libraries(report PRIVATE
    Poco::Foundation Poco::DataSQLite Poco::Net Poco::Util Poco::Zip ${PCAP_LIBRARY}
)


# test stuff to try new rebuild
