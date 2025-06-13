#include <Ps3Controller.h>

// --- Pinos para o Motor Driver (H-Bridge) ---
// Motor 1 (Direito)
int RPWM_Output_1 = 18; // para frente 19, 18, 17, 16
int LPWM_Output_1 = 19; // para trás

// Motor 2 (Esquerdo)
int RPWM_Output_2 = 16; // para frente
int LPWM_Output_2 = 17; // para trás

// --- Configurações de PWM para os motores DC ---
const int  freq = 5000;
const int  resolution = 8; 
// Canais PWM
const int  rChannel_1 = 0;
const int  lChannel_1 = 1;
const int  rChannel_2 = 2;
const int  lChannel_2 = 3;

// --- Configurações para o ESC ---
const int  escPin = 13;
const int  escChannel = 4;
const int  escFreq = 60; // Frequência comum para ESCs
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
    if (Ps3.event.button_down.l2) {
      estadoMotorEsc = !estadoMotorEsc;
      if (estadoMotorEsc) {
        ledcWrite(escChannel, velocidadeEscLigado);
        Serial.println("Motor ESC LIGADO");
      } else {
        ledcWrite(escChannel, velocidadeEscDesligado);
        Serial.println("Motor ESC DESLIGADO");
      }
    }

    // --- Controle de Movimento do Robô ---
    if (Ps3.data.button.square) {
      driveMotor(1, 150);
      driveMotor(2, -150);
    } else if (Ps3.data.button.circle) {
      driveMotor(1, -150);
      driveMotor(2, 150);
    } else {
      int velocidadeFrente = map(Ps3.data.analog.stick.ly, -128, 127, 255, -255);
      int velocidadeLado = map(Ps3.data.analog.stick.rx, -128, 127, -255, 255);
      if (Ps3.data.button.triangle){
      Serial.println(velocidadeFrente);
      Serial.println("velocidadeFrente");//apagar
      Serial.println(velocidadeLado);
      Serial.println("velocidadelado");
      Serial.println("*************");}

      int velMotorDireito;
      int velMotorEsquerdo;

      if (velocidadeFrente > 20 && velocidadeLado > 20) { // Frente e virando para a direita
        velMotorDireito = velocidadeLado / 2; //tornar a curva suave 
        velMotorEsquerdo = velocidadeFrente / 1.5;
      } else if (velocidadeFrente > 20 && velocidadeLado < -20) { // Frente e virando para a esquerda
        velMotorDireito = velocidadeFrente / 1.5;
        velMotorEsquerdo = velocidadeLado / 2;
      } else if (velocidadeFrente < -20 && velocidadeLado > 20) { // Ré e virando para a direita
        velMotorDireito = -velocidadeLado / 2;
        velMotorEsquerdo = velocidadeFrente/1.5;
      } else if (velocidadeFrente < -20 && velocidadeLado < -20) { // Ré e virando para a esquerda
        velMotorDireito = velocidadeFrente / 2;
        velMotorEsquerdo = -velocidadeLado / 1.5;
      } else if (abs(velocidadeFrente) > 20 && abs(velocidadeLado) <= 20) { // Apenas frente ou trás em linha reta
        velMotorDireito = velocidadeFrente;//subtrair 50 do motor mais forte para ir em linha reta (?)
        velMotorEsquerdo = velocidadeFrente;
      } else if (abs(velocidadeFrente) <= 20 && abs(velocidadeLado) > 20) {// Giro no próprio eixo
        velMotorDireito = -velocidadeLado;
        velMotorEsquerdo = velocidadeLado;
      } else {
        // Parado
        velMotorDireito = 0;
        velMotorEsquerdo = 0;
      }

      velMotorDireito = constrain(velMotorDireito, -255, 255);
      velMotorEsquerdo = constrain(velMotorEsquerdo, -255, 255);

      Serial.println("direção do motor direito");
      Serial.println(velMotorDireito);
      Serial.println("direção do motor esquerdo");
      Serial.println(velMotorEsquerdo);

      driveMotor(1, velMotorDireito);
      driveMotor(2, velMotorEsquerdo);
    }
  } else {
    driveMotor(1, 0);
    driveMotor(2, 0);
    if (estadoMotorEsc) {
      ledcWrite(escChannel, velocidadeEscDesligado);
      estadoMotorEsc = false;
    }
  }
}

// O número do motor (1 para direito, 2 para esquerdo).
// A velocidade e direção (-255 a 255). Negativo para ré, Positivo para frente, 0 para parar.
void driveMotor(int motorNum, int speed) {
  int absSpeed = abs(speed);
  int finalSpeed = constrain(absSpeed, 0, 255);

  int rPwmChannel, lPwmChannel;

  if (motorNum == 1) {
    rPwmChannel = rChannel_1;
    lPwmChannel = lChannel_1;
  } else {
    rPwmChannel = rChannel_2;
    lPwmChannel = lChannel_2;
  }

  if (speed > 20) {
    ledcWrite(rPwmChannel, finalSpeed);
    ledcWrite(lPwmChannel, 0);
  } else if (speed < -20) {
    ledcWrite(rPwmChannel, 0);
    ledcWrite(lPwmChannel, finalSpeed);
  } else {
    ledcWrite(rPwmChannel, 0);
    ledcWrite(lPwmChannel, 0);
  }
}
