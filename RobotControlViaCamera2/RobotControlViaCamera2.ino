/*********
  I started using Rui Santos's 
  Complete instructions at: https://randomnerdtutorials.com/esp32-cam-car-robot-web-server/
  and his book at https://RandomNerdTutorials.com/esp32-cam-projects-ebook/
  His permission:
  "Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
  "
  By learning his codes and with the helpf of chatGPT I have added numerous functioning to the robot car
  For example, I added endpoints to set and get the servo position, left and right speeds of the robot car
  as well as end point to take and save an image for training with YOLO8

  By Giang T. Dang
  18 Nov 2023, Christchurch, New Zealand 
*********/

/*********
  Rui Santos
  Complete instructions at https://RandomNerdTutorials.com/esp32-cam-projects-ebook/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*********/

#include "esp_camera.h"
#include <WiFi.h>
#include "esp_timer.h"
#include "img_converters.h"
#include "Arduino.h"
#include "fb_gfx.h"
#include "soc/soc.h"             // disable brownout problems
#include "soc/rtc_cntl_reg.h"    // disable brownout problems
#include "esp_http_server.h"
#include "ESP32Servo.h"

typedef struct {
        httpd_req_t *req;
        size_t len;
} jpg_chunking_t;


// Replace with your network credentials
// const char* ssid = "your_ssid";
// const char* password = "your_pass";
const char* ssid = "SPARK-B315-DB3F";
const char* password = "A2DJG680N80";

#define PART_BOUNDARY "123456789000000000000987654321"

#define CAMERA_MODEL_AI_THINKER
//#define CAMERA_MODEL_M5STACK_PSRAM
//#define CAMERA_MODEL_M5STACK_WITHOUT_PSRAM
//#define CAMERA_MODEL_M5STACK_PSRAM_B
//#define CAMERA_MODEL_WROVER_KIT

#if defined(CAMERA_MODEL_WROVER_KIT)
  #define PWDN_GPIO_NUM    -1
  #define RESET_GPIO_NUM   -1
  #define XCLK_GPIO_NUM    21
  #define SIOD_GPIO_NUM    26
  #define SIOC_GPIO_NUM    27
  
  #define Y9_GPIO_NUM      35
  #define Y8_GPIO_NUM      34
  #define Y7_GPIO_NUM      39
  #define Y6_GPIO_NUM      36
  #define Y5_GPIO_NUM      19
  #define Y4_GPIO_NUM      18
  #define Y3_GPIO_NUM       5
  #define Y2_GPIO_NUM       4
  #define VSYNC_GPIO_NUM   25
  #define HREF_GPIO_NUM    23
  #define PCLK_GPIO_NUM    22

#elif defined(CAMERA_MODEL_M5STACK_PSRAM)
  #define PWDN_GPIO_NUM     -1
  #define RESET_GPIO_NUM    15
  #define XCLK_GPIO_NUM     27
  #define SIOD_GPIO_NUM     25
  #define SIOC_GPIO_NUM     23
  
  #define Y9_GPIO_NUM       19
  #define Y8_GPIO_NUM       36
  #define Y7_GPIO_NUM       18
  #define Y6_GPIO_NUM       39
  #define Y5_GPIO_NUM        5
  #define Y4_GPIO_NUM       34
  #define Y3_GPIO_NUM       35
  #define Y2_GPIO_NUM       32
  #define VSYNC_GPIO_NUM    22
  #define HREF_GPIO_NUM     26
  #define PCLK_GPIO_NUM     21

#elif defined(CAMERA_MODEL_M5STACK_WITHOUT_PSRAM)
  #define PWDN_GPIO_NUM     -1
  #define RESET_GPIO_NUM    15
  #define XCLK_GPIO_NUM     27
  #define SIOD_GPIO_NUM     25
  #define SIOC_GPIO_NUM     23
  
  #define Y9_GPIO_NUM       19
  #define Y8_GPIO_NUM       36
  #define Y7_GPIO_NUM       18
  #define Y6_GPIO_NUM       39
  #define Y5_GPIO_NUM        5
  #define Y4_GPIO_NUM       34
  #define Y3_GPIO_NUM       35
  #define Y2_GPIO_NUM       17
  #define VSYNC_GPIO_NUM    22
  #define HREF_GPIO_NUM     26
  #define PCLK_GPIO_NUM     21

