         /////////////////////////////////////////////  
        //    IoT AI-driven Yogurt Processing      //
       //      & Texture Prediction w/ Blynk      //
      //             ---------------             //
     //             (XIAO ESP32C3)              //           
    //             by Kutluhan Aktar           // 
   //                                         //
  /////////////////////////////////////////////

//
// Collect environmental factors and the culture amount while producing yogurt. Then, run a neural network model via Blynk to predict its texture.
//
// For more information:
// https://www.theamplituhedron.com/projects/IoT_AI_driven_Yogurt_Processing_Texture_Prediction
//
//
// Connections
// XIAO ESP32C3 :  
//                                Grove - Temperature & Humidity Sensor
// A4   --------------------------- SDA
// A5   --------------------------- SCL
//                                Grove - Integrated Pressure Sensor
// A0   --------------------------- S
//                                Gravity: I2C 1Kg Weight Sensor Kit - HX711
// A4   --------------------------- SDA
// A5   --------------------------- SCL
//                                DS18B20 Waterproof Temperature Sensor
// D6   --------------------------- Data
//                                SSD1306 OLED Display (128x64)
// A4   --------------------------- SDA
// A5   --------------------------- SCL
//                                MicroSD Card Module (Built-in on the XIAO Expansion board)
// D10  --------------------------- MOSI
// D9   --------------------------- MISO
// D8   --------------------------- CLK (SCK)
// D2   --------------------------- CS (SS)  
//                                Button (Built-in on the XIAO Expansion board)
// D1   --------------------------- +


// Define the Template ID, Device Name, and Auth Token parameters provided by the Blynk.Cloud.
#define BLYNK_TEMPLATE_ID "<_TEMPLATE_ID_>"
#define BLYNK_DEVICE_NAME "<_DEVICE_NAME_>"
#define BLYNK_AUTH_TOKEN "<_AUTH_TOKEN_>"

// Uncomment the line below to activate debugging for the Blynk application.
//#define BLYNK_PRINT Serial

// Include the required libraries:
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SensirionI2CSht4x.h>
#include <DFRobot_HX711_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>

char ssid[] = "<_SSID_>";    // your network SSID (name)
char pass[] = "<_PASS_>";    // your network password (use for WPA, or use as key for WEP)

// Define the required variables for the Blynk application and the virtual pins connected to the dashboard widgets.
char auth[] = BLYNK_AUTH_TOKEN;
#define TEMP_WIDGET     V4
#define HUMD_WIDGET     V12
#define PRES_WIDGET     V6
#define M_TEMP_WIDGET   V7
#define WEIGHT_WIDGET   V8
#define BUTTON_WIDGET   V9
#define LABEL_WIDGET    V10

// Include the Edge Impulse model converted to an Arduino library:
#include <IoT_AI-driven_Yogurt_Processing_inferencing.h>

// Define the required parameters to run an inference with the Edge Impulse model.
#define FREQUENCY_HZ        EI_CLASSIFIER_FREQUENCY
#define INTERVAL_MS         (1000 / (FREQUENCY_HZ + 1))

// Define the features array to classify one frame of data.
float features[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE];
size_t feature_ix = 0;

// Define the threshold value for the model outputs (predictions).
float threshold = 0.60;

// Define the yogurt consistency level (class) names:
String classes[] = {"Thinner", "Optimum", "Curdling"};

// Define the 0.96 OLED display (SSD1306) on the XIAO Expansion board. 
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET    -1 // Reset pin # (or -1 if sharing Arduino reset pin)

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Define the Grove - Temperature & Humidity Sensor object.
SensirionI2CSht4x sht4x;

// Define the HX711 weight sensor.
DFRobot_HX711_I2C MyScale;

// Define the DS18B20 waterproof temperature sensor settings:
#define ONE_WIRE_BUS D6
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);

// Define minimum and maximum pressure thresholds for the Grove - Integrated Pressure Sensor depending on the initial sensor measurements.
#define pressure_s_pin A0
int rawValue;
int offset = 221; // minimum (adjust)
int fullScale = 8360; // maximum (adjust)
 
