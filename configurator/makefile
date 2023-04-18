APP = ntfyDesktopConfigurator
HEADERS = mainApplication/mainWindow/mainWindow.h mainApplication/mainWindow/notebookTab.h mainApplication/mainApplication.h errorApplication/errorWindow/errorWindow.h errorApplication/errorApplication.h config/config.h
IMPLEMENTATIONS = mainApplication/mainWindow/mainWindow.cpp mainApplication/mainWindow/notebookTab.cpp mainApplication/mainApplication.cpp errorApplication/errorWindow/errorWindow.cpp errorApplication/errorApplication.cpp config/config.cpp
UI = mainApplication/mainApplication.ui mainApplication/mainWindow/toolbar.ui mainApplication/mainWindow/notebookTab.ui
COMPILER = g++
LIBS = `pkg-config --libs --cflags gtkmm-3.0`
STD = -std=c++11

${APP}:: resources.xml ${UI}
	glib-compile-resources --target=resources.cpp --generate-source resources.xml
	glib-compile-resources --target=resources.h --generate-header resources.xml
${APP}:: ${APP}.cpp ${HEADERS} ${IMPLEMENTATIONS} resources.xml ${UI}
	${COMPILER} ${APP}.cpp resources.cpp ${IMPLEMENTATIONS} -o ${APP} ${LIBS} ${STD}
${APP}::
	./${APP}
clean:
	touch ${APP} && rm ${APP}
	touch resources.cpp && rm resources.cpp
	touch resources.h && rm resources.h