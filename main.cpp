#include <iostream>
#include <cstdlib>
#include <cmath>
#include <list>
#include <string>
#include <GL/glut.h>
#include <fstream>
#include "Bala.h"

using namespace std;

//------------------------------------------------------------
// Variables globales

enum EstadoJuego { JUGANDO, PERDIDO, GANADO, TUTORIAL };  //estados del juego
EstadoJuego estado = TUTORIAL;


int w = 800, h = 600,
xMax = 750, xmin = 50,
yMax = 550, yMin = 50,
Energia = 100, EnergiaEnemigo = 100;
int EnemigoX = 350, EnemigoY = 350;  //POSICION OVNI
float velOvniX = 2.0f, velOvniY = 1.5f;  //PARA MOVER AL OVNI
double AvionX = 50, AvionY = 30, AvionAng = 0, ArmaAng = 0;  //POSICION AVION
double PlanetaX = 520, PlanetaY = 270; //POSICION PLANETA
const double PI = 4 * atan(1.0);
bool teclas[256] = { false }; // Estado de teclas
list<Bala> proyectil;  

float movimiento = 0;
bool activarLlamas = false;

//PARA EL ESCUDO
bool escudoActivo = false;
float radioEscudo = 50.0f; // Radio del escudo alrededor del avión
float opacidadEscudo = 0.7f; // Opacidad del escudo
//PARA RADAR
static int AngLineaRadar = 0;
static int AngTurbina = 0;
//ZBUFFER
const double
zTEXTO = 0.99,  // texto por encima de todo
zRADAR = 0.95,
zPROYECTIL = 0.6,
zPISTA = 0.1,
zAVION = 0.3,
zALA = 0.7,
zTURBINA = 0.2,
zPLANETA = 0.2;

unsigned int tiempoInicio = 0;

bool puedeDisparar = true;  //para limitar el disparo

GLuint texID[3]; // Almacenamos IDs de texturas

//VARIABLES PARA COLISIONES
float radioAvion = 25.0f; // ajusta según tamaño del avión
float radioOvni = 40.0f;  // coincidiendo con el gluDisk del OVNI
float radioBala = 4.0f;   // para colisión aproximada



// --- Parametros de la pista ---
int pistaX = 10;   // coordenada X inicial
int pistaY = 0;   // coordenada Y inicial
int pistaAncho = 80;  // ancho total
int pistaAlto = 120;   // alto total


// Material metálico
GLfloat mat_ambient[] = { 0.3f, 0.3f, 0.3f, 1.0f };
GLfloat mat_diffuse[] = { 0.6f, 0.6f, 0.6f, 1.0f };
GLfloat mat_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f }; // reflejo blanco fuerte
GLfloat mat_shininess[] = { 80.0f };                  // brillo concentrado

//LLAMAS
float intensidadLlamas = 0.0f; // 0 apagado, 1 máximo
struct Llama {
    float offsetX, offsetY;  // posición relativa a la cola
    float size;              // tamaño de la llama
    float alpha;             // transparencia
};

const int NUM_LLAMAS = 20;
Llama llamas[NUM_LLAMAS];

void InitLlamas() {
    for (int i = 0; i < NUM_LLAMAS; i++) {
        llamas[i].offsetX = 0;  // Centradas en X
        llamas[i].offsetY = -(40 + rand() % 60); // Hacia atrás del avión (Y negativo)
        llamas[i].size = 6 + rand() % 8;  //ancho
        llamas[i].alpha = 0.0f;
    }
}

//----------------------------------------
//PARA ESCRIBIR TEXTO

void print_text(const string& texto, int x, int y, double z = zTEXTO, void* font = GLUT_BITMAP_HELVETICA_18, int espacio = 0) {
    glRasterPos3d(x, y, z); 
    for (char c : texto) {
        glutBitmapCharacter(font, c);
        if (espacio > 0)
            glBitmap(0, 0, 0, 0, espacio, 0, nullptr);
    }
}
//FUNCION PARA CARGAR TEXTURAS-------------------------------------