#elif defined(CAMERA_MODEL_AI_THINKER)
  #define PWDN_GPIO_NUM     32
  #define RESET_GPIO_NUM    -1
  #define XCLK_GPIO_NUM      0
  #define SIOD_GPIO_NUM     26
  #define SIOC_GPIO_NUM     27
  
  #define Y9_GPIO_NUM       35
  #define Y8_GPIO_NUM       34
  #define Y7_GPIO_NUM       39
  #define Y6_GPIO_NUM       36
  #define Y5_GPIO_NUM       21
  #define Y4_GPIO_NUM       19
  #define Y3_GPIO_NUM       18
  #define Y2_GPIO_NUM        5
  #define VSYNC_GPIO_NUM    25
  #define HREF_GPIO_NUM     23
  #define PCLK_GPIO_NUM     22

#elif defined(CAMERA_MODEL_M5STACK_PSRAM_B)
  #define PWDN_GPIO_NUM     -1
  #define RESET_GPIO_NUM    15
  #define XCLK_GPIO_NUM     27
  #define SIOD_GPIO_NUM     22
  #define SIOC_GPIO_NUM     23
  
  #define Y9_GPIO_NUM       19
  #define Y8_GPIO_NUM       36
  #define Y7_GPIO_NUM       18
  #define Y6_GPIO_NUM       39
  #define Y5_GPIO_NUM        5
  #define Y4_GPIO_NUM       34
  #define Y3_GPIO_NUM       35
  #define Y2_GPIO_NUM       32
  #define VSYNC_GPIO_NUM    25
  #define HREF_GPIO_NUM     26
  #define PCLK_GPIO_NUM     21

#else
  #error "Camera model not selected"
#endif

#define MOTOR_1_PIN_1    12
#define MOTOR_1_PIN_2    13
#define MOTOR_2_PIN_1    14
#define MOTOR_2_PIN_2    15

#define servoPin 2
int servoPos = 120;
int minServoPos = 75;
int maxServoPos = 155; 
int servoStep = 10;


Servo servoN1;
Servo servoN2;
Servo myServo;

int leftSpeed = 195;
int rightSpeed = 195;
int speedDiff = 15;
int minSpeed = 195;
int maxSpeed = 255;
int speedStep = 10;

static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

httpd_handle_t camera_httpd = NULL;
httpd_handle_t stream_httpd = NULL;

static const char PROGMEM INDEX_HTML[] = R"rawliteral(
<html>
  <head>
    <title>ESP32-CAM Robot</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
      body { font-family: Arial; text-align: center; margin:0px auto; padding-top: 30px;}
      table { margin-left: auto; margin-right: auto; }
      td { padding: 8 px; }
      .button {
        background-color: #2f4468;
        border: none;
        color: white;
        padding: 10px 20px;
        text-align: center;
        text-decoration: none;
        display: inline-block;
        font-size: 18px;
        margin: 6px 3px;
        cursor: pointer;
        -webkit-touch-callout: none;
        -webkit-user-select: none;
        -khtml-user-select: none;
        -moz-user-select: none;
        -ms-user-select: none;
        user-select: none;
        -webkit-tap-highlight-color: rgba(0,0,0,0);
      }
      img {  width: auto ;
        max-width: 100% ;
        height: auto ; 
      }
    </style>
  </head>
  <body>
    <h1>ESP32-CAM Robot</h1>
    <img src="" id="photo" >
    <table>
      <tr><td colspan="3" align="center"><button class="button" onmousedown="toggleCheckbox('forward');" ontouchstart="toggleCheckbox('forward');" onmouseup="toggleCheckbox('stop');" ontouchend="toggleCheckbox('stop');">Forward</button></td></tr>
      <tr><td align="center"><button class="button" onmousedown="toggleCheckbox('left');" ontouchstart="toggleCheckbox('left');" onmouseup="toggleCheckbox('stop');" ontouchend="toggleCheckbox('stop');">Left</button></td><td align="center"><button class="button" onmousedown="toggleCheckbox('stop');" ontouchstart="toggleCheckbox('stop');">Stop</button></td><td align="center"><button class="button" onmousedown="toggleCheckbox('right');" ontouchstart="toggleCheckbox('right');" onmouseup="toggleCheckbox('stop');" ontouchend="toggleCheckbox('stop');">Right</button></td></tr>
      <tr><td colspan="3" align="center"><button class="button" onmousedown="toggleCheckbox('backward');" ontouchstart="toggleCheckbox('backward');" onmouseup="toggleCheckbox('stop');" ontouchend="toggleCheckbox('stop');">Backward</button></td></tr>                 
      <!-- Add an <a> element with an initially empty href -->      
    </table>
   
   <!-- Add an <img> element to load the image -->
      <img src="" id="capturedPhoto" alt="Captured Image"> //http://192.168.1.65/action?go=takePhoto
   <script>
    function toggleCheckbox(x) {
      var xhr = new XMLHttpRequest();
      xhr.open("GET", "/action?go=" + x, true);
      xhr.send();
    }
    window.onload = function() {
      document.getElementById("photo").src = window.location.href.slice(0, -1) + ":81/stream";
    };
  </script>
  </body>
