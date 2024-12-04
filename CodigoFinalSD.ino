/*######################################################################
  //#  G0B1T: uC EXAMPLES. 2024.
  //######################################################################
  //# Copyright (C) 2024. F.E.Segura-Quijano (FES) fsegura@uniandes.edu.co
  //#
  //# This program is free software: you can redistribute it and/or modify
  //# it under the terms of the GNU General Public License as published by
  //# the Free Software Foundation, version 3 of the License.
  //#
  //# This program is distributed in the hope that it will be useful,
  //# but WITHOUT ANY WARRANTY; without even the implied warranty of
  //# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  //# GNU General Public License for more details.
  //#
  //# You should have received a copy of the GNU General Public License
  //# along with this program.  If not, see <http://www.gnu.org/licenses/>
  //####################################################################*/
#include <Arduino.h>
#include "LedControl.h"

// Pines para MAX72XX
#define DIN_PIN 23
#define CLK_PIN 5
#define LOAD_PIN 18
LedControl lc = LedControl(DIN_PIN, CLK_PIN, LOAD_PIN, 1);

// Pines para botones
#define RESET_PIN 26
#define START_PIN 27
#define LEFT_PIN 14
#define RIGHT_PIN 12

// Variables globales
enum State_enum {STATERESET, STATENIVEL1, STATENIVEL2, STATENIVEL3, STATELOST, STATEWIN};
uint8_t state = STATERESET;

enum Keys_enum {RESET_KEY, START_KEY, LEFT_KEY, RIGHT_KEY, NO_KEY};
uint8_t keys = NO_KEY;

byte RegMatrix[8];   // Matriz para representar el fondo
byte RegCar[1] = {B00010000}; // Carro inicial en el centro de la fila inferior

unsigned long delaytime = 333; // Velocidad inicial (ms entre movimientos)
int nivel = 1;
int score = 0;

// Variables para detección de pulsación
bool leftPressed = false;
bool rightPressed = false;

//=======================================================
//  FUNCIÓN: generarFilaFacil
//=======================================================
byte generarFilaFacil(int nivel) {
    byte fila = 0;
    int densidad;

    if (nivel == 1) {
        densidad = random(1, 2); // 1-2 obstáculos
    } else if (nivel == 2) {
        densidad = random(2, 3); // 2-3 obstáculos
    } else {
        densidad = random(3, 4); // 3-4 obstáculos
    }

    for (int i = 0; i < densidad; i++) {
        fila |= (1 << random(0, 8));
    }
    return fila;
}

//=======================================================
//  FUNCIÓN: generarObstaculos
//=======================================================
void generarObstaculos() {
    byte nuevaFila = generarFilaFacil(nivel);
    for (int i = 7; i > 0; i--) {
        RegMatrix[i] = RegMatrix[i - 1];
    }
    RegMatrix[0] = nuevaFila;
}

void setup() {
    // Configuración de pines
    pinMode(RESET_PIN, INPUT_PULLUP);
    pinMode(START_PIN, INPUT_PULLUP);
    pinMode(LEFT_PIN, INPUT_PULLUP);
    pinMode(RIGHT_PIN, INPUT_PULLUP);

    // Inicializar matriz
    lc.shutdown(0, false);
    lc.setIntensity(0, 8);
    lc.clearDisplay(0);

    // Inicializar estados
    state = STATERESET;
}

void resetGame() {
    for (int i = 0; i < 8; i++) RegMatrix[i] = 0;
    RegCar[0] = B00010000;
    nivel = 1;
    score = 0;
    delaytime = 333;
    lc.clearDisplay(0);
    state = STATERESET;
}

