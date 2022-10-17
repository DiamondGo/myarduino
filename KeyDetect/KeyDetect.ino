
const char* PIN_NAME[] = {
  "", "",
  "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16",
  "",
  "A0", "A1", "A2", "A3", "A4", "A5"
};
#define PIN_EXIST(pin) (strlen(PIN_NAME[pin]) > 0)

const int MAX_PIN = A5;

int PIN_STATE[MAX_PIN +1];
#define GET_PIN_STATE(pin) PIN_STATE[pin]
#define SET_PIN_STATE(pin, vol) PIN_STATE[pin] = vol

void setupPinButtons() {
  for (int pin = 0; pin <= MAX_PIN; pin++) {
    if (PIN_EXIST(pin)) {
      pinMode(pin, INPUT_PULLUP);
      SET_PIN_STATE(pin, LOW);
    }
  }
}

void setup() {
  // put your setup code here, to run once:
  setupPinButtons();
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  char buffer[40];
  /*
  //sprintf(buffer, "pin %s", PIN_NAME[A5]);
  //sprintf(buffer, "HIGH %d", HIGH);
  //sprintf(buffer, "LOW %d", LOW);
  sprintf(buffer, "Max pin %d", MAX_PIN);
  Serial.println(buffer);
  delay(3000);
  */

  
  for (int pin = 0; pin <= MAX_PIN; pin++) {
    if (PIN_EXIST(pin)) {
      int pinState = digitalRead(pin);
      if (pinState != GET_PIN_STATE(pin)) {
        sprintf(buffer, "pin %s change to %d", PIN_NAME[pin], pinState);
        Serial.println(buffer);
        SET_PIN_STATE(pin, pinState);
      }
    }
  }
  
}