</html>
)rawliteral";

static esp_err_t index_handler(httpd_req_t *req){
  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, (const char *)INDEX_HTML, strlen(INDEX_HTML));
}

static esp_err_t stream_handler(httpd_req_t *req){
  camera_fb_t * fb = NULL;
  esp_err_t res = ESP_OK;
  size_t _jpg_buf_len = 0;
  uint8_t * _jpg_buf = NULL;
  char * part_buf[64];

  res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
  if(res != ESP_OK){
    return res;
  }

  while(true){
    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      res = ESP_FAIL;
    } else {
      if(fb->width > 400){
        if(fb->format != PIXFORMAT_JPEG){
          bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
          esp_camera_fb_return(fb);
          fb = NULL;
          if(!jpeg_converted){
            Serial.println("JPEG compression failed");
            res = ESP_FAIL;
          }
        } else {
          _jpg_buf_len = fb->len;
          _jpg_buf = fb->buf;
        }
      }
    }
    if(res == ESP_OK){
      size_t hlen = snprintf((char *)part_buf, 64, _STREAM_PART, _jpg_buf_len);
      res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
    }
    if(res == ESP_OK){
      res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
    }
    if(res == ESP_OK){
      res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
    }
    if(fb){
      esp_camera_fb_return(fb);
      fb = NULL;
      _jpg_buf = NULL;
    } else if(_jpg_buf){
      free(_jpg_buf);
      _jpg_buf = NULL;
    }
    if(res != ESP_OK){
      break;
    }
    //Serial.printf("MJPG: %uB\n",(uint32_t)(_jpg_buf_len));
  }
  return res;
}

