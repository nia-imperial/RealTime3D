//
// Created by Nic on 26/02/2022.
//

#ifndef REALTIME3D_OPENGLVIEWPORT_H
#define REALTIME3D_OPENGLVIEWPORT_H


#include <QOpenGLWidget>
#include <QOpenGLFunctions>

class OpenGLViewport : public QOpenGLWidget, protected QOpenGLFunctions
{
public:
    explicit OpenGLViewport(QWidget *parent = nullptr);

protected:
    void initializeGL() override;

    void paintGL() override;

    void resizeGL(int w, int h) override;

};


#endif //REALTIME3D_OPENGLVIEWPORT_H
