#include <Bluepad32.h>

//motor 1
int RPWM_Output_1 = 19; //motor 1 pra frente
int LPWM_Output_1 = 18; //motor 1 pra trás

//motor 2
int RPWM_Output_2 = 16; //motor 2 pra frente
int LPWM_Output_2 = 17; //motor 2 pra trás

const int freq = 5000;
const int rChannel_1 = 0;
const int lChannel_1 = 1;
const int rChannel_2 = 2;
const int lChannel_2 = 3;
const int resolution = 8;

int velocidade = 0;

// Esc e motor brushless
int velocidadeEsc = 75; //velocidade inicial da ESC
const int escChannel = 4;
const int escPin = 13;
const int escFreq = 60;  

bool estado = false; //estado da arma

ControllerPtr myControllers[BP32_MAX_GAMEPADS];

// This callback gets called any time a new gamepad is connected.
// Up to 4 gamepads can be connected at the same time.
void onConnectedController(ControllerPtr ctl) {
    bool foundEmptySlot = false;
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (myControllers[i] == nullptr) {
            Serial.printf("CALLBACK: Controller is connected, index=%d\n", i);
            // Additionally, you can get certain gamepad properties like:
            // Model, VID, PID, BTAddr, flags, etc.
            ControllerProperties properties = ctl->getProperties();
            Serial.printf("Controller model: %s, VID=0x%04x, PID=0x%04x\n", ctl->getModelName().c_str(), properties.vendor_id,
                           properties.product_id);
            myControllers[i] = ctl;
            foundEmptySlot = true;
            break;
        }
    }
    if (!foundEmptySlot) {
        Serial.println("CALLBACK: Controller connected, but could not found empty slot");
    }
}

void onDisconnectedController(ControllerPtr ctl) {
    bool foundController = false;

    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (myControllers[i] == ctl) {
            Serial.printf("CALLBACK: Controller disconnected from index=%d\n", i);
            myControllers[i] = nullptr;
            foundController = true;
            break;
        }
    }

    if (!foundController) {
        Serial.println("CALLBACK: Controller disconnected, but not found in myControllers");
    }
}

void dumpGamepad(ControllerPtr ctl) {
    Serial.printf(
        "idx=%d, dpad: 0x%02x, buttons: 0x%04x, axis L: %4d, %4d, axis R: %4d, %4d, brake: %4d, throttle: %4d, "
        "misc: 0x%02x\n",
        ctl->index(),        // Controller Index
        ctl->dpad(),         // D-pad
        ctl->buttons(),      // bitmask of pressed buttons
        ctl->axisX(),        // (-511 - 512) left X Axis
        ctl->axisY(),        // (-511 - 512) left Y axis
        ctl->axisRX(),       // (-511 - 512) right X axis
        ctl->axisRY(),       // (-511 - 512) right Y axis
        ctl->brake(),        // (0 - 1023): brake button
        ctl->throttle(),     // (0 - 1023): throttle (AKA gas) button
        ctl->miscButtons()  // bitmask of pressed "misc" buttons
    );
}

