# Space Shooter - OpenGL y GLUT

<p align="center">
  Juego 2D desarrollado en C++ con OpenGL y GLUT, donde el jugador controla un avión espacial y debe destruir un ovni enemigo mientras administra energía, escudo, turbo y disparos.
</p>

<p align="center">

![C++](https://img.shields.io/badge/language-C%2B%2B-blue)
![OpenGL](https://img.shields.io/badge/graphics-OpenGL-green)
![GLUT](https://img.shields.io/badge/library-GLUT-orange)
![IDE](https://img.shields.io/badge/IDE-Visual%20Studio-purple)

</p>

---

## Descripción

Juego 2D desarrollado en **C++** utilizando **OpenGL** y **GLUT**.

El jugador controla un avión que puede desplazarse, girar, activar turbo, usar un escudo y disparar proyectiles para destruir un ovni enemigo.

El escenario incluye:

- Fondo espacial con texturas
- Un planeta texturizado
- Radar
- Pista de despegue
- Efectos visuales como llamas, escudo y disparos

El objetivo es reducir la energía del enemigo a cero antes de perder toda la energía propia.

---

## Mecánicas del Juego

- Movimiento libre del avión dentro del mapa
- Rotación del avión
- Disparo de proyectiles
- Sistema de energía del jugador
- Sistema de energía del enemigo
- Escudo activable
- Turbo con efecto de llamas
- Radar que muestra la posición relativa del enemigo
- Colisiones entre avión y enemigo
- Estados de juego:
  - Tutorial
  - Jugando
  - Ganado
  - Perdido

---

## Controles

| Tecla | Acción |
|------|--------|
| W | Avanzar |
| S | Retroceder |
| A | Girar a la izquierda |
| D | Girar a la derecha |
| G | Disparar |
| T | Activar / desactivar turbo |
| E | Activar / desactivar escudo |
| R | Comenzar o reiniciar partida |
| Esc | Salir |

---

## Objetivo del Juego

- Destruir al ovni enemigo reduciendo su energía a **0**
- Evitar colisiones directas con el enemigo
- Administrar correctamente el uso de:
  - disparos
  - turbo
  - escudo

El jugador pierde si su energía llega a **0**.

---

## Características Destacadas

- Renderizado 2D con OpenGL
- Uso de texturas `.ppm`
- Radar animado
- Turbinas rotativas
- Efecto visual de llamas al activar turbo
- Escudo semitransparente
- Panel en pantalla con tiempo y energía
- Colisiones y estados de victoria / derrota

---

## Tecnologías Utilizadas

- C++
- OpenGL
- GLUT
- GLU
- Texturas PPM
- Programación estructurada y modular

---

## Estructura General del Proyecto

El juego está dividido en distintas partes funcionales:

### Renderizado
Se encarga de dibujar:

- Fondo espacial
- Planeta
- Ovni enemigo
- Avión
- Pista
- Radar
- Proyectiles
- Panel informativo
- Escudo y llamas

### Lógica de Juego
Controla:

- Estados del juego
- Energía del jugador y del enemigo
- Movimiento del avión
- Movimiento del ovni
- Colisiones
- Reinicio de partida

### Entrada del Usuario
Controla el teclado para:

- mover el avión
- disparar
- activar turbo
- activar escudo
- reiniciar

---

## Gameplay

Durante la partida, el jugador debe perseguir y atacar al enemigo mientras se desplaza por el escenario.

El ovni se mueve automáticamente y rebota dentro del mapa.

El radar ayuda a localizar al enemigo, mientras que el panel muestra:

- tiempo transcurrido
- energía del jugador
- energía del enemigo

El turbo aumenta la velocidad del avión, y el escudo permite resistir choques sin perder energía.

---

## Capturas de Pantalla

<p align="center">

  <img src="https://github.com/user-attachments/assets/c3d8b5c4-79db-4e9f-88f9-d0464fb9bc6b"
       alt="2d3"
       width="30%"
       style="border:3px solid #555; margin:10px; border-radius:8px;" />

  <img src="https://github.com/user-attachments/assets/5c1fc983-20e5-45d0-a4d3-51cf71c53741"
       alt="2d2"
       width="30%"
       style="border:3px solid #555; margin:10px; border-radius:8px;" />

  <img src="https://github.com/user-attachments/assets/b14ee37c-f757-4afd-ac82-6848de89af51"
       alt="2d1"
       width="30%"
       style="border:3px solid #555; margin:10px; border-radius:8px;" />

</p>

---

## Cómo Ejecutar

1. Abrir el proyecto en **Visual Studio**
2. El proyecto ya tiene configurado **OpenGL / GLUT** en 32 bits.
3. Compilar el proyecto
4. Ejecutar el programa

---

## Archivos de Textura

El proyecto utiliza texturas en formato `.ppm`:

- `galaxia.ppm`
- `jupiter.ppm`
- `ovni.ppm`

Estas texturas deben estar correctamente ubicadas para que el juego pueda cargarlas.

---

## Autor

Lucas Boffa  
Tecnicatura en Diseño y Desarrollo de Videojuegos  
UNL
