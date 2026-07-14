#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <ESP32Servo.h>

// 1. Tell the code which pins we wired our components to
#define DHTPIN 15          // DHT22 Data pin is on GPIO 15
#define DHTTYPE DHT22      // We are specifically using the DHT22 sensor
#define SERVO_PIN 13       // Servo signal wire is on GPIO 13

// 2. Initialize our components using their libraries
DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2); // 0x27 is the default address for this LCD screen
Servo myServo;

void setup() {
  // This code runs ONCE when the ESP32 turns on
  
  dht.begin();          // Wake up the temperature sensor
  myServo.attach(SERVO_PIN); // Wake up the servo motor
  
  lcd.init();           // Wake up the LCD screen
  lcd.backlight();      // Turn on the screen's background light
  
  // Print a friendly starting message to the screen
  lcd.setCursor(0, 0);  // Position cursor at Column 0, Row 0
  lcd.print("System Starting");
  delay(2000);          // Wait 2 seconds
  lcd.clear();          // Clear the screen
}

void loop() {
  // This code runs on a continuous, infinite loop over and over again
  
  // Read the temperature as Celsius
  float temperature = dht.readTemperature();

  // Check if the sensor failed to read (just in case)
  if (isnan(temperature)) {
    lcd.setCursor(0, 0);
    lcd.print("Sensor Error! ");
    return;
  }

  // ---- VISUALS: Display data on the screen ----
  lcd.setCursor(0, 0);         // Row 0
  lcd.print("Temp: ");
  lcd.print(temperature, 1);   // Print temp with 1 decimal place (e.g., 24.5)
  lcd.print(" C   ");          // Spaces clear out leftover old numbers

  // ---- ENGINEERING LOGIC: Control Loop ----
  // If the incubator gets too hot (above 35°C), open the safety vent!
  if (temperature > 35.0) {
    lcd.setCursor(0, 1);       // Move to Row 1
    lcd.print("VENT: OPEN (HOT)");
    myServo.write(90);         // Rotate servo to 90 degrees (open position)
  } 
  // Otherwise, keep the vent closed
  else {
    lcd.setCursor(0, 1);       // Move to Row 1
    lcd.print("VENT: CLOSED   ");
    myServo.write(0);          // Rotate servo back to 0 degrees (closed position)
  }

  delay(2000); // Wait 2 seconds before checking the temperature again
}