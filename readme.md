# RC PLANE inspired by :
        https://www.rcpano.net/2025/09/30/make-a-flying-wing-with-simple-materials-diy-rc-plane-remote-control/

### Transmetteur pour avion RC, 3 channel, 1 moteur et 2 servo. ESP32, NRF24L01 et OLED integré.

## Revue de code : 
    - Adapté à l'ideaspark, optimisation du mappage des joysticks pour les plages de l'ESP32, ajout de messages d'erreur pour le NRF24L01, amélioration de l'affichage des commandes, et préparation pour les futures améliorations du circuit.
## Revue du circuit : 
    - Sécurité des GPIO, alimentation 7.4V réduit à 3.3V pouir le NRF24L01, ajout de condensateurs de découplage, protection contre les interférences électromagnétiques à venir...

#include <SPI.h>
#include <RF24.h>
#include <U8g2lib.h>

![schema](./img/Transmetteur-open-source-berrug-SCHEMA.png)

![pcb](./img/Transmetteur-open-source-berrug-PCB.png)

![3D](./img/Transmetteur-open-source-berrug-3D.png)
