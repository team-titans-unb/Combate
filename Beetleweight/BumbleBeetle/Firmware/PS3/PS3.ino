
#include <Ps3Controller.h>

// --- Pinos para o Motor Driver (H-Bridge) ---
// Motor 1 (Direito)
int RPWM_Output_1 = 18; // PWM para frente
int LPWM_Output_1 = 19; // PWM para trás

// Motor 2 (Esquerdo)
int RPWM_Output_2 = 17; // PWM para frente
int LPWM_Output_2 = 16; // PWM para trás

// --- Configurações de PWM para os motores DC ---
const int freq = 5000;
const int resolution = 8; // Resolução de 8 bits (0-255)
// Canais PWM
const int rChannel_1 = 0;
const int lChannel_1 = 1;
const int rChannel_2 = 2;
const int lChannel_2 = 3;

// --- Configurações para o ESC ---
const int escPin = 13;
const int escChannel = 4;
const int escFreq = 60; // Frequência comum para ESCs
int velocidadeEscDesligado = 75; // Valor PWM para o ESC parado
int velocidadeEscLigado = 90;    // Valor PWM para o ESC em baixa rotação
bool estadoMotorEsc = false;   // false = desligado, true = ligado

// --- Protótipo da função de controle dos motores ---
void driveMotor(int motorNum, int speed);

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
  ledcWrite(escChannel, velocidadeEscDesligado); // Garante que o ESC comece desligado

  // --- Inicialização do Controle PS3 ---

  Ps3.begin("74:85:76:73:65:71"); 
  
  Serial.println("Aguardando conexão do controle PS3...");
}

void loop() {
  if (Ps3.isConnected()) {
    // --- Controle do Motor com ESC (Liga/Desliga) ---

    if (Ps3.event.button_down.cross) {
      estadoMotorEsc = !estadoMotorEsc; // Inverte o estado (liga/desliga)
      if (estadoMotorEsc) {
        ledcWrite(escChannel, velocidadeEscLigado);
        Serial.println("Motor ESC LIGADO");
      } else {
        ledcWrite(escChannel, velocidadeEscDesligado);
        Serial.println("Motor ESC DESLIGADO");
      }
    }

    // --- Controle de Movimento do Robô ---

    if (Ps3.data.button.square) { // Gira para a esquerda
      driveMotor(1, 150);  // Motor direito para frente
      driveMotor(2, -150); // Motor esquerdo para trás
    } else if (Ps3.data.button.circle) { // Gira para a direita
      driveMotor(1, -150); // Motor direito para trás
      driveMotor(2, 150);  // Motor esquerdo para frente
    } else {
      // Controle de velocidade e curva pelos analógicos
      // Analógico Esquerdo (Y): Para frente e para trás
      // Analógico Direito (X): Para esquerda e para direita
      int stickFrente = Ps3.data.analog.stick.ly; // -128 (frente) a 127 (trás)
      int stickLado = Ps3.data.analog.stick.rx;   // -128 (esquerda) a 127 (direita)

      // Mapeia os valores dos analógicos para o range de velocidade do PWM (-255 a 255)
      int velocidadeFrente = map(stickFrente, -128, 127, 255, -255);
      int velocidadeLado = map(stickLado, -128, 127, -255, 255);

      // Algoritmo de mixagem para controle tipo "tanque"
      int velMotorDireito = velocidadeFrente - velocidadeLado;
      int velMotorEsquerdo = velocidadeFrente + velocidadeLado;

      // Limita os valores para garantir que fiquem no range do PWM
      velMotorDireito = constrain(velMotorDireito, -255, 255);
      velMotorEsquerdo = constrain(velMotorEsquerdo, -255, 255);

      driveMotor(1, velMotorDireito);
      driveMotor(2, velMotorEsquerdo);
    }
  } else {
    // Se o controle desconectar, para tudo por segurança
    driveMotor(1, 0);
    driveMotor(2, 0);
    if(estadoMotorEsc){
      ledcWrite(escChannel, velocidadeEscDesligado);
      estadoMotorEsc = false;
    }
  }
}

void driveMotor(int motorNum, int speed) {
  int absSpeed = abs(speed); // Velocidade absoluta
  int finalSpeed = constrain(absSpeed, 0, 255); // Garante que a velocidade está entre 0-255

  int rPwmChannel, lPwmChannel;

  if (motorNum == 1) { // Motor Direito
    rPwmChannel = rChannel_1;
    lPwmChannel = lChannel_1;
  } else { // Motor Esquerdo
    rPwmChannel = rChannel_2;
    lPwmChannel = lChannel_2;
  }

  if (speed > 10) { // Mover para frente (com pequena zona morta)
    ledcWrite(rPwmChannel, finalSpeed);
    ledcWrite(lPwmChannel, 0);
  } else if (speed < -10) { // Mover para trás (com pequena zona morta)
    ledcWrite(rPwmChannel, 0);
    ledcWrite(lPwmChannel, finalSpeed);
  } else { // Parar
    ledcWrite(rPwmChannel, 0);
    ledcWrite(lPwmChannel, 0);
  }
}
