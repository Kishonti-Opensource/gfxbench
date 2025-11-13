set(INCLUDE_DIR ${INCLUDE_DIR}
    ${CMAKE_CURRENT_LIST_DIR}
)

set(HEADERS ${HEADERS}
    ${CMAKE_CURRENT_LIST_DIR}/ApiModel.h
    ${CMAKE_CURRENT_LIST_DIR}/BenchmarkPage.h
    ${CMAKE_CURRENT_LIST_DIR}/ChartPainter.h
    ${CMAKE_CURRENT_LIST_DIR}/ComparePage.h
    ${CMAKE_CURRENT_LIST_DIR}/ConcatProxyModel.h
    ${CMAKE_CURRENT_LIST_DIR}/ConfigurationListModel.h
    ${CMAKE_CURRENT_LIST_DIR}/CursorListModel.h
    ${CMAKE_CURRENT_LIST_DIR}/Dictionary.h
    ${CMAKE_CURRENT_LIST_DIR}/EULADialog.h
    ${CMAKE_CURRENT_LIST_DIR}/GroupProxyModel.h
    ${CMAKE_CURRENT_LIST_DIR}/HomePage.h
    ${CMAKE_CURRENT_LIST_DIR}/InfoPage.h 
    ${CMAKE_CURRENT_LIST_DIR}/LoadingScreen.h
    ${CMAKE_CURRENT_LIST_DIR}/MainWindow.h
    ${CMAKE_CURRENT_LIST_DIR}/OptionsPage.h
    ${CMAKE_CURRENT_LIST_DIR}/ResultDetailPage.h
    ${CMAKE_CURRENT_LIST_DIR}/ResultsPage.h
    ${CMAKE_CURRENT_LIST_DIR}/SplashScreen.h
    ${CMAKE_CURRENT_LIST_DIR}/TestMode.h
    ${CMAKE_CURRENT_LIST_DIR}/TestListModel.h
)

set(SOURCE ${SOURCE}
    ${CMAKE_CURRENT_LIST_DIR}/ApiModel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/main.cpp
    ${CMAKE_CURRENT_LIST_DIR}/BenchmarkPage.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ChartPainter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ComparePage.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ConcatProxyModel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ConfigurationListModel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/CursorListModel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Dictionary.cpp
    ${CMAKE_CURRENT_LIST_DIR}/EULADialog.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GroupProxyModel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/HomePage.cpp
    ${CMAKE_CURRENT_LIST_DIR}/InfoPage.cpp
    ${CMAKE_CURRENT_LIST_DIR}/LoadingScreen.cpp
    ${CMAKE_CURRENT_LIST_DIR}/MainWindow.cpp
    ${CMAKE_CURRENT_LIST_DIR}/OptionsPage.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ResultDetailPage.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ResultsPage.cpp
    ${CMAKE_CURRENT_LIST_DIR}/SplashScreen.cpp
    ${CMAKE_CURRENT_LIST_DIR}/TestListModel.cpp
)

set(MOC_FILES ${MOC_FILES}
    ${CMAKE_CURRENT_LIST_DIR}/BenchmarkPage.h
    ${CMAKE_CURRENT_LIST_DIR}/ComparePage.h
    ${CMAKE_CURRENT_LIST_DIR}/ConcatProxyModel.h
    ${CMAKE_CURRENT_LIST_DIR}/ConfigurationListModel.h
    ${CMAKE_CURRENT_LIST_DIR}/CursorListModel.h
    ${CMAKE_CURRENT_LIST_DIR}/Dictionary.h
    ${CMAKE_CURRENT_LIST_DIR}/EULADialog.h
    ${CMAKE_CURRENT_LIST_DIR}/GroupProxyModel.h
    ${CMAKE_CURRENT_LIST_DIR}/HomePage.h
    ${CMAKE_CURRENT_LIST_DIR}/InfoPage.h
    ${CMAKE_CURRENT_LIST_DIR}/MainWindow.h
    ${CMAKE_CURRENT_LIST_DIR}/LoadingScreen.h
    ${CMAKE_CURRENT_LIST_DIR}/OptionsPage.h
    ${CMAKE_CURRENT_LIST_DIR}/ResultDetailPage.h
    ${CMAKE_CURRENT_LIST_DIR}/ResultsPage.h
    ${CMAKE_CURRENT_LIST_DIR}/SplashScreen.h
    ${CMAKE_CURRENT_LIST_DIR}/TestListModel.h
)

set(UI_FILES ${UI_FILES}
    ${RESOURCE_DIR}/forms/BenchmarkPage.ui
    ${RESOURCE_DIR}/forms/HomePage.ui
    ${RESOURCE_DIR}/forms/ComparePage.ui
    ${RESOURCE_DIR}/forms/CompareWidget.ui
    ${RESOURCE_DIR}/forms/EULADialog.ui
    ${RESOURCE_DIR}/forms/InfoPage.ui
    ${RESOURCE_DIR}/forms/LoadingScreen.ui
    ${RESOURCE_DIR}/forms/MainWindow.ui
    ${RESOURCE_DIR}/forms/OptionsPage.ui
    ${RESOURCE_DIR}/forms/ResultDetailPage.ui
    ${RESOURCE_DIR}/forms/ResultsPage.ui
    ${RESOURCE_DIR}/forms/SplashScreen.ui
)



if(WIN32)
    set(SOURCE ${SOURCE}
        ${CMAKE_CURRENT_LIST_DIR}/TestMode_win.cpp
    )
elseif(APPLE)
    set(SOURCE ${SOURCE}
        ${CMAKE_CURRENT_LIST_DIR}/TestMode_osx.mm
    )
else()
    set(SOURCE ${SOURCE}
        ${CMAKE_CURRENT_LIST_DIR}/TestMode_linux.cpp
    )
endif()
