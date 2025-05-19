# 🌊 Monitoramento de Cheias com FreeRTOS – EmbarcaTech

Este projeto simula um sistema embarcado para monitoramento de cheias e chuvas utilizando a placa **Raspberry Pi Pico W** com o sistema **FreeRTOS**.

---

## 📷 Demonstração

[🔗 Link para o vídeo](https://www.youtube.com/watch?v=yxLL56xPt_c&ab_channel=LorenzoBaroni)

---

## 🎯 Objetivos

- Implementar multitarefas com FreeRTOS (uso de `xTaskCreate`, `queue`)
- Simular sensores via joystick e ADC
- Exibir informações em display OLED
- Acionar alertas visuais e sonoros automaticamente
- Tornar o sistema acessível a pessoas com deficiência visual

---

## 🚨 Modo Alerta

| Critério                              | Ação Realizada                                                      |
|--------------------------------------|---------------------------------------------------------------------|
| Nível de água ≥ 70% ou chuva ≥ 80%   | LED RGB vermelho, buzzer intermitente, matriz WS2812 com símbolo 'X'|

---

## 🧠 Acessibilidade

A `vInclusaoTask` oferece **feedback sonoro** com padrões diferenciados:

| Situação         | Buzzer                                      |
|------------------|----------------------------------------------|
| Nível seguro     | 1 beep leve                                  |
| Risco moderado   | 2 beeps médios                               |
| Alerta           | 3 beeps longos com pausa                     |

---

## 🧱 Tasks Principais

| Task            | Descrição                                                         |
|-----------------|-------------------------------------------------------------------|
| `vJoystickTask` | Lê posição do joystick e envia para fila                          |
| `vDisplayTask`  | Converte dados em % e exibe no display                            |
| `vAlertaTask`   | Aciona alerta RGB, buzzer e matriz de LEDs                        |
| `vInclusaoTask` | Emite sons diferenciados conforme risco (acessibilidade sonora)   |

---

## 🧰 Tecnologias

- C + FreeRTOS
- Raspberry Pi Pico W
- OLED SSD1306 (via I2C)
- Matriz WS2812 (PIO)
- ADC (Joystick)
- GPIOs (LEDs e buzzer)

---

## 🛠️ Compilação

Requer o **Pico SDK**. Para compilar:

```bash
mkdir build && cd build
cmake ..
make
```

> 💡 Certifique-se de ter o SDK da Raspberry Pi Pico configurado corretamente.

---

## 📝 Licença
Este programa foi desenvolvido como um exemplo educacional e pode ser usado livremente para fins de estudo e aprendizado.

## 📌 Autor
LORENZO GIUSEPPE OLIVEIRA BARONI

