/*

Verifique se a versão da board da esp32 é a 2.0.17 ou inferior

Neste código foi-se utilizado uma "board" chamada BluePad32
Para instalar a board vá em File > Preferences e em "Aditional boards managers URLs" cole o link a seguir:
https://raw.githubusercontent.com/ricardoquesada/esp32-arduino-lib-builder/master/bluepad32_files/package_esp32_bluepad32_index.json

A board possui suporte para a maioria dos controles de videogame, teclados e mouses modernos como:
- Sony DualSense (PS5)
- Sony DualShock 4 (PS4)
- Sony DualShock 3 (PS3)
- Nintendo Switch Pro controller
- Nintendo Switch JoyCon
- Nintendo Wii U controller
- Nintendo Wii Remote + accessories
- Xbox Wireless controller (models 1708, 1914, adaptive)
- Android controllers
- Steam controller
- Stadia controller
- PC/Windows controller
- 8BitDo controllers
- Atari joystick
- iCade
- Mouse
- Keyboards
- DataFrog S80
*/ 

#include <Bluepad32.h>

// --- Pinos para o Motor Driver (H-Bridge) ---
// Motor 1 (Direito)
int RPWM_Output_1 = 18; // para frente 19, 18, 17, 16
int LPWM_Output_1 = 19; // para trás

// Motor 2 (Esquerdo)
int RPWM_Output_2 = 16; // para frente
int LPWM_Output_2 = 17; // para trás

// --- Configurações de PWM para os motores DC ---
const int freq = 5000;
const int resolution = 8; 
// Canais PWM
const int rChannel_1 = 0;
const int lChannel_1 = 1;
const int rChannel_2 = 2;
const int lChannel_2 = 3;

// --- Configurações para o ESC ---
const int escPin = 13;
const int escChannel = 4;
const int escFreq = 60; // Frequência comum para ESCs
int velocidadeEscMin = 75;  // Valor PWM mínimo (parado)
int velocidadeEscMax = 120; // Valor PWM máximo
int velocidadeEscAtual = velocidadeEscMin; // Velocidade atual
bool estadoMotorEsc = false; // false = desligado, true = ligado
bool r2PressedLast = false;   // Para detectar mudança no botão R2

// --- Objeto para o controle ---
ControllerPtr myController;

// --- Protótipo da função de controle dos motores ---
void driveMotor(int motorNum, int speed);

void onConnectedController(ControllerPtr ctl) {
    if (myController == nullptr) {
        myController = ctl;
        Serial.println("Controle conectado!");
    }
}

void onDisconnectedController(ControllerPtr ctl) {
    if (myController == ctl) {
        myController = nullptr;
        Serial.println("Controle desconectado!");
    }
}

void setup() {
    Serial.begin(115200);

    // --- Configuração dos canais PWM para os motores DC ---
    ledcSetup(rChannel_1, freq, resolution);
    ledcSetup(lChannel_1, freq, resolution);
    ledcSetup(rChannel_2, freq, resolution);
    ledcSetup(lChannel_2, freq, resolution);

    // --- Associa os pinos aos canais PWM ---
    ledcAttachPin(RPWM_Output_1, rChannel_1);
    ledcAttachPin(LPWM_Output_1, lChannel_1);
    ledcAttachPin(RPWM_Output_2, rChannel_2);
    ledcAttachPin(LPWM_Output_2, lChannel_2);

    // --- Configuração do canal PWM para o ESC ---
    ledcSetup(escChannel, escFreq, resolution);
    ledcAttachPin(escPin, escChannel);
    ledcWrite(escChannel, velocidadeEscMin); // Garante que o ESC comece desligado

    // --- Inicialização do Bluepad32 ---
    BP32.setup(&onConnectedController, &onDisconnectedController);
    BP32.forgetBluetoothKeys();
    
    Serial.println("=== SISTEMA PRONTO ===");
    Serial.println("Controles:");
    Serial.println("R2 (Trigger direito): Ativa/Desativa arma");
    Serial.println("L2: Aumenta velocidade arma");
    Serial.println("----------------------");
    printStatusArma();
}

