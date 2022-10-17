
int buttonPin = 3;

void setup() {
  // put your setup code here, to run once:

  
  pinMode(buttonPin, INPUT_PULLUP);
  Serial.begin(9600);
  
}

int buttonStatus = 0;

void loop() {
  // put your main code here, to run repeatedly:
  int value = digitalRead(buttonPin);
  if (value == LOW) {
    Serial.println("LOW");
  }
}
