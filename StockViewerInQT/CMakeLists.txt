cmake_minimum_required(VERSION 3.19)

project(StockViewerInQT VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_AUTORCC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS
    Quick
    Network
    Charts
    Qml
    QuickControls2
)

qt_standard_project_setup()

qt_add_executable(appStockViewerInQT
    main.cpp
)

qt_add_qml_module(appStockViewerInQT
    URI StockViewerInQT
    QML_FILES
        Main.qml
        SOURCES stockmodel.h stockmodel.cpp
        SOURCES stockfetcher.h stockfetcher.cpp
        RESOURCES
        RESOURCES res.qrc
        SOURCES stockprovider.h stockprovider.cpp
        SOURCES stockhistorystore.h stockhistorystore.cpp
        SOURCES stockhistorymodel.h stockhistorymodel.cpp
        SOURCES watchlistmodel.h watchlistmodel.cpp
        SOURCES signalengine.h signalengine.cpp
        SOURCES alertmodel.h alertmodel.cpp
        SOURCES logger.h logger.cpp
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appStockViewerInQT PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appStockViewerInQT
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appStockViewerInQT
    PRIVATE Qt6::Quick
            Qt6::Network
            Qt6::Charts
)

install(TARGETS appStockViewerInQT
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_generate_deploy_app_script(
    TARGET appStockViewerInQT
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})