void loop() {
    BP32.update();

    if (myController && myController->isConnected()) {
        // --- Controle do Motor com ESC (Liga/Desliga com trigger R2) ---
        bool r2Pressed = (myController->throttle() > 0); // Trigger R2 pressionado
        if (r2Pressed && !r2PressedLast) {
            estadoMotorEsc = !estadoMotorEsc;
            if (estadoMotorEsc) {
                velocidadeEscAtual = velocidadeEscMin + 5; // Liga com velocidade mínima
                Serial.println(">> ARMA LIGADA <<");
            } else {
                velocidadeEscAtual = velocidadeEscMin; // Desliga
                Serial.println(">> ARMA DESLIGADA <<");
            }
            ledcWrite(escChannel, velocidadeEscAtual);
            printStatusArma();
        }
        r2PressedLast = r2Pressed;

        // --- Controle de Velocidade da Arma (L2) ---
        if (estadoMotorEsc) {
            // L2 - Aumenta velocidade
            if (myController->brake() > 0) { // L2 pressionado
                velocidadeEscAtual = constrain(velocidadeEscAtual + 1, velocidadeEscMin, velocidadeEscMax);
                ledcWrite(escChannel, velocidadeEscAtual);
                Serial.println("L2: ↑ Aumentando velocidade arma ↑");
                printStatusArma();
                delay(100);
            }
        }

        // --- Controle de Movimento do Robô ---
        if (myController->buttons() & 0x1000) {  // Square button - Girar para a direita
            driveMotor(1, -150);  // Motor direito para trás
            driveMotor(2, 150);   // Motor esquerdo para frente
        } else if (myController->buttons() & 0x2000) {  // Circle button - Girar para a esquerda
            driveMotor(1, 150);   // Motor direito para frente
            driveMotor(2, -150);  // Motor esquerdo para trás
        } else {
            int velocidadeFrente = -map(myController->axisY(), -512, 511, -255, 255);  // <-- Invertido para frente correto
            int velocidadeLado = map(myController->axisRX(), -512, 511, 255, -255);

            if (myController->buttons() & 0x100) {  // Triangle button - Debug
                Serial.println(velocidadeFrente);
                Serial.println("velocidadeFrente");
                Serial.println(velocidadeLado);
                Serial.println("velocidadelado");
                Serial.println("*************");
            }

            int velMotorDireito = velocidadeFrente - velocidadeLado;
            int velMotorEsquerdo = velocidadeFrente + velocidadeLado;

            // Normalização para manter proporção
            int maxVal = max(abs(velMotorDireito), abs(velMotorEsquerdo));
            if (maxVal > 255) {
                velMotorDireito = (velMotorDireito * 255) / maxVal;
                velMotorEsquerdo = (velMotorEsquerdo * 255) / maxVal;
            }

            driveMotor(1, velMotorDireito);
            driveMotor(2, velMotorEsquerdo);

            // Debug
            Serial.print("Motor D: ");
            Serial.print(velMotorDireito);
            Serial.print(" | Motor E: ");
            Serial.println(velMotorEsquerdo);
        }
    } else {
        driveMotor(1, 0);
        driveMotor(2, 0);
        if (estadoMotorEsc) {
            ledcWrite(escChannel, velocidadeEscMin);
            estadoMotorEsc = false;
        }
    }
}

// Função para imprimir status da arma
void printStatusArma() {
    Serial.print("Status arma: ");
    Serial.print(estadoMotorEsc ? "LIGADA" : "DESLIGADA");
    Serial.print(" | Velocidade: ");
    Serial.print(velocidadeEscAtual);
    Serial.print(" (");
    Serial.print(map(velocidadeEscAtual, velocidadeEscMin, velocidadeEscMax, 0, 100));
    Serial.println("%)");
    Serial.println("----------------------");
}

// Função para controle dos motores DC
void driveMotor(int motorNum, int speed) {
    if (motorNum == 2) {
        speed = -speed;  // Inverte sentido do motor esquerdo para corrigir direção
    }

    int absSpeed = abs(speed);
    int finalSpeed = constrain(absSpeed, 0, 255);

    int rPwmChannel, lPwmChannel;

    if (motorNum == 1) {  // Motor direito
        rPwmChannel = rChannel_1;
        lPwmChannel = lChannel_1;
    } else {  // Motor esquerdo
        rPwmChannel = rChannel_2;
        lPwmChannel = lChannel_2;
    }

    if (speed > 20) {  // Para frente
        ledcWrite(rPwmChannel, finalSpeed);
        ledcWrite(lPwmChannel, 0);
    } else if (speed < -20) {  // Para trás
        ledcWrite(rPwmChannel, 0);
        ledcWrite(lPwmChannel, finalSpeed);
    } else {  // Parado
        ledcWrite(rPwmChannel, 0);
        ledcWrite(lPwmChannel, 0);
    }
}

