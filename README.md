# Levitador Neumático — Proyecto Final Técnicas de Control (2025-I)

**Universidad Nacional de Colombia**  
**Facultad de Ingeniería — Departamento de Ingeniería Mecánica y Mecatrónica**

---

## Autores

| Nombre |
|--------|
| Jorge Emilio Melo Guevara |
| Juan David Medina Pérez |
| Santiago Camilo Fonseca Prieto |

---

## Descripción del proyecto

Este proyecto aborda el diseño, construcción, modelado matemático, simulación e implementación de controladores para una **planta levitadora neumática**: un pistón que flota dentro de un tubo acrílico por efecto del flujo de aire generado por un ventilador. La entrada de la planta es el voltaje suministrado al ventilador y la salida es la altura del pistón medida con un sensor.

El trabajo cubre el flujo completo de un proyecto de control: desde la fabricación física de la planta hasta la implementación y validación experimental de controladores **PID** y **H∞** en una tarjeta **ESP32**.

---

## Contenido del repositorio

| Archivo | Descripción |
|---------|-------------|
| `definitions.h` | Comunicación serial PC-ESP32 y conversión de señal de control a PWM para el ventilador |
| `levitador_control_prop.ino` | Caracterización por tren de pulsos, ganancias del PID y lógica de control realimentado |
| `BaseProyectoLevitador.pdf` | Documento base del proyecto |
| `Tapa.pdf` | Plano de la tapa del tubo |
| `piston (1).pdf` | Plano de fabricación del pistón |
| `paso_PID.eps` | Respuesta al escalón del controlador PID |

---

## 1. Diseño y montaje de la planta

### 1.1 Pistón

El pistón fue diseñado con dos criterios principales: minimizar el tambaleo dentro del tubo (garantizando un desplazamiento vertical limpio) y maximizar el área superficial expuesta al flujo de aire para aumentar la fuerza de arrastre. Los planos de fabricación se encuentran en el repositorio.

### 1.2 Tubo

Se empleó un tubo acrílico transparente de **20 mm de diámetro**, seleccionado para permitir la visualización directa del pistón durante la operación.

---

## 2. Modelamiento de la planta

### 2.1 Modelo aerodinámico

El modelo parte de la expresión de la fuerza de arrastre (**drag**) sobre el pistón, aproximado como un cilindro de radio ligeramente menor al del tubo:

$$F_D = \frac{1}{2} \rho \, C_D \, A \, (v_f - v_p)^2$$

Aplicando la segunda ley de Newton sobre el pistón (masa $m$, peso $W = mg$):

$$\sum F = ma \implies a = Z(v_f - v_p)^2 - g$$

Incorporando una pérdida lineal de velocidad del flujo a lo largo del tubo (altura $h$):

$$\boxed{a = Z(v_f - v_p - q\,h)^2 - g}$$

donde:

| Símbolo | Descripción | Valor |
|---------|-------------|-------|
| $\rho$ | Densidad del aire en Bogotá | $0.887\ \text{kg/m}^3$ |
| $A$ | Área superficial del pistón (Autodesk Inventor) | $0.0017\ \text{m}^2$ |
| $C_D$ | Coeficiente de drag (cilindro, 20 °C) | $1.5$ |
| $m$ | Masa del pistón (con platinista) | $3\ \text{g}$ |
| $q$ | Constante de pérdida de flujo (experimental) | $0.33$ |
| $Z$ | Constante aerodinámica: $Z = \rho C_D A / 2m$ | calculado |

> **Nota:** cuando el pistón está en contacto con el piso del tubo ($h = 0$), la fuerza normal cancela el peso y la aceleración pasa a ser $a = Z(v_f - v_p - qh)^2$.

### 2.2 Relación voltaje–velocidad del flujo

El control de la planta se realiza mediante el voltaje $V$ al ventilador. La curva $v_f(V)$ fue caracterizada experimentalmente con un sensor tipo **tubo de Pitot**, obteniéndose el ajuste polinomial:

$$v_f(V) = 0.0003V^5 - 0.0081V^4 + 0.0551V^3 + 0.0086V^2 - 0.269V + 3.0983$$

### 2.3 Modelo en espacio de estados

Definiendo los estados $x_1 = h$ (altura) y $x_2 = v_p$ (velocidad del pistón), la entrada $u = V$ y la salida $y = 0.5\,x_1$:

$$\dot{x}_1 = x_2$$

$$\dot{x}_2 = Z\bigl(v_f(u) - x_2 - q\,x_1\bigr)^2 - g$$

$$y = 0.5\,x_1$$

### 2.4 Modelo en Simulink

El modelo fue implementado en **Simulink** con las siguientes consideraciones adicionales:
- Bloque de comparación con cero para activar/desactivar la gravedad según si el pistón tiene contacto con el piso.
- Bloque de saturación para evitar alturas negativas a la salida.

---

## 3. Validación del modelo

Para verificar el modelo, se sometió tanto la planta física como el modelo simulado a una **señal de pulsos periódica** alternando entre 4 V y 6.5 V con periodo de 1 s (voltaje crítico de levantamiento ≈ 5.3 V). Los resultados (tomados a partir de 6 s para excluir transitorios) mostraron que:

- La elevación producida por los pulsos es comparable en simulación y planta real.
- La simulación **no reproduce las oscilaciones** presentes en la planta física, lo que indica fenómenos no modelados: posibles turbulencias, retardo del actuador o efectos no lineales del flujo.