bool mipmap_ppm(const char* ifile) // Carga una imagen en formato ppm, crea el canal alpha (ya que las imagenes ppm no guardan este canal), crea los mipmaps y deja la textura lista para usar en nuestro programa
{
    char dummy;
    int maxColor, wideTexture, highTexture; // Cantidad de colores, ancho y alto de texturas

    ifstream fileInput(ifile, ios::binary);
    if (!fileInput.is_open())
    {
        cerr << "No se encuentra el archivo" << endl;
        return false;
    }
    fileInput.get(dummy);

    if (dummy != 'P')
    {
        cerr << "No se encuentra P6 PPM" << endl;
        return false;
    }
    fileInput.get(dummy);

    if (dummy != '6')
    {
        cerr << "No se encuentra P6 PPM" << endl;
        return false;
    }
    fileInput.get(dummy);

    dummy = fileInput.peek();
    if (dummy == '#') do
    {
        fileInput.get(dummy);
    } while (dummy != 10);

    fileInput >> wideTexture >> highTexture; // Lee ancho y alto de textura
    fileInput >> maxColor; // Lee cantidad de colores de la imagen
    fileInput.get(dummy);

    unsigned char* image = new unsigned char[3 * wideTexture * highTexture]; // 3 bytes (RGB)
    fileInput.read((char*)image, 3 * wideTexture * highTexture);
    fileInput.close();

    unsigned char* imageAlpha = new unsigned char[4 * wideTexture * highTexture]; // 4 bytes (RGBA)
    unsigned char r, g, b;
    for (int i = 0; i < wideTexture * highTexture; i++)
    {
        r = imageAlpha[4 * i + 0] = image[3 * i + 0];
        g = imageAlpha[4 * i + 1] = image[3 * i + 1];
        b = imageAlpha[4 * i + 2] = image[3 * i + 2];
        imageAlpha[4 * i + 3] = ((r + g + b == 765) ? 0 : 255);
    }
    delete[] image;
    gluBuild2DMipmaps(GL_TEXTURE_2D, 4, wideTexture, highTexture, GL_RGBA, GL_UNSIGNED_BYTE, imageAlpha);
    delete[] imageAlpha;
    return true;
}


//------------------------FUNCIONES PARA DIBUJAR------------------------------------

