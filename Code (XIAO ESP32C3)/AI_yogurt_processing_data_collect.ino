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


// Include the required libraries:
#include <FS.h>
#include <SPI.h>
#include <SD.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SensirionI2CSht4x.h>
#include <DFRobot_HX711_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Initialize the File class:
File myFile;
// Define the CSV file name: 
const char* data_file = "/yogurt_data.csv";

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
static const unsigned char PROGMEM sd [] = {
0x0F, 0xFF, 0xFF, 0xFE, 0x1F, 0xFF, 0xFF, 0xFF, 0x1F, 0xFE, 0x7C, 0xFF, 0x1B, 0x36, 0x6C, 0x9B,
0x19, 0x26, 0x4C, 0x93, 0x19, 0x26, 0x4C, 0x93, 0x19, 0x26, 0x4C, 0x93, 0x19, 0x26, 0x4C, 0x93,
0x19, 0x26, 0x4C, 0x93, 0x19, 0x26, 0x4C, 0x93, 0x19, 0x26, 0x4C, 0x93, 0x1F, 0xFF, 0xFF, 0xFF,
0x1F, 0xFF, 0xFF, 0xFF, 0x1F, 0xFF, 0xFF, 0xFF, 0x1F, 0xFF, 0xFF, 0xFF, 0x1F, 0xFF, 0xFF, 0xFF,
0x3F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 0xC7, 0xFF, 0xFF, 0xF9, 0x41, 0xFF, 0x1F, 0xF9, 0xDD, 0xFF,
0x1F, 0xFC, 0xDD, 0xFF, 0x1F, 0xFE, 0x5D, 0xFF, 0x1F, 0xF8, 0x43, 0xFF, 0x1F, 0xFD, 0xFF, 0xFF,
0x3F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE
};

// Define the integrated button pin on the XIAO Expansion board.
#define button D1
// Define the button state and the duration to utilize the integrated button in two different modes: long press and short press.
int button_state = 0;
#define DURATION 5000

// Define the data holders:
float temperature, m_temperature, humidity, pressure, weight;
int class_number = 0;
long timer;
uint16_t error;
char errorMessage[256];
 
void setup(){
  Serial.begin(115200);

  pinMode(button, INPUT_PULLUP);
  
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
  
  // Check the connection status between XIAO ESP32C3 and the SD card.
  if(!SD.begin()){
    Serial.println("Error: SD card initialization failed!\n");
    err_msg();
    while (1);
  }
  Serial.println("SD card is detected successfully!\n");

  delay(5000);    
 
}
 
void loop(){
  get_temperature_and_humidity();
  get_pressure();
  get_weight(15);
  get_milk_temperature();
  
  // Show the collected data on the screen.
  home_screen();

  // Detect the long press and short press button modes:
  button_state = 0;
  if(!digitalRead(button)){
    timer = millis();
    button_state = 1;
    while((millis()-timer) <= DURATION){
      if(digitalRead(button)){
        button_state = 2;
        break;
      }
    }
  }
  
  if(button_state == 1){
    // Save the given data record to the given CSV file on the SD card when long-pressed.
    save_data_to_SD_Card(SD, class_number);
  }else if(button_state == 2){
    // Change the class number when short-pressed.
    class_number++;
    if(class_number > 2) class_number = 0;
    Serial.println("\n\nSelected Class: " + String(class_number) + "\n");
  }

}

void save_data_to_SD_Card(fs::FS &fs, int consistency_level){
  // Open the given CSV file on the SD card in the APPEND file mode.
  // FILE MODES: WRITE, READ, APPEND
  myFile = fs.open(data_file, FILE_APPEND);
  delay(1000);
  // If the given file is opened successfully:
  if(myFile){
    Serial.print("\n\nWriting to "); Serial.print(data_file); Serial.println("...");
    // Create the data record to be inserted as a new row: 
    String data_record = String(temperature) + "," + String(humidity) + "," + String(pressure) + "," + String(m_temperature) + "," + String(weight) + ',' + String(consistency_level);
    // Append the data record:
    myFile.println(data_record);
    // Close the CSV file:
    myFile.close();
    Serial.println("Data saved successfully!\n");
    // Notify the user after appending the given data record successfully.
    display.clearDisplay(); 
    display.drawBitmap(48, 0, sd, 32, 44, SSD1306_WHITE);
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);  
    display.setCursor(0,48); 
    display.println("Data saved to the SD card!");
    display.display();  
  }else{
    // If XIAO ESP32C3 cannot open the given CSV file successfully:
    Serial.println("\nXIAO ESP32C3 cannot open the given CSV file successfully!\n");
    err_msg();
  }
  // Exit and clear:
  delay(4000);
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
  display.println();
  display.println("Selected Class => " + String(class_number));
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