static esp_err_t cmd_handler(httpd_req_t *req){
  char*  buf;
  size_t buf_len;
  char variable[32] = {0,};

  // camera_fb_t * fb = NULL;
  // //esp_err_t res = ESP_OK;
  // size_t _jpg_buf_len = 0;
  // uint8_t * _jpg_buf = NULL;
  // char * part_buf[64];

  camera_fb_t * fb = NULL;
  //esp_err_t res = ESP_OK;
  size_t fb_len = 0;
  int64_t fr_start = esp_timer_get_time();
  
  buf_len = httpd_req_get_url_query_len(req) + 1;
  if (buf_len > 1) {
    buf = (char*)malloc(buf_len);
    if(!buf){
      httpd_resp_send_500(req);
      return ESP_FAIL;
    }
    if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
      if (httpd_query_key_value(buf, "go", variable, sizeof(variable)) == ESP_OK) {
      } else {
        free(buf);
        httpd_resp_send_404(req);
        return ESP_FAIL;
      }
    } else {
      free(buf);
      httpd_resp_send_404(req);
      return ESP_FAIL;
    }
    free(buf);
  } else {
    httpd_resp_send_404(req);
    return ESP_FAIL;
  }

  sensor_t * s = esp_camera_sensor_get();
  int res = 0;
  
  if(!strcmp(variable, "forward")) {
    Serial.println("Forward");
    analogWrite(MOTOR_1_PIN_1, leftSpeed - speedDiff);
    analogWrite(MOTOR_1_PIN_2, 0);
    analogWrite(MOTOR_2_PIN_1, rightSpeed);
    analogWrite(MOTOR_2_PIN_2, 0);
  }
  else if(!strcmp(variable, "veerLeft")) {
    Serial.println("Veer Left");
    analogWrite(MOTOR_1_PIN_1, 0);
    analogWrite(MOTOR_1_PIN_2, 0);
    analogWrite(MOTOR_2_PIN_1, rightSpeed);
    analogWrite(MOTOR_2_PIN_2, 0);
  }
  else if(!strcmp(variable, "veerRight")) {
    Serial.println("Veer Right");
    analogWrite(MOTOR_1_PIN_1, leftSpeed);
    analogWrite(MOTOR_1_PIN_2, 0);
    analogWrite(MOTOR_2_PIN_1, 0);
    analogWrite(MOTOR_2_PIN_2, 0);
  }
  else if(!strcmp(variable, "left")) {
    Serial.println("Left");
    analogWrite(MOTOR_1_PIN_1, 0);
    analogWrite(MOTOR_1_PIN_2, leftSpeed);
    analogWrite(MOTOR_2_PIN_1, rightSpeed);
    analogWrite(MOTOR_2_PIN_2, 0);
  }
  else if(!strcmp(variable, "right")) {
    Serial.println("Right");
    analogWrite(MOTOR_1_PIN_1, leftSpeed);
    analogWrite(MOTOR_1_PIN_2, 0);
    analogWrite(MOTOR_2_PIN_1, 0);
    analogWrite(MOTOR_2_PIN_2, rightSpeed);
  }
  else if(!strcmp(variable, "backward")) {
    Serial.println("Backward");
    analogWrite(MOTOR_1_PIN_1, 0);
    analogWrite(MOTOR_1_PIN_2, leftSpeed);
    analogWrite(MOTOR_2_PIN_1, 0);
    analogWrite(MOTOR_2_PIN_2, rightSpeed);
  }
   else if(!strcmp(variable, "reverseLeft")) {
    Serial.println("Reverse Left");
    analogWrite(MOTOR_1_PIN_1, 0);
    analogWrite(MOTOR_1_PIN_2, 0);
    analogWrite(MOTOR_2_PIN_1, 0);
    analogWrite(MOTOR_2_PIN_2, rightSpeed);
  }
  else if(!strcmp(variable, "reverseRight")) {
    Serial.println("Reverse Right");
    analogWrite(MOTOR_1_PIN_1, 0);
    analogWrite(MOTOR_1_PIN_2, rightSpeed);
    analogWrite(MOTOR_2_PIN_1, 0);
    analogWrite(MOTOR_2_PIN_2, 0);
  }

  else if(!strcmp(variable, "stop")) {
    Serial.println("Stop");
    analogWrite(MOTOR_1_PIN_1, 0);
    analogWrite(MOTOR_1_PIN_2, 0);
    analogWrite(MOTOR_2_PIN_1, 0);
    analogWrite(MOTOR_2_PIN_2, 0);
  }
  
  else if(!strcmp(variable, "takePhoto")) {
    Serial.println("Take a photo");
    //----------------------------
    fb = esp_camera_fb_get();
    if (!fb) {
        ESP_LOGE(TAG, "Camera capture failed");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    res = httpd_resp_set_type(req, "image/jpeg");
    if(res == ESP_OK){
        res = httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=capture.jpg");
    }

    if(res == ESP_OK){
        if(fb->format == PIXFORMAT_JPEG){
            fb_len = fb->len;
            res = httpd_resp_send(req, (const char *)fb->buf, fb->len);
        } else {
            jpg_chunking_t jchunk = {req, 0};
            res = frame2jpg_cb(fb, 80, jpg_encode_stream, &jchunk)?ESP_OK:ESP_FAIL;
            httpd_resp_send_chunk(req, NULL, 0);
            fb_len = jchunk.len;
        }
    }
    esp_camera_fb_return(fb);
    int64_t fr_end = esp_timer_get_time();
    ESP_LOGI(TAG, "JPG: %uKB %ums", (uint32_t)(fb_len/1024), (uint32_t)((fr_end - fr_start)/1000));
    return res;
  }

  else if(!strcmp(variable, "leftSpeedIncrease")) {
    if ((leftSpeed + speedStep) > maxSpeed) {
      Serial.println("Maximum speed has been reached");
      leftSpeed = maxSpeed;
    } else {
      leftSpeed += speedStep;
    };
    Serial.println("Left speed increased");
  }

  else if(!strcmp(variable, "leftSpeedDecrease")) {
    if ((leftSpeed - speedStep) < minSpeed) {
      Serial.println("Minimum speed has been reached");
      leftSpeed = minSpeed;
    } else {
      leftSpeed -= speedStep;
    };
    Serial.println("Left speed decreased");
  }

  else if(!strcmp(variable, "leftSpeed")) {
    Serial.println("Get left speed");
    char leftSpeedStr[3];
    sprintf(leftSpeedStr, "%d", leftSpeed);
    httpd_resp_send(req, leftSpeedStr, HTTPD_RESP_USE_STRLEN);
    return res;
  }
  //-------------------------------------------
  else if(!strcmp(variable, "rightSpeedIncrease")) {
    if ((rightSpeed + speedStep) > maxSpeed) {
      Serial.println("Maximum speed has been reached");
      rightSpeed = maxSpeed;
    } else {
      rightSpeed += speedStep;
    };
    Serial.println("Right speed increased");
  }

  else if(!strcmp(variable, "rightSpeedDecrease")) {
    if ((rightSpeed - speedStep) < minSpeed) {
      Serial.println("Minimum speed has been reached");
      rightSpeed = minSpeed;
    } else {
      rightSpeed -= speedStep;
    };
    Serial.println("Right speed decreased");
  }

  else if(!strcmp(variable, "rightSpeed")) {
    Serial.println("Get right speed");
    char rightSpeedStr[3];
    sprintf(rightSpeedStr, "%d", rightSpeed);
    httpd_resp_send(req, rightSpeedStr, HTTPD_RESP_USE_STRLEN);
    return res;
  }

  //-------------------------------------------

  else if(!strcmp(variable, "servoLeft")) {
    Serial.println("Servo Left");
    if ((servoPos + servoStep) > maxServoPos) {
      Serial.println("Servo left limit has been reached");
      servoPos = maxServoPos;
    } else {
      servoPos += servoStep;
    };
    myServo.write(servoPos);
  }

  else if(!strcmp(variable, "servoRight")) {
    Serial.println("Servo Right");
    if ((servoPos - servoStep) < minServoPos) {
      Serial.println("Servo right limit has been reached");
      servoPos = minServoPos;
    } else {
      servoPos -= servoStep;
    };
    
    myServo.write(servoPos);
  }

  else if(!strcmp(variable, "servoPos")) {
    Serial.println("Get Servo Position");
    char servoPosStr[3];
    sprintf(servoPosStr, "%d", servoPos);
    httpd_resp_send(req, servoPosStr, HTTPD_RESP_USE_STRLEN);
    return res;
  }

  //-------------------------------------------

  
  else {
    res = -1;
  }

  if(res){
    return httpd_resp_send_500(req);
  }

  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  return httpd_resp_send(req, NULL, 0);
}