void DibujarEscudo() {
    if (!escudoActivo) return;

    glColor3f(1.0, 0.0, 0.0);
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    // Dibujar texto titilante    
    float elapsedTime = glutGet(GLUT_ELAPSED_TIME) % 1500 / 1499.f; // Tiempo en milisegundos
    float opacidad = 315 / (64 * 3.1415 * pow(0.25, 9)) *
        pow((pow(0.25, 2) - pow(elapsedTime - 0.25, 2)), 3) / 12.5335 + 0.1f;
    if (opacidad > 1.0f)
        opacidad = 1.0f;
    glColor4f(1.0f, 0.0f, 0.0f, opacidad);
    print_text("ESCUDO ACTIVADO", w / 2, h - 40);
    glPopAttrib();
    

    // Guardar el estado actual de las matrices
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, w, 0, h);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glPushAttrib(GL_ALL_ATTRIB_BITS); // Guardar todos los atributos

    glTranslated(AvionX, AvionY , zAVION + 0.05f); // Justo encima del avión

    
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);

    // Escudo muy transparente - apenas visible
    glColor4f(0.4f, 0.6f, 1.0f, 0.15f); // Azul muy claro y transparente

    // Dibujar círculo simple
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(0, 0);
    for (int i = 0; i <= 360; i += 15) {
        float angle = i * PI / 180.0f;
        glVertex2f(cos(angle) * radioEscudo, sin(angle) * radioEscudo);
    }
    glEnd();

    // Borde sutil
    glColor4f(0.3f, 0.5f, 0.9f, 0.25f);
    glLineWidth(1.5f);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 360; i += 15) {
        float angle = i * PI / 180.0f;
        glVertex2f(cos(angle) * radioEscudo, sin(angle) * radioEscudo);
    }
    glEnd();

    glPopAttrib(); // Restaurar atributos para no afectar otros elementos

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void UpdateLlamas(float dt) {
    // Intensidad general (fade in/out)
    if (activarLlamas) {
        if (intensidadLlamas < 1.0f) intensidadLlamas += dt * 0.8f;
    }
    else {
        if (intensidadLlamas > 0.0f) intensidadLlamas -= dt * 2.0f;
    }
    intensidadLlamas = fmax(0.0f, fmin(1.0f, intensidadLlamas));

    // Actualiza cada llama individualmente
    for (int i = 0; i < NUM_LLAMAS; i++) {
        llamas[i].offsetY -= dt * 180; // Más rápido para llamas más largas
        llamas[i].alpha = intensidadLlamas * (1.0f - (-llamas[i].offsetY / 120.0f)); // Ajustar para mayor longitud

        if (llamas[i].offsetY < -120) { // Mayor distancia máxima
            llamas[i].offsetY = -(40 + rand() % 60); // Reinicia con longitud similar
            llamas[i].size = 10 + rand() % 10;         // Mantener estrechas
        }
    }
}
void DibujarLlamas() {
    //if (intensidadLlamas <= 0.0f) return;
    if (!activarLlamas) return;

    glColor3f(1.0, 0.0, 0.0);
    
    float colaX = 0.0f;   // centro horizontal (mismo que el cuerpo)
    float colaY = -16.0f; // parte trasera del avión (coordenada local)

    glPushMatrix();
    
    glTranslatef(colaX, colaY, 0);

    for (int i = 0; i < NUM_LLAMAS; i++) {
        // Calcular posición intermedia para mejor gradiente
        float midY = llamas[i].offsetY * 0.5f;

        glBegin(GL_TRIANGLES);
        // Punta (amarillo intenso)
        glColor4f(1.0f, 1.0f, 0.0f, llamas[i].alpha);
        glVertex2f(0, 0);

        // Medio (naranja)
        glColor4f(1.0f, 0.7f, 0.2f, llamas[i].alpha * 0.8f);
        glVertex2f(-llamas[i].size / 3, midY);
        glVertex2f(llamas[i].size / 3, midY);

        // Base (rojo anaranjado, más estrecha)
        glColor4f(1.0f, 0.3f, 0.0f, llamas[i].alpha * 0.5f);
        glVertex2f(-llamas[i].size / 4, llamas[i].offsetY);
        glVertex2f(llamas[i].size / 4, llamas[i].offsetY);
        glEnd();
    }

    glPopMatrix();
}


void DibujarCabina() {
    glColor3d(0.6, 0.7, 0.9);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2d(0, 0);
    for (double r = 0; r < 2 * PI; r += 0.1)
        glVertex2d(cos(r), sin(r));
    glEnd();
}

void DibujarCuerpo() {
    glColor3d(0.4, 0.4, 0.4);
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0, 0, 1);  // normal para reflejos
    glVertex2d(0.0, 0.0);
    glVertex2d(0.0, 70.0);
    glVertex2d(-8, 35.0);
    glVertex2d(-10, -30.0);
    glVertex2d(0.0, -15.0);
    glVertex2d(10, -30.0);
    glVertex2d(8, 35.0);
    glVertex2d(0.0, 70.0);
    glEnd();
}

void DibujarAla() {
    glColor3d(0.7, 0.7, 0.7);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2d(35, 10);
    glVertex2d(0.0, 20.0);
    glVertex2d(0.0, 0.0);
    glVertex2d(35, 4);
    glVertex2d(50.0, 0.0);
    glEnd();
}