Este hallazgo motivó el diseño de un controlador robusto ($H_\infty$) capaz de manejar dichas incertidumbres.

---

## 4. Diseño de los controladores

### 4.1 Linealización e identificación de la planta

Dado que la planta es inestable en lazo abierto (no existen puntos de equilibrio sin realimentación), se aplicó primero un **control proporcional** para estabilizarla en la mitad del tubo. Con la planta estabilizada, se realizó un **relevo de Åström** para identificar sus parámetros en la zona de operación lineal.

La función de transferencia linealizada identificada es:

$$G(s) = \frac{b}{s(\tau_1 s + 1)}\,e^{-Ls} = \frac{46.944}{s\,(0.6012\,s + 1)}\,e^{-0.6587\,s}$$

El punto de equilibrio fue $u_{eq} = 5.325\ \text{V}$.

Las matrices del sistema linealizado (Jacobiano evaluado en el punto de equilibrio) son:

$$A = \begin{bmatrix} 0 & 1 \\ -0.9884 & -3.9537 \end{bmatrix}, \quad B = \begin{bmatrix} 0 \\ 3.2565 \end{bmatrix}, \quad C = \begin{bmatrix} 0.5 & 0 \end{bmatrix}, \quad d = 0$$

### 4.2 Controlador PID

El PID fue sintonizado por el **método empírico de Åström–Hägglund** a partir de los parámetros del relevo:

$$k_p = 1.5\left(\frac{0.37}{bL}\right) + \frac{0.02\,\tau_1}{bL^2} \qquad k_i = 1.05\left(\frac{0.03}{bL^2}\right) + \frac{0.0012\,\tau_1}{bL^3} \qquad k_d = 1.4\left(\frac{0.16}{b}\right) + \frac{0.28\,\tau_1}{bL}$$

| Ganancia | Valor |
|----------|-------|
| $k_p$ | 0.02045 |
| $k_i$ | 0.00193 |
| $k_d$ | 0.01130 |

**Desempeño en simulación:** sobrepico ≈ 20%, tiempo de asentamiento ≈ 15 s.

### 4.3 Controlador H∞

El controlador robusto $H_\infty$ fue diseñado a partir del modelo linealizado, con los siguientes filtros de forma:

**Filtro $W_1$** — seguimiento de referencia:

| $M_1$ | $A_1$ | $\omega_{b1}$ |
|--------|--------|----------------|
| 1.5 | 0.005 | 4.1 rad/s |

**Filtro $W_3$** — rechazo de ruido:

| $M_3$ | $A_3$ | $\omega_{b3}$ |
|--------|--------|----------------|
| 2 | 0.02 | 20 rad/s |

**Filtro $W_2$** — penalización de la señal de control (aplicado como filtro en lugar de constante, por problemas de magnitud de la señal de control):

| $M_2$ | $A_2$ | $\omega_{b2}$ |
|--------|--------|----------------|
| 0.2 | 0.01 | 50 rad/s |

Las funciones de sensibilidad $S$, $T$ y $KS$ se verificaron contra los inversos de los pesos $W_1^{-1}$, $W_2^{-1}$ y $W_3^{-1}$ respectivamente, confirmando el cumplimiento de las especificaciones de diseño.

---

## 5. Implementación en la planta física

Ambos controladores fueron implementados en una **ESP32** usando el **IDE de Arduino**, con comunicación serial para el cambio de referencia en tiempo real.

### Archivos de implementación

- **`definitions.h`**: gestión de la comunicación serial y conversión de la señal de control a señal PWM para el ventilador.
- **`levitador_control_prop.ino`**: caracterización de la planta por tren de pulsos, ganancias del PID y lógica del lazo de control realimentado.

### Ajustes requeridos en la planta para el H∞

Al implementar el controlador $H_\infty$ en la planta física fue necesario:
- Aplicar una **ganancia proporcional de 0.1** a la salida del controlador.
- Agregar un **offset de 5.16 V** a la señal de control, de modo que ante una señal nula el pistón descendiera de forma controlada (en lugar de caer libremente).

Estos ajustes reflejan la importancia de considerar las características físicas del actuador en la etapa de implementación, algo no capturado completamente por el modelo linealizado.

### Demostraciones en video

| Controlador | Enlace |
|-------------|--------|
| PID | [Ver demostración](https://youtube.com/shorts/Yb6yQg3QmOs?feature=share) |
| H∞ | [Ver demostración](https://youtube.com/shorts/Yb6yQg3QmOs?feature=share) |

---

## 6. Conclusiones

El modelo matemático derivado de primeros principios aerodinámicos reprodujo con buena aproximación el comportamiento de la planta en su zona operativa — en particular la altura de levitación — pero no capturó las oscilaciones observadas en la planta real, atribuidas a turbulencias, retardos del actuador y efectos no lineales del flujo no incluidos en el modelo.

El controlador $H_\infty$, diseñado a partir de la linealización en torno al punto de equilibrio, demostró un desempeño robusto ante las incertidumbres del modelo, logrando estabilizar el pistón a distintas alturas de referencia. Los ajustes de ganancia y offset requeridos en la implementación física subrayan la brecha práctica entre el modelo linealizado y el sistema real, y motivan el refinamiento del modelo en trabajos futuros.