// Define monochrome graphics:
static const unsigned char PROGMEM _error [] = {
0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, 0xFC, 0x00, 0x00, 0xE0, 0x07, 0x00, 0x01, 0x80, 0x01, 0x80,
0x06, 0x00, 0x00, 0x60, 0x0C, 0x00, 0x00, 0x30, 0x08, 0x01, 0x80, 0x10, 0x10, 0x03, 0xC0, 0x08,
0x30, 0x02, 0x40, 0x0C, 0x20, 0x02, 0x40, 0x04, 0x60, 0x02, 0x40, 0x06, 0x40, 0x02, 0x40, 0x02,
0x40, 0x02, 0x40, 0x02, 0x40, 0x02, 0x40, 0x02, 0x40, 0x02, 0x40, 0x02, 0x40, 0x02, 0x40, 0x02,
0x40, 0x02, 0x40, 0x02, 0x40, 0x02, 0x40, 0x02, 0x40, 0x03, 0xC0, 0x02, 0x40, 0x01, 0x80, 0x02,
0x40, 0x00, 0x00, 0x02, 0x60, 0x00, 0x00, 0x06, 0x20, 0x01, 0x80, 0x04, 0x30, 0x03, 0xC0, 0x0C,
0x10, 0x03, 0xC0, 0x08, 0x08, 0x01, 0x80, 0x10, 0x0C, 0x00, 0x00, 0x30, 0x06, 0x00, 0x00, 0x60,
0x01, 0x80, 0x01, 0x80, 0x00, 0xE0, 0x07, 0x00, 0x00, 0x3F, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00
};
static const unsigned char PROGMEM thinner [] = {
0x00, 0x00, 0x00, 0x00, 0x0F, 0xFE, 0x00, 0x00, 0x0F, 0xFF, 0x00, 0x00, 0x07, 0xFE, 0x00, 0x00,
0x08, 0x03, 0x00, 0x00, 0x18, 0x01, 0x80, 0x00, 0x30, 0x00, 0x80, 0x00, 0x20, 0x00, 0xC0, 0x00,
0x60, 0x00, 0x60, 0x00, 0x40, 0x3F, 0xE0, 0x00, 0x79, 0xFF, 0xE0, 0x00, 0x7F, 0xFF, 0xE0, 0x00,
0x7F, 0xFF, 0xE0, 0x00, 0x7F, 0xFF, 0xE0, 0x00, 0x7F, 0xFF, 0xE0, 0x00, 0x7F, 0xFF, 0xE0, 0x00,
0x7F, 0xFF, 0xE0, 0x00, 0x7F, 0xFF, 0xE0, 0x00, 0x7F, 0xFF, 0xE0, 0x00, 0x7F, 0xFF, 0xE0, 0x00,
0x7F, 0xFF, 0xEF, 0xFE, 0x7F, 0xFF, 0xE8, 0x02, 0x7F, 0xFF, 0xE8, 0x02, 0x7F, 0xFF, 0xE8, 0x12,
0x7F, 0xFF, 0xEF, 0xFE, 0x7F, 0xFF, 0xEF, 0xFE, 0x7F, 0xFF, 0xEF, 0xFE, 0x7F, 0xFF, 0xEF, 0xFE,
0x7F, 0xFF, 0xE7, 0xFC, 0x7F, 0xFF, 0xE7, 0xFC, 0x7F, 0xFF, 0xE7, 0xFC, 0x3F, 0xFF, 0xC7, 0xFC
};
static const unsigned char PROGMEM optimum [] = {
0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xE0, 0x0F, 0x80, 0x00, 0x3C, 0x38, 0x00, 0x70, 0x06,
0x61, 0xEC, 0xFF, 0xE2, 0xC7, 0x13, 0x98, 0x3B, 0xD8, 0x00, 0x0F, 0x06, 0x70, 0x1E, 0x3B, 0x9E,
0x7F, 0x80, 0x01, 0xFE, 0x2F, 0xFF, 0xFF, 0xFC, 0x21, 0xFF, 0xFF, 0xC4, 0x20, 0x0F, 0xFC, 0x04,
0x20, 0x18, 0x03, 0x04, 0x10, 0x60, 0x01, 0x84, 0x10, 0xC0, 0x00, 0xC4, 0x11, 0x00, 0x00, 0x44,
0x1E, 0x00, 0x00, 0x68, 0x13, 0x20, 0x01, 0xF8, 0x1F, 0x79, 0x0E, 0xA8, 0x0C, 0xFB, 0xDA, 0x98,
0x0C, 0xA6, 0x56, 0x98, 0x0C, 0xAF, 0xD6, 0x58, 0x0A, 0xAE, 0xD6, 0x78, 0x0A, 0x93, 0xF0, 0x30,
0x0E, 0x00, 0x00, 0x30, 0x0D, 0x00, 0x00, 0x70, 0x04, 0x80, 0x00, 0xD0, 0x04, 0x60, 0x01, 0x90,
0x02, 0x38, 0x06, 0x30, 0x01, 0xCF, 0xF9, 0xC0, 0x00, 0x7F, 0xFF, 0x00, 0x00, 0x00, 0xC0, 0x00
};
static const unsigned char PROGMEM curdling [] = {
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x7F, 0x00,
0x00, 0x01, 0xE3, 0x00, 0x00, 0x03, 0xC3, 0x80, 0x00, 0x0F, 0xC7, 0xC0, 0x00, 0x0F, 0xFF, 0xF8,
0x00, 0x0F, 0xFF, 0xFC, 0x00, 0x1F, 0xFF, 0xFC, 0x00, 0x3F, 0xFF, 0xFC, 0x07, 0xFF, 0x0F, 0xFC,
0x1F, 0xFE, 0x0F, 0xFE, 0x3F, 0xFC, 0x07, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x7F, 0xFE, 0x0F, 0xFE,
0x7F, 0xFE, 0x1F, 0xFE, 0x7F, 0xFF, 0x3F, 0xFE, 0x7F, 0xFF, 0xFF, 0x3E, 0x7F, 0x1F, 0xFF, 0x3E,
0x7F, 0x0F, 0xFF, 0x3E, 0x1F, 0x0F, 0xFF, 0xFE, 0x1F, 0x0F, 0xFF, 0xFE, 0x0F, 0x0F, 0xFF, 0xFE,
0x0F, 0x9F, 0xDF, 0xFE, 0x0F, 0xFF, 0x8F, 0xFE, 0x1F, 0xFF, 0x8F, 0xFE, 0x3F, 0xFF, 0xCF, 0xFE,
0x7F, 0xFF, 0xFF, 0xFE, 0x7F, 0xFF, 0xFF, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// Create an array including icons for labels (classes).
static const unsigned char PROGMEM *class_icons[] = {thinner, optimum, curdling};

// Define the data holders:
float temperature, m_temperature, humidity, pressure, weight;
long timer;
int predicted_class = -1;
volatile boolean model_running = false;
uint16_t error;
char errorMessage[256];

void setup(){
  Serial.begin(115200);

  // Create the Blynk object.
  Blynk.begin(auth, ssid, pass);
  
  // Initialize the SSD1306 screen:
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.display();
  delay(1000);

  display.clearDisplay();      
  display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
  display.setCursor(0,0);
  display.setTextSize(1);
  display.println("\nIoT AI-driven\n");
  display.setTextSize(3);
  display.println("Yogurt");
  display.setTextSize(1);
  display.println("\nProcessing");
  display.display();

  // Initialize the DS18B20 sensor.
  DS18B20.begin();

  // Define the required settings to initialize the Grove - Temperature & Humidity Sensor.
  sht4x.begin(Wire);
  uint32_t serialNumber;
  error = sht4x.serialNumber(serialNumber);

  // Check the Grove - Temperature & Humidity Sensor connection status and print the error message on the serial monitor, if any.
  if(error){
    Serial.print("Error: Grove - Temperature & Humidity Sensor not initialized!\n");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
    err_msg();
  }else{
    Serial.print("Grove - Temperature & Humidity Sensor successfully initialized: "); Serial.println(serialNumber);
  }

  // Check the connection status between the weight (HX711) sensor and XIAO ESP32C3.
  while (!MyScale.begin()) {
    Serial.println("Error: HX711 initialization is failed!");
    err_msg();
    delay(1000);
  }
  Serial.println("HX711 initialization is successful!");
  
  // Set the calibration weight (g) to calibrate the weight sensor automatically.
  MyScale.setCalWeight(100);
  // Set the calibration threshold (g).
  MyScale.setThreshold(30);
  // Display the current calibration value. 
  Serial.print("\nCalibration Value: "); Serial.println(MyScale.getCalibration());
  MyScale.setCalibration(MyScale.getCalibration());
  delay(1000);
}
 
void loop(){
  // Initiate the communication between the Blynk dashboard and XIAO ESP32C3.
  Blynk.run();
  
  get_temperature_and_humidity();
  get_pressure();
  get_weight(15);
  get_milk_temperature();
  
  // Show the collected data on the screen.
  home_screen();

  // If the switch widget on the Blynk dashboard is activated, run the Edge Impulse model to make predictions on the yogurt consistency levels (classes).
  if(model_running){ run_inference_to_make_predictions(1); model_running = false; }

  // If the Edge Impulse model predicts a yogurt consistency level (class) successfully:
  if(predicted_class != -1){
      // Transfer the predicted label (class) to the Blynk application (dashboard).
      Blynk.virtualWrite(LABEL_WIDGET, classes[predicted_class]);
      // Print the predicted label (class) on the built-in screen.
      display.clearDisplay();
      display.drawBitmap(48, 0, class_icons[predicted_class], 32, 32, SSD1306_WHITE);
      display.setTextSize(1); 
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(0,40);
      display.println("Transferred to Blynk");
      String c = "Class: " + classes[predicted_class];
      int str_x = c.length() * 6;
      display.setCursor((SCREEN_WIDTH - str_x) / 2, 56);
      display.println(c);
      display.display();
      // Clear the predicted label (class).
      predicted_class = -1;
      delay(1000);
  }

  // Every 30 seconds, transfer the collected environmental factors and culture amount to the Blynk application so as to update the assigned widgets on the Blynk dashboard.
  if(millis() - timer >= 30*1000){ update_Blynk_parameters(); Serial.println("\n\nBlynk Dashboard: Data Transferred Successfully!\n"); timer = millis(); }
}

void run_inference_to_make_predictions(int multiply){
  // Scale (normalize) data items depending on the given model:
  float scaled_temperature = temperature / 100;
  float scaled_humidity = humidity / 100;
  float scaled_pressure = pressure / 1000;
  float scaled_milk_temperature = m_temperature / 100;
  float scaled_starter_weight = weight / 10;

  // Copy the scaled data items to the features buffer.
  // If required, multiply the scaled data items while copying them to the features buffer.
  for(int i=0; i<multiply; i++){  
    features[feature_ix++] = scaled_temperature;
    features[feature_ix++] = scaled_humidity;
    features[feature_ix++] = scaled_pressure;
    features[feature_ix++] = scaled_milk_temperature;
    features[feature_ix++] = scaled_starter_weight;
  }

  // Display the progress of copying data to the features buffer.
  Serial.print("Features Buffer Progress: "); Serial.print(feature_ix); Serial.print(" / "); Serial.println(EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE);
  
  // Run inference:
  if(feature_ix == EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE){    
    ei_impulse_result_t result;
    // Create a signal object from the features buffer (frame).
    signal_t signal;
    numpy::signal_from_buffer(features, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);
    // Run the classifier:
    EI_IMPULSE_ERROR res = run_classifier(&signal, &result, false);
    ei_printf("\nrun_classifier returned: %d\n", res);
    if(res != 0) return;

    // Print the inference timings on the serial monitor.
    ei_printf("Predictions (DSP: %d ms., Classification: %d ms., Anomaly: %d ms.): \n", 
        result.timing.dsp, result.timing.classification, result.timing.anomaly);

    // Obtain the prediction results for each label (class).
    for(size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++){
      // Print the prediction results on the serial monitor.
      ei_printf("%s:\t%.5f\n", result.classification[ix].label, result.classification[ix].value);
      // Get the predicted label (class).
      if(result.classification[ix].value >= threshold) predicted_class = ix;
    }
    Serial.print("\nPredicted Class: "); Serial.println(predicted_class);

    // Detect anomalies, if any:
    #if EI_CLASSIFIER_HAS_ANOMALY == 1
      ei_printf("Anomaly : \t%.3f\n", result.anomaly);
    #endif

    // Clear the features buffer (frame):
    feature_ix = 0;
  }
}

void update_Blynk_parameters(){
  // Transfer the collected yogurt processing information to the Blynk dashboard.
  Blynk.virtualWrite(TEMP_WIDGET, temperature);
  Blynk.virtualWrite(HUMD_WIDGET, humidity);
  Blynk.virtualWrite(PRES_WIDGET, pressure);
  Blynk.virtualWrite(M_TEMP_WIDGET, m_temperature);
  Blynk.virtualWrite(WEIGHT_WIDGET, weight);
}

// Obtain the incoming value from the switch (button) widget on the Blynk dashboard.
BLYNK_WRITE(BUTTON_WIDGET){
  int buttonValue = param.asInt();
  if(buttonValue){ model_running = true; }
  else{ Blynk.virtualWrite(LABEL_WIDGET, "Waiting..."); }
}

void get_temperature_and_humidity(){
  // Obtain the measurements generated by the Grove - Temperature & Humidity Sensor.
  error = sht4x.measureHighPrecision(temperature, humidity);
  if(error){
    Serial.print("Error trying to execute measureHighPrecision(): ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
  }else{
    Serial.print("\nTemperature : "); Serial.print(temperature); Serial.println("°C");
    Serial.print("Humidity : "); Serial.print(humidity); Serial.println("%");
  }
  delay(500);
}

void get_pressure(){
  // Obtain the measurements generated by the Grove - Integrated Pressure Sensor.
  rawValue = 0;
  // Convert the accumulation of raw data to the pressure estimation.
  for (int x = 0; x < 10; x++) rawValue = rawValue + analogRead(pressure_s_pin);
  pressure = (rawValue - offset) * 700.0 / (fullScale - offset);
  Serial.print("\nPressure : "); Serial.print(pressure); Serial.println(" kPa");
}

void get_weight(int calibration){
  weight = MyScale.readWeight();
  weight = weight - calibration;
  if(weight < 0.5) weight = 0;
  Serial.print("\nWeight: "); Serial.print(weight); Serial.println(" g");
  delay(500);
}

void get_milk_temperature(){
  // Obtain the temperature measurement generated by the DS18B20 Waterproof Temperature Sensor.
  DS18B20.requestTemperatures(); 
  m_temperature = DS18B20.getTempCByIndex(0);
  Serial.print("\nMilk Temperature: "); Serial.print(m_temperature); Serial.println("°C");
}

void home_screen(){
  display.clearDisplay();   
  display.setTextSize(1); 
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("Temp => " + String(temperature) + " *C");
  display.println("Humidity => " + String(humidity) + " %");
  display.println("Pres. => " + String(pressure) + " kPa");
  display.println();
  display.println("M_Temp => " + String(m_temperature) + " *C");
  display.println("Weight => " + String(weight) + " g");
  display.display();  
}

void err_msg(){
  // Show the error message on the SSD1306 screen.
  display.clearDisplay();   
  display.drawBitmap(48, 0, _error, 32, 32, SSD1306_WHITE);
  display.setTextSize(1); 
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,40); 
  display.println("Check the serial monitor to see the error!");
  display.display();  
}