static esp_err_t set_handler(httpd_req_t *req){
  char*  buf;
  size_t buf_len;
  char var_name[32] = {0,};
  char var_value[4] = {0,};  
  buf_len = httpd_req_get_url_query_len(req) + 1;
  if (buf_len > 1) {
    buf = (char*)malloc(buf_len);
    if(!buf){
      httpd_resp_send_500(req);
      return ESP_FAIL;
    }
    if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
      // Serial.print("buf: ");
      // Serial.println(buf);
      if (httpd_query_key_value(buf, "name", var_name, sizeof(var_name)) == ESP_OK) {
        // Serial.print("var_name: ");
        // Serial.println(var_name);
        
        if (httpd_query_key_value(buf, "value", var_value, sizeof(var_value)) == ESP_OK){
          // Serial.println(var_value);
        }
      } else {
        free(buf);
        httpd_resp_send_404(req);
        return ESP_FAIL;
      }
    } else {
      free(buf);
      httpd_resp_send_404(req);
      return ESP_FAIL;
    }
    free(buf);
  } else {
    httpd_resp_send_404(req);
    return ESP_FAIL;
  }

  //sensor_t * s = esp_camera_sensor_get();
  int res = 0;
  
  if(!strcmp(var_name, "setLeftSpeed")) {
    int speedValue =  atoi(var_value);;
    if (speedValue > maxSpeed){
      leftSpeed = maxSpeed;
      Serial.println("Max speed has been reached");
    } else if (speedValue < minSpeed){
      leftSpeed = minSpeed;
      Serial.println("Min speed has been reached");
    } else {
      leftSpeed = speedValue;
    }; 
    Serial.println("Left Speed Set");
    
  }
  else if(!strcmp(var_name, "setRightSpeed")) {
    int speedValue =  atoi(var_value);;
    if (speedValue > maxSpeed){
      rightSpeed = maxSpeed;
      Serial.println("Max speed has been reached");
    } else if (speedValue < minSpeed){
      rightSpeed = minSpeed;
      Serial.println("Min speed has been reached");
    } else {
      rightSpeed = speedValue;
    };
    Serial.println("Right Speed Set");  
  }
  else if(!strcmp(var_name, "setServoPos")) {
    int servoValue =  atoi(var_value);
    if (servoValue > maxServoPos){
      servoPos = maxServoPos;
      Serial.println("Left servo limit has been reached");
    } else if (servoValue < minServoPos){
      servoPos = minServoPos;
      Serial.println("Right servo limit has been reached");
    } else {
      servoPos = servoValue;
    }
    myServo.write(servoPos);
    Serial.println("Servo Position Set");
  }
  else {
    res = -1;
  }
  if(res){
    return httpd_resp_send_500(req);
  }

  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  return httpd_resp_send(req, NULL, 0);
}



