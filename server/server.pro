QT += network widgets

HEADERS       = server.h
SOURCES       = server.cpp \
                main.cpp

# install
target.path = $$[QT_INSTALL_EXAMPLES]/network/server
INSTALLS += target
