#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <ESP32Servo.h>

// Pin Connections
#define DHTPIN 15          
#define DHTTYPE DHT22      
#define SERVO_PIN 13       
#define BUTTON_PIN 14      
#define BUZZER_PIN 12      

// Initialize hardware objects
DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo myServo;

// Interrupt & Debouncing Variables
volatile bool manualOverrideActive = false; 
unsigned long lastDebounceTime = 0;         
const unsigned long debounceDelay = 250;    // 250ms ignore window to prevent switch bouncing

// Non-blocking Timing Variables (Eliminating delay() so the system never freezes)
unsigned long lastTempUpdate = 0;
const unsigned long tempInterval = 2000;    // Check temperature every 2 seconds

unsigned long lastBuzzerToggle = 0;
const unsigned long buzzerInterval = 500;   // Buzzer beeps every 0.5 seconds when triggered
bool buzzerState = false;

// --- HARDWARE INTERRUPT SERVICE ROUTINE (ISR) ---
// This function triggers instantly the microsecond the button is pressed
void IRAM_ATTR handleButtonInterrupt() {
  unsigned long currentTime = millis();
  
  // Software Debounce: Filter out rapid, accidental hardware electrical bounces
  if (currentTime - lastDebounceTime > debounceDelay) {
    manualOverrideActive = !manualOverrideActive; // Toggle override state
    lastDebounceTime = currentTime;               
  }
}

void setup() {
  dht.begin();
  myServo.attach(SERVO_PIN);
  pinMode(BUZZER_PIN, OUTPUT);
  
  // Set up button with an internal pull-up resistor (defaults to HIGH, goes LOW when pressed)
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  // Tell the ESP32 to monitor GPIO 14 for a falling edge (button press) and jump to the ISR
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), handleButtonInterrupt, FALLING);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("System Starting");
  delay(1500);
  lcd.clear();
}

void loop() {
  unsigned long currentMillis = millis();

  // NON-BLOCKING TEMP READINGS: Update reading variable every 2 seconds without stopping the program
  static float currentTemp = 24.0; 
  if (currentMillis - lastTempUpdate >= tempInterval) {
    lastTempUpdate = currentMillis;
    currentTemp = dht.readTemperature();
  }

  // Handle a failed sensor read gracefully
  if (isnan(currentTemp)) {
    lcd.setCursor(0, 0);
    lcd.print("Sensor Error! ");
    return;
  }

  // --- STATE CONTROL LOGIC ---
  
  // STATE 1: Manual Override is Enabled
  if (manualOverrideActive) {
    lcd.setCursor(0, 0);
    lcd.print("OVERRIDE ACTIVE ");
    lcd.setCursor(0, 1);
    lcd.print("VENT: OPEN (MAN)");
    myServo.write(90);             // Force the physical vent open
    noTone(BUZZER_PIN);            // Keep buzzer off during manual maintenance
  }
  
  // STATE 2: Automatic Critical Temperature Alarm (> 35°C)
  else if (currentTemp > 35.0) {
    lcd.setCursor(0, 0);
    lcd.print("Temp: ");
    lcd.print(currentTemp, 1);
    lcd.print(" C (HOT!) ");
    
    lcd.setCursor(0, 1);
    lcd.print("VENT: OPEN (ALRM)");
    myServo.write(90);             // Autonomously open vent

    // Non-blocking alarm tone: Beep the buzzer on/off every 500ms
    if (currentMillis - lastBuzzerToggle >= buzzerInterval) {
      lastBuzzerToggle = currentMillis;
      buzzerState = !buzzerState;
      if (buzzerState) {
        tone(BUZZER_PIN, 1000);    // Play high-pitch 1000Hz beep
      } else {
        noTone(BUZZER_PIN);        // Silence beep
      }
    }
  } 
  
  // STATE 3: Normal Automatic Operation
  else {
    lcd.setCursor(0, 0);
    lcd.print("Temp: ");
    lcd.print(currentTemp, 1);
    lcd.print(" C      ");
    
    lcd.setCursor(0, 1);
    lcd.print("VENT: CLOSED    ");
    myServo.write(0);              // Keep vent closed
    noTone(BUZZER_PIN);            // Ensure buzzer is quiet
  }
}