void DibujarTurbina() {
    glColor3f(0.5, 0.5, 0.5);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(0, 0);
    for (double a = 0; a <= 2 * PI; a += 0.2)
        glVertex2f(5 * cos(a), 5 * sin(a));
    glEnd();

    glColor3f(0.2, 0.2, 0.2);
    glLineWidth(2.0);
    glPushMatrix();
    glRotated(AngTurbina, 0, 0, 1);
    glBegin(GL_LINES);
    glVertex2f(-7, 0); glVertex2f(7, 0);
    glVertex2f(0, -7); glVertex2f(0, 7);
    glEnd();
    glPopMatrix();
}

void DibujarAvion() {


    glPushMatrix();
    glTranslated(AvionX, AvionY, zAVION);
    glRotated(AvionAng, 0, 0, 1);

    glPushMatrix(); DibujarAla(); glPopMatrix();
    glPushMatrix(); glScaled(-1, 1, 1); DibujarAla(); glPopMatrix();

    glPushMatrix(); glTranslated(25, -5, zTURBINA); DibujarTurbina(); glPopMatrix();
    glPushMatrix(); glScaled(-1, 1, 1); glTranslated(25, -5, zTURBINA); DibujarTurbina(); glPopMatrix();

    DibujarCuerpo();

    glPushMatrix(); glScaled(6, 12, 1); DibujarCabina(); glPopMatrix();

    glColor3f(0.0f, 0.0f, 0.0f);
    glPointSize(5.0);
    glBegin(GL_POINTS); glVertex2d(0.0, 0.0); glEnd();

    DibujarLlamas();

    glPopAttrib();

    glPopMatrix();
}


void DibujarRadar() // Dibuja el radar con enemigo relativo
{
    glPushMatrix(); // Inicia push para el radar

    // --- Perimetro del radar ---
    glPushMatrix();
    glTranslatef(700, 500, zRADAR);
    GLUquadricObj* q = gluNewQuadric();
    glColor4f(0.1f, 0.1f, 0.1f, 1.0f);
    gluQuadricDrawStyle(q, GLU_FILL);
    gluDisk(q, 85, 90, 30, 30);
    gluDeleteQuadric(q);
    glPopMatrix();

    // --- Línea giratoria del radar ---
    glPushMatrix();
    glColor4f(1.0f, 0.0f, 0.0f, 0.9f);
    glTranslatef(700, 500, zRADAR);
    glRotatef(AngLineaRadar, 0.0, 0.0, -1.0);
    glLineWidth(3);
    glBegin(GL_LINES);
    glVertex2i(0, 0);
    glVertex2i(85, 0);
    glEnd();

    // Estela de triángulos verdes (opcional, tal como la tenías)
    float AngTrianguloRadar = 0.0;
    glColor4f(0.0f, 1.0f, 0.0f, 0.9f);
    for (int i = 0, limite = 120; i < limite; i++) {
        glBegin(GL_TRIANGLES);
        glVertex2d(0, 0);
        glColor4f(0.0f, 1.0f, 0.0f, 0.9f - (0.9f * (float(i) / limite)));
        AngTrianguloRadar = (360 * float(i) / limite) * (PI / 180.0f);
        glVertex2d(cos(AngTrianguloRadar) * 86, sin(AngTrianguloRadar) * 86);
        glColor4f(0.0f, 1.0f, 0.0f, 0.9f - (0.9f * (float(i + 1) / limite)));
        AngTrianguloRadar = (360 * float(i + 1) / limite) * (PI / 180.0f);
        glVertex2d(cos(AngTrianguloRadar) * 86, sin(AngTrianguloRadar) * 86);
        glEnd();
    }
    glPopMatrix();

    // --- Puntito rojo del enemigo relativo al avión ---
    glPushMatrix();
    glTranslatef(700, 500, zRADAR); // mismo centro que el radar

    // Calculamos la posición relativa
    float dx = EnemigoX - AvionX;
    float dy = EnemigoY - AvionY;

    // Escala para que encaje en el radio del radar
    float radarRadio = 85.0f;
    float escala = 0.05f; // ajusta este factor según el tamaño del mapa/mundo
    float rx = dx * escala;
    float ry = dy * escala;

    // Limitar dentro del radar
    float dist = sqrt(rx * rx + ry * ry);
    if (dist > radarRadio) {
        rx = rx * radarRadio / dist;
        ry = ry * radarRadio / dist;
    }

    glColor3f(1.0f, 0.0f, 0.0f); // rojo
    glPointSize(5.0f);
    glBegin(GL_POINTS);
    glVertex2f(rx, ry);
    glEnd();

    glPopMatrix();

    glPopMatrix(); // Fin push radar
}