void startCameraServer(){
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = 80;
  httpd_uri_t index_uri = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = index_handler,
    .user_ctx  = NULL
  };

  httpd_uri_t cmd_uri = {
    .uri       = "/action",
    .method    = HTTP_GET,
    .handler   = cmd_handler,
    .user_ctx  = NULL
  };
  httpd_uri_t set_uri = {
    .uri       = "/set",
    .method    = HTTP_POST,
    .handler   = set_handler,
    .user_ctx  = NULL
  };
  httpd_uri_t stream_uri = {
    .uri       = "/stream",
    .method    = HTTP_GET,
    .handler   = stream_handler,
    .user_ctx  = NULL
  };
  if (httpd_start(&camera_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(camera_httpd, &index_uri);
    httpd_register_uri_handler(camera_httpd, &cmd_uri);
    httpd_register_uri_handler(camera_httpd, &set_uri);
  }
  config.server_port += 1;
  config.ctrl_port += 1;
  if (httpd_start(&stream_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(stream_httpd, &stream_uri);
  }
}

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
  
  pinMode(MOTOR_1_PIN_1, OUTPUT);
  pinMode(MOTOR_1_PIN_2, OUTPUT);
  pinMode(MOTOR_2_PIN_1, OUTPUT);
  pinMode(MOTOR_2_PIN_2, OUTPUT);
  
  //servoN1.attach(2, 1000, 2000);
  //servoN2.attach(16, 1000, 2000);

  myServo.setPeriodHertz(50);
  myServo.attach(servoPin, 700, 2400);
  myServo.write(servoPos);
  
  Serial.begin(115200);
  Serial.setDebugOutput(false);
  
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
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG; 
  
  if(psramFound()){
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  
  // Camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
  // Wi-Fi connection
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  
  Serial.print("Camera Stream Ready! Go to: http://");
  Serial.println(WiFi.localIP());
  
  // Start streaming web server
  startCameraServer();
}

static size_t jpg_encode_stream(void * arg, size_t index, const void* data, size_t len){
    jpg_chunking_t *j = (jpg_chunking_t *)arg;
    if(!index){
        j->len = 0;
    }
    if(httpd_resp_send_chunk(j->req, (const char *)data, len) != ESP_OK){
        return 0;
    }
    j->len += len;
    return len;
}

void loop() {
  
}
