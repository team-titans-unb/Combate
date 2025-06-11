
#define CUSTOM_SETTINGS
#define INCLUDE_GAMEPAD_MODULE
#include <DabbleESP32.h>

int RPWM_Output_1 = 18; 
int LPWM_Output_1 = 19;

int RPWM_Output_2 = 17; 
int LPWM_Output_2 = 16; 

const int freq = 5000;
const int rChannel_1 = 0;
const int lChannel_1 = 1;
const int rChannel_2 = 2;
const int lChannel_2 = 3;
const int resolution = 8;

int velocidade = 0;

int velocidadeEsc = 75;
const int escChannel = 4;
const int escPin = 13;
const int escFreq = 60;  

bool estado = false;

void setup() {
  Serial.begin(115200);
  Dabble.begin("Beetle prototipo");  //set bluetooth name of your device

  ledcSetup(rChannel_1, freq, resolution);
  ledcSetup(lChannel_1, freq, resolution);
  ledcSetup(rChannel_2, freq, resolution);
  ledcSetup(lChannel_2, freq, resolution);

  ledcAttachPin(RPWM_Output_1, rChannel_1);
  ledcAttachPin(LPWM_Output_1, lChannel_1);
  ledcAttachPin(RPWM_Output_2, rChannel_2);
  ledcAttachPin(LPWM_Output_2, lChannel_2);

  ledcSetup(escChannel, escFreq, resolution);
  ledcAttachPin(escPin, escChannel);
  ledcWrite(escChannel, velocidadeEsc);

}

void loop() {
  Dabble.processInput();             //this function is used to refresh data obtained from smartphone.Hence calling this function is mandatory in order to get data properly from your mobile.
  Serial.print("Motor: ");
  Serial.print(estado);
  Serial.print("  KeyPressed: ");

  if (GamePad.isUpPressed())
  {
    Serial.print("Up");
  }

  if (GamePad.isDownPressed())
  {
    Serial.print("Down");
  }

  if (GamePad.isLeftPressed())
  {
    Serial.print("Left");
  }

  if (GamePad.isRightPressed())
  {
    Serial.print("Right");
  }

  if (GamePad.isSquarePressed())//esquerda
  {
    Serial.print("Square");
    ledcWrite(rChannel_1, 0);
    ledcWrite(lChannel_1, 125);
    ledcWrite(rChannel_2, 125);
    ledcWrite(lChannel_2, 0);
    delay(200);
  }

  if (GamePad.isCirclePressed())//direita
  {
    Serial.print("Circle");
    ledcWrite(rChannel_1, 125);
    ledcWrite(lChannel_1, 0);
    ledcWrite(rChannel_2, 0);
    ledcWrite(lChannel_2, 125);
    delay(200);
  }

  if (GamePad.isCrossPressed())
  {
    Serial.print("Cross ");
    estado =! estado; 
    if (estado == true){
      ledcWrite(escChannel, 90);
    } else{
    ledcWrite(escChannel, 75);
    }
    delay(300);
  }

  if (GamePad.isTrianglePressed())
  {
    Serial.print("Triangle");
  }

  if (GamePad.isStartPressed())
  {
    Serial.print("Start");
  }

  if (GamePad.isSelectPressed())
  {
    Serial.print("Select");
  }
  Serial.print('\t');

  int a = GamePad.getAngle();
  Serial.print("Angle: ");
  Serial.print(a);
  Serial.print('\t');
  int b = GamePad.getRadius();
  Serial.print("Radius: ");
  Serial.print(b);
  Serial.print('\t');

  int rotacao = 0;
  rotacao = map(GamePad.getXaxisData(), -7, 7, -255, 255);
  Serial.print("x_axis: ");
  Serial.print(rotacao);
  Serial.print('\t');

  velocidade = map(GamePad.getYaxisData(), -7, 7, -255, 255);
  Serial.print("y_axis: ");
  Serial.println(velocidade);
  Serial.println();

  if (velocidade < -50) { //pra trás
    ledcWrite(rChannel_1, 0);//velocidade*(-1)
    ledcWrite(lChannel_1, velocidade*(-1));
    ledcWrite(rChannel_2, 0);//velocidade*(-1)
    ledcWrite(lChannel_2, velocidade*(-1));
  }

  if (velocidade > 50) { //pra frente
    ledcWrite(rChannel_1, velocidade);
    ledcWrite(lChannel_1, 0);//velocidade
    ledcWrite(rChannel_2, velocidade);
    ledcWrite(lChannel_2, 0);//velocidade
  }

  else if((velocidade >= -50) && (velocidade <= 50)){
    ledcWrite(rChannel_1, 0);
    ledcWrite(lChannel_1, 0);
    ledcWrite(rChannel_2, 0);
    ledcWrite(lChannel_2, 0);
  }
}