void DibujarFondo()
{
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texID[0]);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2i(0, 0);
    glTexCoord2f(1.0f, 0.0f); glVertex2i(800, 0);
    glTexCoord2f(1.0f, 1.0f); glVertex2i(800, 600);
    glTexCoord2f(0.0f, 1.0f); glVertex2i(0, 600);
    glEnd();
    glDisable(GL_TEXTURE_2D);
}

void DibujarPlaneta() // Dibuja el planeta Júpiter (imagen PPM)
{
    glPushMatrix();
    glTranslated(PlanetaX, PlanetaY, zPLANETA);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texID[1]);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex2i(0, 0);
    glTexCoord2f(1, 0); glVertex2i(250, 0);
    glTexCoord2f(1, 1); glVertex2i(250, 250);
    glTexCoord2f(0, 1); glVertex2i(0, 250);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
}


GLUquadric* qobj = nullptr;

void InitOvni() {
    qobj = gluNewQuadric();
    gluQuadricTexture(qobj, GL_TRUE);   // Permite usar coordenadas de textura
    gluQuadricNormals(qobj, GLU_SMOOTH);
}

void DibujarOvni() {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texID[2]);  // textura del ovni

    glPushMatrix();
    glTranslatef(EnemigoX, EnemigoY, zAVION);
    gluDisk(qobj, 0.0f, 40, 60, 1);
    // radioInterior = 0.0 -> disco sólido
    // radioExterior = radio del ovni
    // slices = 60 para suavidad
    glPopMatrix();

    glDisable(GL_TEXTURE_2D);
}




void DibujarPista() {
    glPushMatrix();
    glTranslated(0.0, 0.0, zPISTA);

    

    // --- Rectangulo gris ---
    glColor3f(0.2f, 0.2f, 0.2f); // gris oscuro
    glBegin(GL_QUADS);
    glVertex2i(pistaX, pistaY);
    glVertex2i(pistaX + pistaAncho, pistaY);
    glVertex2i(pistaX + pistaAncho, pistaY + pistaAlto);
    glVertex2i(pistaX, pistaY + pistaAlto);
    glEnd();

    // --- Linea central discontinua ---
    glColor3f(1.0f, 1.0f, 1.0f);
    glLineWidth(3.0);
    glBegin(GL_LINES);
    for (int y = pistaY + 10; y < pistaY + pistaAlto - 10; y += 25) {
        glVertex2i(pistaX + pistaAncho / 2, y);
        glVertex2i(pistaX + pistaAncho / 2, y + 15);
    }
    glEnd();

    // --- Bordes ---
    glLineWidth(2.0);
    glBegin(GL_LINES);
    // Lado izquierdo
    glVertex2i(pistaX + 5, pistaY);
    glVertex2i(pistaX + 5, pistaY + pistaAlto);
    // Lado derecho
    glVertex2i(pistaX + pistaAncho - 5, pistaY);
    glVertex2i(pistaX + pistaAncho - 5, pistaY + pistaAlto);
    glEnd();

    glPopMatrix();
}



void DibujarProyectiles() {
    glPushMatrix();
    glTranslated(0, 0, zPROYECTIL);
    glPointSize(8);
    glColor3f(1.0, 1.0, 1.0);
    glBegin(GL_POINTS);
    for (auto& p : proyectil) p.Draw();
    glEnd();
    glPopMatrix();
}


