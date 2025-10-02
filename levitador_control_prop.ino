#include "definitions.h"
#include <Wire.h>

// Tiempo de muestreo
const float h = 0.025;

// Parámetros del sistema
const float calibration = 60;
const float umax = 12;
const float umin = 0;
const float b = 46.944;
const float L = 0.601;
const float tau1 = 0.658;

// Ganancias PID
const float kp = 1.5 * (0.37 / (b * L)) + (0.02 * tau1 / (b * L * L));
const float ki = 1.05 * (0.03 / (b * L * L)) + (0.0012 * tau1 / (b * L * L * L));
const float kd = 1.4 * (0.16 / b) + (0.28 * tau1 / (b * L));

float reference = 1.0;  // Setpoint inicial (altura en cm)
float y, u, usat, e, e_anterior = 0;
float integral = 0, derivada = 0;
const float ueq = 6;
int  v_previo=0;
// ⚠️ IMPORTANTE: Usa un pin ADC1 para evitar problemas
const int potmeterPin = 34;  
int potmeterVal = 0;

void setup() {
    Serial.begin(115200);
    Wire.begin();

    pinMode(potmeterPin, INPUT);
    analogSetPinAttenuation(potmeterPin, ADC_11db);  // Rango de 0V a 3.6V

    while (!setupSensor()) {
        vTaskDelay(1000);
    }
    setupPwm();
    vTaskDelay(100);

    xTaskCreatePinnedToCore(
        controlTask,
        "PID Control Task",
        8192, NULL, 1, NULL, CORE_1
    );
}

// ***************************************************************************
//                CONTROL PID DEL LEVITADOR
// ***************************************************************************

static void controlTask(void *pvParameters) {
    const TickType_t taskInterval = pdMS_TO_TICKS(h * 1000);
    TickType_t xLastWakeTime = xTaskGetTickCount();

    for (;;) {
    

        reference= 35;
        // Medición de la distancia
        y = calibration - (float)sensor.readRangeContinuousMillimeters() / 10;

        // Cálculo del error
        e = reference - y;

        // Control PID
        float P = kp * e;
        integral += e * h;
        float I = ki * integral;
        derivada = (e - e_anterior) / h;
        float D = kd * derivada;
        e_anterior = e;
        u = P + I + D + ueq;

        // Saturación
        usat = constrain(u, umin, umax);
        voltsToFan(usat);

        // 📊 FORMATO PARA SERIAL PLOTTER (CSV)
        Serial.printf("%.2f,%.2f,%.3f\n", reference, y, usat);

        
        // Mantener el periodo de muestreo exacto
        vTaskDelayUntil(&xLastWakeTime, taskInterval);
    }
}

void loop() {
    vTaskDelete(NULL);
}
