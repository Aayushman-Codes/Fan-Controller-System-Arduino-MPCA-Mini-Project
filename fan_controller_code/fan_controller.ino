#include <DHT.h>

// Pins
#define LM35_PIN A0
#define LDR_PIN A1
#define DHT11_PIN 2
#define DHT22_PIN 3
#define LED_PIN 5
#define FAN_PWM_PIN 9

// Sensor Configuration
#define DHT11_TYPE DHT11
#define DHT22_TYPE DHT22
DHT dht11(DHT11_PIN, DHT11_TYPE);
DHT dht22(DHT22_PIN, DHT22_TYPE);

// Thresholds
const float TEMP_MIN = 33.75; 
const float TEMP_MAX = 45.0; 
const int LDR_BRIGHT_THRESHOLD = 700; 

void setup() {
  Serial.begin(9600);

  analogReference(DEFAULT); 
  
  dht11.begin();
  dht22.begin();

  pinMode(LM35_PIN, INPUT);
  pinMode(LDR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(FAN_PWM_PIN, OUTPUT);

  analogWrite(FAN_PWM_PIN, 0);
  digitalWrite(LED_PIN, LOW);
  
  Serial.println(F("--- System Booted: Detecting Sensor Readings ---"));
}

void loop() {
  long totalLM35 = 0;
  for(int i = 0; i < 15; i++) {
    totalLM35 += analogRead(LM35_PIN);
    delay(2);
  }
  float avgLM35 = totalLM35 / 15.0;
  
  // Math for 5.0V: (Avg * 5.0 * 100) / 1024
  float tempLM35 = (avgLM35 * 5.0 * 100.0) / 1024.0;
  tempLM35 = tempLM35 - 7; 
  
  // --- 2. Read Digital DHTs ---
  float tempDHT11 = dht11.readTemperature() - 1; 
  float tempDHT22 = dht22.readTemperature();

  // --- 3. Sensor Validation ---
  if (isnan(tempDHT11) || tempDHT11 > 100 || tempDHT11 < 0) tempDHT11 = tempLM35; 
  if (isnan(tempDHT22) || tempDHT22 > 100 || tempDHT22 < 0) tempDHT22 = tempLM35;

  float maxTemp = max(tempLM35, max(tempDHT11, tempDHT22));

  // --- 4. Read LDR (Trigger Logic) ---
  int lightLevel = analogRead(LDR_PIN);
  bool isDark = (lightLevel > LDR_BRIGHT_THRESHOLD);

  // --- 5. Fan Control Logic (OR Logic) ---
  int pwmValue = 0;

  // Check if EITHER Temp is high OR LDR is cupped/bright
  if (maxTemp >= TEMP_MIN || isDark) {
    
    if (maxTemp >= TEMP_MIN) {
      pwmValue = map(maxTemp, TEMP_MIN, TEMP_MAX, 150, 255);
    } else {
      pwmValue = 200; 
    }
    
    // Ensure speed stays within hardware limits
    pwmValue = constrain(pwmValue, 150, 255);
    
    // Visual indicator that system is active
    digitalWrite(LED_PIN, HIGH);
    
  } else {
    // Neither condition met: Turn everything OFF
    pwmValue = 0;
    digitalWrite(LED_PIN, LOW);
  }

  analogWrite(FAN_PWM_PIN, pwmValue);

  int displayLightLevel = 1023 - lightLevel;

  // --- 6. Monitoring Output ---
  Serial.print("LM35: "); Serial.print(tempLM35);
  Serial.print(" | DHT11: "); Serial.print(tempDHT11);
  Serial.print(" | DHT22: "); Serial.print(tempDHT22);
  Serial.print(" | MAX: "); Serial.print(maxTemp);
  Serial.print("C | LDR: "); Serial.print(displayLightLevel);
  
  if (isDark) Serial.print(" [DARK|FAN ON]");
  else Serial.print(" [LIGHT|FAN OFF]");

  Serial.print(" | Fan PWM: "); Serial.println(pwmValue);

  delay(1000); 
}