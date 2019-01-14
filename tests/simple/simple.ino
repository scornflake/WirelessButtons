int ledPIN = 17;
int controlPin = 11;
int lDelay = 150;

bool shouldBeUp = true;

void setup()
{
  pinMode(ledPIN, OUTPUT);
  pinMode(controlPin, OUTPUT);
  digitalWrite(controlPin, HIGH);
}

void loop()
{
  if (shouldBeUp) {
    for (int i = 0; i < 10; i++)
    {
      digitalWrite(ledPIN, LOW); // on
      delay(lDelay);
      digitalWrite(ledPIN, HIGH); // off
      delay(lDelay / 2);
    }

    // Leave the LED low (so, ground == ON)
    digitalWrite(ledPIN, LOW);
    digitalWrite(controlPin, LOW);
    shouldBeUp = false;
  }
}
