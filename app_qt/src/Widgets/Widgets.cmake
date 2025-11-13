set(INCLUDE_DIR ${INCLUDE_DIR}
	${CMAKE_CURRENT_LIST_DIR}
)

set(HEADERS ${HEADERS}
	${CMAKE_CURRENT_LIST_DIR}/MessageBox.h
	${CMAKE_CURRENT_LIST_DIR}/DuelWidget.h
	${CMAKE_CURRENT_LIST_DIR}/ChartWidget.h
	${CMAKE_CURRENT_LIST_DIR}/CompareWidget.h
	${CMAKE_CURRENT_LIST_DIR}/ScrollWidget.h
	${CMAKE_CURRENT_LIST_DIR}/PushButton.h
	${CMAKE_CURRENT_LIST_DIR}/SelectableScrollWidget.h
	${CMAKE_CURRENT_LIST_DIR}/SelectableWidget.h
	${CMAKE_CURRENT_LIST_DIR}/DescriptionWidget.h
	${CMAKE_CURRENT_LIST_DIR}/ItemScrollWidget.h
	${CMAKE_CURRENT_LIST_DIR}/DeviceWidget.h
	${CMAKE_CURRENT_LIST_DIR}/GroupWidget.h
	${CMAKE_CURRENT_LIST_DIR}/TestWidget.h
	${CMAKE_CURRENT_LIST_DIR}/InfoWidget.h
	${CMAKE_CURRENT_LIST_DIR}/InfoDetailWidget.h
	${CMAKE_CURRENT_LIST_DIR}/ResultsHistoryItemWidget.h
	${CMAKE_CURRENT_LIST_DIR}/ApiSelector.h
)

set(SOURCE ${SOURCE}
	${CMAKE_CURRENT_LIST_DIR}/MessageBox.cpp
	${CMAKE_CURRENT_LIST_DIR}/DuelWidget.cpp
	${CMAKE_CURRENT_LIST_DIR}/ChartWidget.cpp
	${CMAKE_CURRENT_LIST_DIR}/CompareWidget.cpp
	${CMAKE_CURRENT_LIST_DIR}/ScrollWidget.cpp
	${CMAKE_CURRENT_LIST_DIR}/PushButton.cpp
	${CMAKE_CURRENT_LIST_DIR}/SelectableScrollWidget.cpp
	${CMAKE_CURRENT_LIST_DIR}/SelectableWidget.cpp
	${CMAKE_CURRENT_LIST_DIR}/DescriptionWidget.cpp
	${CMAKE_CURRENT_LIST_DIR}/ItemScrollWidget.cpp
	${CMAKE_CURRENT_LIST_DIR}/DeviceWidget.cpp
	${CMAKE_CURRENT_LIST_DIR}/GroupWidget.cpp
	${CMAKE_CURRENT_LIST_DIR}/TestWidget.cpp
	${CMAKE_CURRENT_LIST_DIR}/InfoWidget.cpp
	${CMAKE_CURRENT_LIST_DIR}/InfoDetailWidget.cpp
	${CMAKE_CURRENT_LIST_DIR}/ResultsHistoryItemWidget.cpp
	${CMAKE_CURRENT_LIST_DIR}/ApiSelector.cpp
)

set(MOC_FILES ${MOC_FILES}
	${CMAKE_CURRENT_LIST_DIR}/DuelWidget.h
	${CMAKE_CURRENT_LIST_DIR}/ChartWidget.h
	${CMAKE_CURRENT_LIST_DIR}/CompareWidget.h
	${CMAKE_CURRENT_LIST_DIR}/ScrollWidget.h
	${CMAKE_CURRENT_LIST_DIR}/PushButton.h
	${CMAKE_CURRENT_LIST_DIR}/SelectableScrollWidget.h
	${CMAKE_CURRENT_LIST_DIR}/SelectableWidget.h
	${CMAKE_CURRENT_LIST_DIR}/DescriptionWidget.h
	${CMAKE_CURRENT_LIST_DIR}/ItemScrollWidget.h
	${CMAKE_CURRENT_LIST_DIR}/DeviceWidget.h
	${CMAKE_CURRENT_LIST_DIR}/GroupWidget.h
	${CMAKE_CURRENT_LIST_DIR}/TestWidget.h
	${CMAKE_CURRENT_LIST_DIR}/InfoWidget.h
	${CMAKE_CURRENT_LIST_DIR}/InfoDetailWidget.h
	${CMAKE_CURRENT_LIST_DIR}/ResultsHistoryItemWidget.h
	${CMAKE_CURRENT_LIST_DIR}/ApiSelector.h
)

set(UI_FILES ${UI_FILES}
	${RESOURCE_DIR}/forms/DuelWidget.ui
	${RESOURCE_DIR}/forms/CompareWidget.ui
	${RESOURCE_DIR}/forms/ScrollWidget.ui
	${RESOURCE_DIR}/forms/DescriptionWidget.ui
	${RESOURCE_DIR}/forms/DeviceWidget.ui
	${RESOURCE_DIR}/forms/GroupWidget.ui
	${RESOURCE_DIR}/forms/TestWidget.ui
	${RESOURCE_DIR}/forms/InfoWidget.ui
	${RESOURCE_DIR}/forms/InfoDetailWidget.ui
	${RESOURCE_DIR}/forms/ResultsHistoryItemWidget.ui
	${RESOURCE_DIR}/forms/ApiSelector.ui
)
