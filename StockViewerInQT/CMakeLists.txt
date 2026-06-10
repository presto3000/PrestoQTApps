cmake_minimum_required(VERSION 3.19)

project(StockViewerInQT VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS
    Quick
    Network
    Charts
    Qml
    QuickControls2
    WebSockets
    Test
)

qt_standard_project_setup()

qt_add_executable(appStockViewerInQT
    main.cpp
)

qt_add_qml_module(appStockViewerInQT
    URI StockViewerInQT
    QML_FILES
        Main.qml
    RESOURCES
        res.qrc
    SOURCES
        stockmodel.h stockmodel.cpp
        stockfetcher.h stockfetcher.cpp
        stockprovider.h stockprovider.cpp
        stockhistorystore.h stockhistorystore.cpp
        stockhistorymodel.h stockhistorymodel.cpp
        watchlistmodel.h watchlistmodel.cpp
        signalengine.h signalengine.cpp
        alertmodel.h alertmodel.cpp
        logger.h logger.cpp
        alpacawebsocket.h alpacawebsocket.cpp
        alpacapositionprovider.h alpacapositionprovider.cpp
        ipositionprovider.h
        SOURCES positionmodel.h positionmodel.cpp
        SOURCES tradetickmodel.h tradetickmodel.cpp
        SOURCES finnhubwebsocket.h finnhubwebsocket.cpp
)

set_target_properties(appStockViewerInQT PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appStockViewerInQT
    PRIVATE Qt6::Quick
            Qt6::Network
            Qt6::Charts
            Qt6::WebSockets
            Qt6::Test
            Qt6::QuickControls2
)

add_custom_command(TARGET appStockViewerInQT POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_SOURCE_DIR}/Keys.env
    $<TARGET_FILE_DIR:appStockViewerInQT>
)