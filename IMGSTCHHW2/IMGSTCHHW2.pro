#-------------------------------------------------
#
# Project created by QtCreator 2015-04-16T21:16:41
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = IMGSTCHHW2
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    mystitch.cpp \
    estimate.cpp \
    exif.cpp \
    getF.cpp \
    myblend.cpp \
    qsmartgraphicsview.cpp

HEADERS  += mainwindow.h \
    mystitch.h \
    estimate.h \
    exif.h \
    getF.h \
    featureproperties.h \
    myblend.h \
    qsmartgraphicsview.h

FORMS    += mainwindow.ui
INCLUDEPATH += $$quote(D:\libraries\opencv249o\include)\
               $$quote(D:\libraries\opencv249o\include\opencv2)

OPENCVLIB += $$quote(D:\libraries\opencv249o\x64\vc12\lib)

CONFIG(debug, debug|release){
LIBS+= $$OPENCVLIB/opencv_core249d.lib\
       $$OPENCVLIB/opencv_highgui249d.lib\
       $$OPENCVLIB/opencv_features2d249d.lib\
       $$OPENCVLIB/opencv_flann249d.lib\
       $$OPENCVLIB/opencv_nonfree249d.lib\
       $$OPENCVLIB/opencv_imgproc249d.lib\
       $$OPENCVLIB/opencv_ocl249d.lib\
       $$OPENCVLIB/opencv_stitching249d.lib
}

CONFIG(release, debug|release){
LIBS+= $$OPENCVLIB/opencv_core249.lib\
       $$OPENCVLIB/opencv_highgui249.lib\
       $$OPENCVLIB/opencv_features2d249.lib\
       $$OPENCVLIB/opencv_flann249.lib\
       $$OPENCVLIB/opencv_nonfree249.lib\
       $$OPENCVLIB/opencv_imgproc249.lib\
       $$OPENCVLIB/opencv_ocl249.lib\
       $$OPENCVLIB/opencv_stitching249.lib
}

DEFINES += HAVE_OPENCV