void DibujarPanel() {
    // Mostrar tiempo transcurrido
    unsigned int tiempoActual = glutGet(GLUT_ELAPSED_TIME);
    int segundos = (tiempoActual - tiempoInicio) / 1000;
    print_text("Tiempo: " + to_string(segundos) + "s", 20, h - 30);

    // Mostrar energía
    print_text("Energia: " + to_string(Energia), 20, h - 50);
    print_text("Energia del enemigo: " + to_string(EnergiaEnemigo), 20, h - 70);

    glColor3f(1.0, 1.0, 0.0); // Amarillo
    print_text("Lucas Boffa - TP3 MO2D", 20, h - 90);
}

//-----------------------------------------------------------------MANEJO DE COLISIONES ---------------------------------------------------------------

bool ColisionRombo(float x1, float y1, float x2, float y2, float radio) {
    float dx = fabs(x1 - x2);
    float dy = fabs(y1 - y2);
    return (dx + dy) <= radio;
}



void ChequearColisiones() {

    // ----  Colisión AVION vs OVNI
    if (ColisionRombo(AvionX, AvionY, EnemigoX, EnemigoY, radioAvion + radioOvni)) {
        if(!escudoActivo)
            Energia -= 3;
        velOvniX = -velOvniX; // rebote simple
        velOvniY = -velOvniY;

        if (Energia <= 0) {
            Energia = 0;
            estado = PERDIDO;
        }
            
    }

}

//-----------------------------------------------------------------REINICIAR JUEGO-------------------------------------------------------------------------
void ReiniciarJuego() {
    Energia = 100;
    EnergiaEnemigo = 100;
    AvionX = 50; AvionY = 30; AvionAng = 0;
    EnemigoX = 350; EnemigoY = 350;
    proyectil.clear();
    estado = JUGANDO;
}


// -------------------------------------------------------------------CALLBACKS----------------------------------------------------------------------------

void Display_cb() {
    int _textoX = 40;
    int _textoY = h / 2;
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    


    if (estado == JUGANDO) {
        // Dibujar todo normalmente

        DibujarFondo();
        DibujarPlaneta();
        DibujarOvni();
        DibujarPista();


        glPushAttrib(GL_ALL_ATTRIB_BITS);

        // Activo material metálico
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambient);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);

        DibujarAvion();

        glPopAttrib();
        DibujarEscudo();
        DibujarRadar();
        DibujarProyectiles();

        DibujarPanel();
    }
    else if (estado == PERDIDO) {
        activarLlamas = false;
        print_text("PERDISTE!!! Te quedaste sin energía", w / 2 - 100, h / 2);
        print_text("Pulsa R para reiniciar o Esc para salir", w / 2 - 100, h / 2 + 30);
    }
    else if (estado == GANADO) {
        activarLlamas = false;
        print_text("GANASTE!!! Eliminaste al enemigo!!", w / 2 - 100, h / 2);
        print_text("Pulsa R para reiniciar o Esc para salir", w / 2 - 100, h / 2 + 30);
    }

    else {
        print_text("TUTORIAL:", w / 2 - 200, _textoY + 170);
        print_text("Debes eliminar al enemigo antes que te elimine a ti", _textoX, _textoY + 150);
        print_text("Chocar al enemigo resta energia a la nave. Si te quedas sin energia pierdes", _textoX, _textoY + 130);
        print_text("Moverse con teclas W,S,A,D", _textoX, _textoY + 110);
        print_text("Disparar con tecla G", _textoX, _textoY + 90);
        print_text("Turbo ton tecla T", _textoX, _textoY + 70);
        print_text("Escudo con tecla E (no pierdes energia y no disparas mientras se encuentra activo)", _textoX, _textoY + 50);
        print_text("Pulsa R para comenzar o Esc para salir", _textoX, _textoY + 20);
    }

    if (activarLlamas) {
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        // Dibujar texto titilante    
        float elapsedTime = glutGet(GLUT_ELAPSED_TIME) % 1500 / 1499.f; // Tiempo en milisegundos
        float opacidad = 315 / (64 * 3.1415 * pow(0.25, 9)) *
            pow((pow(0.25, 2) - pow(elapsedTime - 0.25, 2)), 3) / 12.5335 + 0.1f;
        if (opacidad > 1.0f)
            opacidad = 1.0f;
        glColor4f(1.0f, 0.0f, 0.0f, opacidad);
        print_text("TURBO ACTIVADO", w / 2, h - 20);
        glPopAttrib();
    }

    glutSwapBuffers();
}