void processGamepad(ControllerPtr ctl) {

    //Cor do led controle Ps4
    if (ctl->a()) {
        static int colorIdx = 0;
        // Some gamepads like DS4 and DualSense support changing the color LED.
        // It is possible to change it by calling:
        switch (colorIdx % 3) {
            case 0:
                // Red
                ctl->setColorLED(255, 0, 0);
                break;
            case 1:
                // Green
                ctl->setColorLED(0, 255, 0);
                break;
            case 2:
                // Blue
                ctl->setColorLED(0, 0, 255);
                break;
        }
        colorIdx++;
    }

    //Indicador de player (controle Ps3)
    if (ctl->b()) {
        // Turn on the 4 LED. Each bit represents one LED.
        static int led = 0;
        led++;
        // Some gamepads like the DS3, DualSense, Nintendo Wii, Nintendo Switch
        // support changing the "Player LEDs": those 4 LEDs that usually indicate
        // the "gamepad seat".
        // It is possible to change them by calling:
        ctl->setPlayerLEDs(led & 0x0f);
    }

    //seta pra cima direciona o robô pra frente com valor fixo
    if (ctl -> buttons() == 0x01) {
      ledcWrite(rChannel_1, 250);
      ledcWrite(lChannel_1, 0);
      ledcWrite(rChannel_2, 250);
      ledcWrite(lChannel_2, 0); 
    }

    //seta pra baixo direciona o robô pra trás com valor fixo
    if (ctl -> buttons() == 0x02) {
      ledcWrite(rChannel_1, 0);//velocidade*(-1)
      ledcWrite(lChannel_1, 200);
      ledcWrite(rChannel_2, 0);//velocidade*(-1)
      ledcWrite(lChannel_2, 200); 
    }

    //seta para direita gira o robô no próprio eixo para direita
    if (ctl -> buttons() == 0x04){
      ledcWrite(rChannel_1, 125);
      ledcWrite(lChannel_1, 0);
      ledcWrite(rChannel_2, 0);
      ledcWrite(lChannel_2, 125);
      delay(200);
    }

    //seta para esquerda gira o robô no próprio eixo para esquerda
    if (ctl -> buttons() == 0x08){
      ledcWrite(rChannel_1, 0);
      ledcWrite(lChannel_1, 125);
      ledcWrite(rChannel_2, 125);
      ledcWrite(lChannel_2, 0);
      delay(200);
    }

    //Gatilho R2, responsável por ligar/desligar a arma
    if (ctl -> buttons() == 0x0080){
      estado =! estado; 
      if (estado == true){
        ledcWrite(escChannel, 90);
        ctl->playDualRumble(0 /* delayedStartMs */, 1000 /* durationMs */, 0x80 /* weakMagnitude */, 0x40 /* strongMagnitude */);
      } else{
        ledcWrite(escChannel, 0);
      }
    delay(300);
    } 


    dumpGamepad(ctl);
}

void processControllers() {
    for (auto myController : myControllers) {
        if (myController && myController->isConnected() && myController->hasData()) {
            if (myController->isGamepad()) {
                processGamepad(myController);
            } else {
                Serial.println("Unsupported controller");
            }
        }
    }
}

// Arduino setup function. Runs in CPU 1
void setup() {
    Serial.begin(115200);
    Serial.printf("Firmware: %s\n", BP32.firmwareVersion());
    const uint8_t* addr = BP32.localBdAddress();
    Serial.printf("BD Addr: %2X:%2X:%2X:%2X:%2X:%2X\n", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);

    // Setup the Bluepad32 callbacks
    BP32.setup(&onConnectedController, &onDisconnectedController);

    // "forgetBluetoothKeys()" should be called when the user performs
    // a "device factory reset", or similar.
    // Calling "forgetBluetoothKeys" in setup() just as an example.
    // Forgetting Bluetooth keys prevents "paired" gamepads to reconnect.
    // But it might also fix some connection / re-connection issues.
    BP32.forgetBluetoothKeys();

    // Enables mouse / touchpad support for gamepads that support them.
    // When enabled, controllers like DualSense and DualShock4 generate two connected devices:
    // - First one: the gamepad
    // - Second one, which is a "virtual device", is a mouse.
    // By default, it is disabled.
    BP32.enableVirtualDevice(false);

    //Configurando a frequencia, resolução e attachando os pinos da esp
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

// Arduino loop function. Runs in CPU 1.
void loop() {
    // This call fetches all the controllers' data.
    // Call this function in your main loop.
    bool dataUpdated = BP32.update();
    if (dataUpdated)
        processControllers();

    // The main loop must have some kind of "yield to lower priority task" event.
    // Otherwise, the watchdog will get triggered.
    // If your main loop doesn't have one, just add a simple `vTaskDelay(1)`.
    // Detailed info here:
    // https://stackoverflow.com/questions/66278271/task-watchdog-got-triggered-the-tasks-did-not-reset-the-watchdog-in-time

    //     vTaskDelay(1);
    delay(150);
}
