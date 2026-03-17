#pragma once
#include <GL/glut.h>
#include <cmath> // fabs
#include <gl/GL.h>

class Bala {
private:
    double x;
    double y;
    double incrementox;
    double incrementoy;
    bool collision = false;
    bool out = false;
public:
    Bala(double posX, double posY, double incX, double incY) : x(posX), y(posY), incrementox(incX), incrementoy(incY) {}
    void Update(int EnemigoX, int EnemigoY, int w, int h) {
        x += incrementox;
        y += incrementoy;
        //Me fijo si hay colision de la bala con una torre enemiga
        if (fabs(x - EnemigoX) + fabs(y - EnemigoY) < 50) {
            //Energia -= 10;
            collision = true;
        }
        //Si esta fuera de la pantalla, elimino la bala
        if (x > w || x < 0 || y > h || y < 0)
            out = true;
    }
    void Draw() {
        //glColor3f(1.0, 1.0, 1.0); // blanco
        //glPointSize(8);
        //glBegin(GL_POINTS);
        glVertex2d(x, y);
        //glEnd();
    }

    double GetX() {
        return x;
    }

    double GetY() {
        return y;
    }
    bool GetOut() {
        return out;
    }

    bool GetCollision() {
        return collision;
    }
};