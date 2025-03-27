         /////////////////////////////////////////////  
        //     AI-assisted Air Quality Monitor     //
       //          w/ IoT Surveillance            //
      //             ---------------             //
     //            (FireBeetle ESP32)           //           
    //             by Kutluhan Aktar           // 
   //                                         //
  /////////////////////////////////////////////

//
// Log NO2, O3, and weather data, train a NN model to detect air pollution, and display real-time results w/ surveillance footage on a PHP web app.
//
// For more information:
// https://www.theamplituhedron.com/projects/AI_assisted_Air_Quality_Monitor_w_IoT_Surveillance
//
//
// Connections
// FireBeetle ESP32 :  
//                                Arduino Mega
// D4   --------------------------- D18 (RX1)
// D2   --------------------------- D19 (TX1)


// Include the required libraries:
#include <Arduino.h>
#include <WiFi.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "esp_camera.h"
#include "FS.h"
#include "SD_MMC.h"

// Include the Edge Impulse model converted to an Arduino library:
#include <AI-assisted_Air_Quality_Monitor_inferencing.h>

// Define the required parameters to run an inference with the Edge Impulse model.
#define FREQUENCY_HZ        EI_CLASSIFIER_FREQUENCY
#define INTERVAL_MS         (1000 / (FREQUENCY_HZ + 1))

// Define the features array to classify one frame of data.
float features[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE];
size_t feature_ix = 0;

// Define the threshold value for the model outputs (predictions).
float threshold = 0.60;

// Define the air quality level (class) names:
String classes[] = {"Clean", "Risky", "Unhealthy"};

char ssid[] = "<_SSID_>";        // your network SSID (name)
char pass[] = "<_PASSWORD_>";    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;                // your network key Index number (needed only for WEP)

// Define the server on LattePanda 3 Delta 864.
char server[] = "192.168.1.22";
// Define the web application path.
String application = "/weather_station_data_center/update_data.php";

// Initialize the WiFiClient object.
WiFiClient client; /* WiFiSSLClient client; */

// FireBeetle Covers - Camera & Audio Media Board
// https://wiki.dfrobot.com/FireBeetle_Covers-Camera%26Audio_Media_Board_SKU_DFR0498
// Pinout:
#define PWDN_GPIO_NUM     -1
#define RESET_GPIO_NUM    0
#define XCLK_GPIO_NUM     21
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       19
#define Y4_GPIO_NUM       18
#define Y3_GPIO_NUM       5
#define Y2_GPIO_NUM       17
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// Define the camera (image) buffer array.
camera_fb_t * fb = NULL;

// Define the built-in button on the media board.
#define button  16

// Create a struct (data) including all air quality data parameters:
struct data {
  float temperature;
  float humidity;
  float no2;
  int ozone;
  int wind_speed;
};

// Define the data holders:
struct data air_quality;
int predicted_class = -1;
#define RXD  4
#define TXD  2
String data_packet = "";
String _header = "no2,ozone,temperature,humidity,wind_speed\n";
int del_1, del_2, del_3, del_4, del_5;
int c_s = 0, r_s = 0, u_s = 0;    
unsigned long model_timer = 0;

void setup(){
  // Disable the brownout detector.
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  
  Serial.begin(115200);

  pinMode(button, INPUT_PULLUP);

  // Initialize the hardware serial port (2) to communicate with Arduino Mega.
  Serial2.begin(115200, SERIAL_8N1, RXD, TXD); // (BaudRate, SerialMode, RX_pin, TX_pin)

  // Initiate the built-in SD card module on the media board.
  if(!SD_MMC.begin()){
    Serial.println("SD Card not detected!\n");
    return;
  }
  Serial.println("SD Card detected successfully!\n");

  // Define the OV7725 camera pin configuration settings.
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  
  // Define the pixel format and the frame size settings.
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_GRAYSCALE;
  config.frame_size = FRAMESIZE_QVGA; // FRAMESIZE_96X96, FRAMESIZE_240X240
  config.jpeg_quality = 20; // 0-63 lower number means higher quality
  config.fb_count = 1;

  // No PSRAM
  config.fb_location = CAMERA_FB_IN_DRAM;

  // Initiate the OV7725 camera on the media board.
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    delay(1000);
    ESP.restart();
  }
  Serial.println("Camera initialized successfully!\n");
                   
  // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  // Attempt to connect to the Wi-Fi network:
  while(WiFi.status() != WL_CONNECTED){
    // Wait for the connection:
    delay(500);
    Serial.print(".");
  }
  // If connected to the network successfully:
  Serial.println("Connected to the Wi-Fi network successfully!");

  // Update the model timer.
  model_timer = millis();
}

