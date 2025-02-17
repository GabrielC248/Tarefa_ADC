# Tarefa do EmbarcaTech Sobre ADC para Controle de LEDs e Display com  o Joystick da BitDogLab
  - **Desenvolvedor:** <ins>Gabriel Cavalcanti Coelho</ins>;
  - **Vídeo:** [YouTube](https://www.youtube.com/).

### Este projeto implementa o ADC na placa BitDogLab para controlar um LED RGB e um display utilizando um joystick. O código contempla a leitura das entradas analógicas do joystick, o controle de brilho de LEDs via PWM, a exibição de um quadrado móvel e de diferentes bordas no display e interrupção no botão A e no botão do joystick.

**Funcionalidades:**
  1. **Controle do LED RGB:**
     - O brilho do LED vermelho é ajustado com base na posição do joystick no eixo X (máximo nos extremos e mínimo no meio);
     - O brilho do LED azul é ajustado com base na posição do joystick no eixo Y (máximo nos extremos e mínimo no meio);
     - O LED verde alterna entre ligado e desligado ao pressionar o botão do joystick.
  2. **Display:**
     - Mostra um quadrado que se move de acordo com a posição do joystick;
     - Exibe bordas que alternam de estado ao pressionar o botão do joystick.
  3. **Botão A:**
     - Habilita e desabilita os LEDs PWM (vermelho e azul) a cada acionamento.

**Observações:**
  - O joystick do simulador tem a leitura do eixo X invertida em relação ao joystick da BitDogLab, portanto, o eixo X aparece invertido no display também.
