
# Proyecto Final - Técnicas de Control (2025-I)

Universidad Nacional de Colombia  
Facultad de Ingeniería – Departamento de Ingeniería Mecánica y Mecatrónica  

Autores:  
- Jorge Emilio Melo Guevara  
- Juan David Medina Pérez  
- Santiago Camilo Fonseca Prieto  

---

##  Descripción
Este proyecto implementa el diseño, modelado y control de una planta experimental consistente en un pistón suspendido dentro de un tubo de aire. El objetivo principal fue **desarrollar y validar modelos matemáticos y de simulación**, y posteriormente diseñar controladores (**PID y H∞**) para estabilizar la posición del pistón.

---

##  Diseño y montaje de la planta
- **Pistón**: diseñado para minimizar oscilaciones dentro del tubo y maximizar el área de contacto con el flujo de aire.  
- **Tubo**: acrílico transparente de 20 mm de diámetro para visualizar el movimiento del pistón.

  Los planos se encuentras en los archivos del repositorio.

---

##  Modelamiento de la planta
Se utilizó un modelo basado en la fuerza de arrastre (**drag**) sobre el pistón:

$$
F_D = \tfrac{1}{2} \rho C_D A (v_f - v_p)^2
$$

Donde:
- ρ: densidad del aire en Bogotá (0.887 kg/m³)  
- A: área superficial del pistón (0.0017 m²)  
- Cd: coeficiente de drag (1.5)  
- m: masa del pistón (3 g)  
- q: constante de pérdida experimental (0.33)  

El modelo dinámico se implementó en **Simulink**, considerando saturación y contacto con el suelo del tubo.

---

##  Validación del modelo
- Se aplicó una señal de pulsos periódica al ventilador.  
- Se comparó la respuesta simulada con la planta real.  
- El modelo predijo adecuadamente la altura alcanzada, pero no capturó las oscilaciones observadas en la planta física (fenómenos no modelados como turbulencias o retardos).  

---

##  Diseño de control
### PID
- Se aplicó un **control proporcional** inicial para encontrar un punto de equilibrio.  
- Posteriormente se ajustó un **PID** mediante el método empírico de Åström y Hägglund.  
- Resultados: sobrepico ≈ 20% y tiempo de asentamiento ≈ 15 s.
<img width="905" height="682" alt="image" src="https://github.com/user-attachments/assets/c84925c5-5539-4e4a-bc86-0d385b431b4b" />

### Linealización
- Se obtuvo un modelo linealizado en torno al punto de equilibrio $u_{eq} = 5.325 \, V$.  
- Matrices del sistema:


     A = [  0        1      ;
     -0.9884  -3.9537 ]


B = [ 0 ;
      3.2565 ]


### Control Hinf
- Se diseñaron filtros de sensibilidad $W1, W2, W3$.  
- Se logró un buen desempeño en simulación, aunque en la planta física se requirió:  
  - Ajustar la ganancia global.  
  - Aplicar un **offset** de 5.16 V en la señal de control.  

<img width="1214" height="605" alt="image" src="https://github.com/user-attachments/assets/c7ae2e9e-78ae-408f-810f-df5a6a76760e" />
salida en el tiempo

<img width="1210" height="586" alt="image" src="https://github.com/user-attachments/assets/849dd8be-f683-456c-93bf-e7cd9e7a0d83" />
Señal de control

---

##  Implementación
- El control fue implementado en la planta con una **ESP32** programada en el **IDE de Arduino**, haciendo uso de comunicación serial para cambios de referencia en tiempo real; El archivo "definitions.h" se encarga de la comunicacion entre la planta y el computador, ademas de la conversion a PWM que recibe el ventilador. El archiv "levitador_control_prop.ino" tiene los parametros de caracterizacion de la planta y las ganacias propias del control PID,  asi como la logica del sistema de control realimentado.   
  PID:
  En el enlace: https://youtube.com/shorts/Yb6yQg3QmOs?feature=share se encuentra la implentacion del control PID.

  H-inf:
  En el enlace: https://youtube.com/shorts/Yb6yQg3QmOs?feature=share se encuentra la implentacion del control Hinf, el cual al ser un control robusto permite un segumiento a pesar de las incertidumbres en el modelo de la planta. 

---

##  Conclusiones
- El modelo matemático reproduce con buena aproximación el comportamiento de la planta en su rango operativo.  
- Se identificó la necesidad de incluir fenómenos no lineales y retardos del actuador en versiones futuras.  
- El controlador H∞ mostró un desempeño robusto, aunque requirió ajustes prácticos en la implementación física.  


