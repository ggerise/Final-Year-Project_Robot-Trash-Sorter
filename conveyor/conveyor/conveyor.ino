#define IN1 2 //motor driver pins
#define IN2 3
#define IN3 4
#define IN4 5
#define ENA 9
#define ENB 10

#define POT_PIN A0 //potentiometer pin

void setup() {
  // Set motor driver pins as outputs
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  
  // Initialize serial communication for debugging
  Serial.begin(9600);
}

void loop() {
  // Read the potentiometer value
  int potValue = analogRead(POT_PIN);
  
  // Map the potentiometer value to a PWM range (0-255)
  int speed = map(potValue, 0, 1023, 0, 255);
  
  // Set motor speed
  analogWrite(ENA, speed);
  analogWrite(ENB, speed);
  
  // Set motor direction (both forward)
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  
  // Print potentiometer and speed values for debugging
  Serial.print("Potentiometer Value: ");
  Serial.print(potValue);
  Serial.print(" | Motor Speed: ");
  Serial.println(speed);
  
  // Small delay to stabilize readings
  delay(1000);
}