void loop(){
  // Obtain the data packet and commands transferred by Arduino Mega via serial communication.
  if(Serial2.available() > 0){
    data_packet = Serial2.readString();
  }

  if(data_packet != ""){
    if(data_packet.startsWith("Save")){
      // Glean information as substrings from the transferred data packet by Arduino Mega.
      del_1 = data_packet.indexOf("&");
      del_2 = data_packet.indexOf("&", del_1 + 1);
      String data_record = data_packet.substring(del_1 + 1, del_2);
      String level = data_packet.substring(del_2 + 1);
      // Increment the sample number of the given level (class) by 1.
      int i;
      if(level == "Clean") { c_s+=1; i=c_s;}
      if(level == "Risky") { r_s+=1; i=r_s;}
      if(level == "Unhealthy") { u_s+=1; i=u_s;}
      // Save the transferred data record as a sample (CSV file) depending on the given air quality level.
      String file_name = "/samples/" + level + ".training.sample_" + String(i) + ".csv";
      String line = _header + data_record;
      save_data_to_CSV(file_name.c_str(), line.c_str(), file_name);
    }
    if(data_packet.startsWith("Data")){
      // Glean information as substrings from the transferred data packet by Arduino Mega.
      del_1 = data_packet.indexOf(",");
      del_2 = data_packet.indexOf(",", del_1 + 1);
      del_3 = data_packet.indexOf(",", del_2 + 1);
      del_4 = data_packet.indexOf(",", del_3 + 1);
      del_5 = data_packet.indexOf(",", del_4 + 1);
      // Convert and store the received data items.
      air_quality.no2 = data_packet.substring(del_1 + 1, del_2).toFloat();
      air_quality.ozone = data_packet.substring(del_2 + 1, del_3).toInt();
      air_quality.temperature = data_packet.substring(del_3 + 1, del_4).toFloat();
      air_quality.humidity = data_packet.substring(del_4 + 1, del_5).toFloat();
      air_quality.wind_speed = data_packet.substring(del_5 + 1).toInt();
      Serial.println("\nData parameters obtained and saved successfully!\n");
    }
    // Clear the incoming data packet.
    delay(1000);
    data_packet = "";
  }

  // Every 5 minutes, run the Edge Impulse model to make predictions on the air quality levels (classes).
  // If manual testing is required, FireBeetle ESP32 can also run an inference when the built-in button is pressed.
  if((millis() - model_timer > 300000) || !digitalRead(button)){
    // Run inference:
    run_inference_to_make_predictions(1);
    // If the Edge Impulse model predicts an air quality level (class) successfully:
    if(predicted_class != -1){
      // Create the request string.
      String request = "?no2=" + String(air_quality.no2)
                     + "&o3=" + String(air_quality.ozone)
                     + "&temperature=" + String(air_quality.temperature)
                     + "&humidity=" + String(air_quality.humidity)
                     + "&wind_speed=" + String(air_quality.wind_speed)
                     + "&model_result=" + classes[predicted_class];
      // Capture a picture with the OV7725 camera.               
      take_picture(true);
      // Send the obtained data parameters, the recently captured image, and the model detection result to the web application via an HTTP POST request.
      make_a_post_request(request);
        
      // Clear the predicted label (class).
      predicted_class = -1; 
      // Update the model timer.
      model_timer = millis();
    }
  }
}

void run_inference_to_make_predictions(int multiply){
  // Scale (normalize) data items depending on the given model:
  float scaled_no2 = air_quality.no2;
  float scaled_ozone = float(air_quality.ozone);
  float scaled_temperature = air_quality.temperature;
  float scaled_humidity = air_quality.humidity;
  float scaled_wind_speed = float(air_quality.wind_speed);

  // Copy the scaled data items to the features buffer.
  // If required, multiply the scaled data items while copying them to the features buffer.
  for(int i=0; i<multiply; i++){  
    features[feature_ix++] = scaled_no2;
    features[feature_ix++] = scaled_ozone;
    features[feature_ix++] = scaled_temperature;
    features[feature_ix++] = scaled_humidity;
    features[feature_ix++] = scaled_wind_speed;
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

void take_picture(bool _abort){
  // Release the image buffer if the board throws memory allocation errors.
  if(_abort) esp_camera_fb_return(fb);
  // Capture a picture with the OV7725 camera.
  fb = esp_camera_fb_get();
  // If successful:
  if(!fb) {
    Serial.println("\nImage capture failed!");
    delay(1000);
    ESP.restart();
  }
  Serial.print("\nImage captured successfully: "); Serial.println(fb->len);
  delay(500);
}

void make_a_post_request(String request){
  // Connect to the web application named weather_station_data_center. Change '80' with '443' if you are using SSL connection.
  if (client.connect(server, 80)){
    // If successful:
    Serial.println("\nConnected to the web application successfully!\n");
    // Create the query string:
    String query = application + request;
    // Make an HTTP POST request:
    String head = "--EnvNotification\r\nContent-Disposition: form-data; name=\"captured_image\"; filename=\"new_image.txt\"\r\nContent-Type: text/plain\r\n\r\n";
    String tail = "\r\n--EnvNotification--\r\n";
    // Get the total message length.
    uint32_t totalLen = head.length() + fb->len + tail.length();
    // Start the request:
    client.println("POST " + query + " HTTP/1.1");
    client.println("Host: 192.168.1.22");
    client.println("Content-Length: " + String(totalLen));
    client.println("Connection: Keep-Alive");
    client.println("Content-Type: multipart/form-data; boundary=EnvNotification");
    client.println();
    client.print(head);
    client.write(fb->buf, fb->len);
    client.print(tail);
    // Release the image buffer.
    esp_camera_fb_return(fb);
    delay(2000);
    // If successful:
    Serial.println("HTTP POST => Data transfer completed!\n");
  }else{
    Serial.println("\nConnection failed to the web application!\n");
    delay(2000);
  }
}

void save_data_to_CSV(const char * file_path, const char * _data, String f_name){  
  // Create a CSV file on the SD card with the given file name. 
  File file = SD_MMC.open(file_path, FILE_WRITE);
  if(!file){ Serial.println("SD Card: Failed to open the given CSV file!\n"); return; }
  // Append the header and the given data items to the generated CSV file. 
  if(file.print(_data)){ Serial.println("SD Card => Data appended successfully: " + f_name + "\n"); }
  else{ Serial.println("SD Card: Data append failed!\n"); }
}
