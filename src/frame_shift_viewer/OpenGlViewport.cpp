//
// Created by Nic on 26/02/2022.
//

#include "OpenGlViewport.h"

OpenGLViewport::OpenGLViewport(QWidget *parent) : QOpenGLWidget(parent) {
    setAttribute(Qt::WA_AlwaysStackOnTop);
}

void OpenGLViewport::initializeGL() {
    initializeOpenGLFunctions();
//    glClearColor();
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glClearColor(1, 0, 0, 1); //sets a red background
    glEnable(GL_DEPTH_TEST);
}

void OpenGLViewport::paintGL() {
    // Draw the scene:
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    glClear(GL_COLOR_BUFFER_BIT);
}

void OpenGLViewport::resizeGL(int w, int h) {
    QOpenGLWidget::resizeGL(w, h);
}