void Idle_cb() {
    // Tiempo real entre frames
    static unsigned int lt = glutGet(GLUT_ELAPSED_TIME);
    unsigned int now = glutGet(GLUT_ELAPSED_TIME);
    float dt = (now - lt) / 1000.0f; // delta time en segundos
    lt = now;

    // Actualizar avión
    double vel = 2 + movimiento; // si hay turbo, corre más rápido
    double ang = AvionAng * PI / 180.0;

    if (teclas['w'] || teclas['W']) {
        AvionX -= vel * sin(ang);
        AvionY += vel * cos(ang);
    }
    if (teclas['s'] || teclas['S']) {
        AvionX += 5 * sin(ang);
        AvionY -= 5 * cos(ang);
    }
    if (teclas['a'] || teclas['A']) {
        AvionAng += 2;
    }
    if (teclas['d'] || teclas['D']) {
        AvionAng -= 2;
    }

    // Limitar posición
    if (AvionX < xmin) AvionX = xmin;
    if (AvionX > xMax) AvionX = xMax;
    if (AvionY < yMin) AvionY = yMin;
    if (AvionY > yMax) AvionY = yMax;

    // Radar y turbinas
    AngLineaRadar = (AngLineaRadar + 2) % 360;
    AngTurbina = (AngTurbina + 10) % 360;

    // Actualizar proyectiles (me fijo si se salen de la pantalla o chocan con el enemigo mientras se mueven)
    for (auto it = proyectil.begin(); it != proyectil.end();) {
        it->Update(EnemigoX, EnemigoY, w, h);

        if (it->GetOut() || it->GetCollision()) {
            if (it->GetCollision()) {
                EnergiaEnemigo -= 5;
                if (EnergiaEnemigo <= 0) {
                    EnergiaEnemigo = 0;
                    estado = GANADO;
                }
                    

            }               

            it = proyectil.erase(it); // devuelve el siguiente iterador válido
        }
        else {
            ++it; // solo avanzamos si no borramos
        }
    }


    // Llamas con fade in/out
    UpdateLlamas(dt);

    // Ajuste de turbo
    activarLlamas ? movimiento = 5 : movimiento = 0;  //para ir más rápido si tengo las llamas encendidas


    // Actualizar posición del OVNI
    EnemigoX += velOvniX;
    EnemigoY += velOvniY;

    // Márgenes para no pegarse al borde
    float margen = 40.0f;

    // Rebote en los bordes con margen
    if (EnemigoX > w - margen) {
        EnemigoX = w - margen;   // corregir posición
        velOvniX = -(1 + rand() % 3); // nueva velocidad negativa aleatoria
        velOvniY = (rand() % 5 - 2);  // cambiar Y al azar
    }
    else if (EnemigoX < margen) {
        EnemigoX = margen;
        velOvniX = 1 + rand() % 3;    // nueva velocidad positiva aleatoria
        velOvniY = (rand() % 5 - 2);
    }

    if (EnemigoY > h - margen) {
        EnemigoY = h - margen;
        velOvniY = -(1 + rand() % 3); // nueva velocidad negativa aleatoria
        velOvniX = (rand() % 5 - 2);  // cambiar X al azar
    }
    else if (EnemigoY < margen) {
        EnemigoY = margen;
        velOvniY = 1 + rand() % 3;    // nueva velocidad positiva aleatoria
        velOvniX = (rand() % 5 - 2);
    }

    ChequearColisiones();  //veo si la nave y el ovni colisionan

    glutPostRedisplay();
}