void showNumber(int num) {
    lc.clearDisplay(0);
    switch (num) {
        case 1: lc.setRow(0, 1, B00011000);
                lc.setRow(0, 2, B00011000);
                lc.setRow(0, 3, B00011000);
                lc.setRow(0, 4, B00011000);
                lc.setRow(0, 5, B00011000);
                break;
        case 2: lc.setRow(0, 1, B00111100);
                lc.setRow(0, 2, B01100110);
                lc.setRow(0, 3, B00001100);
                lc.setRow(0, 4, B00011000);
                lc.setRow(0, 5, B01111110);
                break;
        case 3: lc.setRow(0, 1, B00111100);
                lc.setRow(0, 2, B01100110);
                lc.setRow(0, 3, B00001100);
                lc.setRow(0, 4, B01100110);
                lc.setRow(0, 5, B00111100);
                break;
    }
}

void showFace(bool happy) {
    lc.clearDisplay(0);
    if (happy) {
        lc.setRow(0, 0, B01111110);
        lc.setRow(0, 1, B10000001);
        lc.setRow(0, 2, B10011001);
        lc.setRow(0, 3, B10000001);
        lc.setRow(0, 4, B10100101);
        lc.setRow(0, 5, B10011001);
        lc.setRow(0, 6, B10000001);
        lc.setRow(0, 7, B01111110);
    } else {
        lc.setRow(0, 0, B01111110);
        lc.setRow(0, 1, B10000001);
        lc.setRow(0, 2, B10011001);
        lc.setRow(0, 3, B10000001);
        lc.setRow(0, 4, B10011001);
        lc.setRow(0, 5, B10100101);
        lc.setRow(0, 6, B10000001);
        lc.setRow(0, 7, B01111110);
    }
}

void drawMatrix() {
    for (int i = 0; i < 8; i++) lc.setRow(0, i, RegMatrix[i]);
    lc.setRow(0, 7, RegCar[0]);
}

void checkCollision() {
    if (RegCar[0] & RegMatrix[7]) {
        state = STATELOST;
    }
}

void readInputs() {
    if (digitalRead(RESET_PIN) == LOW) {
        resetGame(); // Reset en cualquier momento
        return;
    }

    if (digitalRead(LEFT_PIN) == LOW && !leftPressed) {
        leftPressed = true;
        if (RegCar[0] != B10000000) RegCar[0] <<= 1;
    } else if (digitalRead(LEFT_PIN) == HIGH) {
        leftPressed = false;
    }

    if (digitalRead(RIGHT_PIN) == LOW && !rightPressed) {
        rightPressed = true;
        if (RegCar[0] != B00000001) RegCar[0] >>= 1;
    } else if (digitalRead(RIGHT_PIN) == HIGH) {
        rightPressed = false;
    }

    if (digitalRead(START_PIN) == LOW) keys = START_KEY;
    else keys = NO_KEY;
}

void loop() {
    readInputs();

    switch (state) {
        case STATERESET:
            if (keys == START_KEY) {
                showNumber(1); // Mostrar el número del nivel
                delay(1000); // Pausa breve para que se vea el número
                state = STATENIVEL1;
                lc.clearDisplay(0);
            }
            break;

        case STATENIVEL1:
        case STATENIVEL2:
        case STATENIVEL3:
            static unsigned long lastUpdate = 0;
            if (millis() - lastUpdate > delaytime) {
                generarObstaculos(); // Generar obstáculos dinámicamente
                checkCollision();

                if (state != STATELOST) {
                    score++;
                    if (score == 10 && state == STATENIVEL1) {
                        nivel = 2;
                        delaytime = 250; // Incrementar velocidad
                        showNumber(2); // Mostrar el número del nivel
                        delay(1000);
                        state = STATENIVEL2;
                    } else if (score == 25 && state == STATENIVEL2) {
                        nivel = 3;
                        delaytime = 200; // Incrementar velocidad
                        showNumber(3); // Mostrar el número del nivel
                        delay(1000);
                        state = STATENIVEL3;
                    } else if (score == 45 && state == STATENIVEL3) {
                        state = STATEWIN;
                    }
                }

                lastUpdate = millis();
            }
            break;

        case STATELOST:
            showFace(false);
            delay(2000);
            resetGame();
            break;

        case STATEWIN:
            showFace(true);
            delay(2000);
            resetGame();
            break;
    }

    drawMatrix();
}