void Reshape_cb(int width, int height) {
    if (!width || !height) return;
    w = width; h = height;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    glOrtho(0, w, 0, h, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glutPostRedisplay();
}

void KeyDown_cb(unsigned char key, int x, int y) {
    teclas[key] = true;
    if (key == 27) exit(EXIT_SUCCESS);

    if ((key == 'g' || key == 'G') && puedeDisparar && !escudoActivo) {
        puedeDisparar = false;
        double ang3 = (AvionAng + ArmaAng) * PI / 180.0;
        proyectil.push_back(Bala(AvionX, AvionY, -30 * sin(ang3), 30 * cos(ang3)));
    }

    if (key == 't' || key == 'T') {
        activarLlamas = !activarLlamas; // cambia entre true/false
        
    }

    // Activar/desactivar escudo con la tecla 'e' o 'E'
    if (key == 'e' || key == 'E') {
        escudoActivo = !escudoActivo;
        
    }

    if (key == 'r' || key == 'R') {
        if (estado == PERDIDO || estado == GANADO || estado == TUTORIAL)
            ReiniciarJuego();
    }
}

void KeyUp_cb(unsigned char key, int x, int y) {
    teclas[key] = false;

    if (key == 'g' || key == 'G')
        puedeDisparar = true;

  
}

void Special_cb(int key, int xm = 0, int ym = 0) {
    if (key == GLUT_KEY_F4 && glutGetModifiers() == GLUT_ACTIVE_ALT)
        exit(EXIT_SUCCESS);
}

void inicializa() {

    InitOvni();
    tiempoInicio = glutGet(GLUT_ELAPSED_TIME);

    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(w, h); glutInitWindowPosition(10, 10);
    glutCreateWindow("Ejemplo Avion con Turbinas");

    glutDisplayFunc(Display_cb);
    glutReshapeFunc(Reshape_cb);
    glutKeyboardFunc(KeyDown_cb);
    glutKeyboardUpFunc(KeyUp_cb);
    glutSpecialFunc(Special_cb);
    glutIdleFunc(Idle_cb);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    glEnable(GL_LIGHTING);   // Habilita el sistema de iluminación
    glEnable(GL_LIGHT0);     // Activa una luz
    glEnable(GL_COLOR_MATERIAL); // Permite usar glColor junto con materiales
    glShadeModel(GL_SMOOTH); // Suaviza los colores

    // Configuración de la luz
    GLfloat light_pos[] = { 200.0f, 200.0f, 300.0f, 1.0f }; // desde arriba a la derecha
    GLfloat white_light[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat ambient_light[] = { 0.2f, 0.2f, 0.2f, 1.0f };   // leve luz ambiente

    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, white_light);
    glLightfv(GL_LIGHT0, GL_SPECULAR, white_light);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient_light);


    //----------------------------------------

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDepthRange(0.0, 1.0);
    glClearDepth(1.0);
    glClearColor(0.01f, 0.01f, 0.01f, 1.f);
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();

    // Texturas 2D
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(3, texID);
    // Textura 1
    glBindTexture(GL_TEXTURE_2D, texID[0]);
    mipmap_ppm("galaxia.ppm");
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    // Textura 2
    glBindTexture(GL_TEXTURE_2D, texID[1]);
    mipmap_ppm("jupiter.ppm");
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    // Textura 3
    glBindTexture(GL_TEXTURE_2D, texID[2]);
    mipmap_ppm("ovni.ppm");
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

}

int main(int argc, char** argv) {
    cout << "Teclas:\nw/s: avanzar/retroceder\na/d: girar\ng: disparar\nt: turbo\ne: escudo" << endl;
    glutInit(&argc, argv);
    inicializa();
    glutMainLoop();
    return 0;
}
