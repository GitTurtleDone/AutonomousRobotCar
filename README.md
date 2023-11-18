AutonomousRobotCar is a project to build a robot car that can run autonomously.

## Hardware

- ESP32 CAM. I bought mine from [here] (https://www.aliexpress.com/item/1005003804757059.html?spm=a2g0o.order_list.order_list_main.5.4a1d1802VfxSE3)
- Motor driver. Any motor drivers capable of running two motors are fine. I designed mine
- Chassis with 2 motors and 2 wheels and a servo motor. I bought [this] but didn't use their boards, ultrasonic sensor, or battery case (https://www.aliexpress.com/item/1005003057841508.html?spm=a2g0o.order_list.order_list_main.15.21ef1802QNeYe5)
- A high-end computer where python, flask, ultralytics, and cuda packages can be installed. The better the computer, the faster the predictions, and the smoother the run.
- Wi-fi network that can be shared by both the ESP32 CAM and the computer.

## Use

- Connects the motor driver pins to the ESP32 CAM pins 12, 13, 14, and 15
- Connect the servo pin to the ESP32 CAM pins 2 (somehow pins 4 and 16 didn't work for me, I suspected that it is related to the working of the camera flash, but didn't delve into why)
- Connects appropriate the 5 V power supply pins to the servo, the ESP32 CAM, and the motor drivers.
- Upload the ./RobotControlViaCamera2/RobotControlViaCamera.ino Arduino sketch (there are plenty of tutorials onlin to show you how to do that, in my case I use an Arduino Uno to upload the codes to
  the ESP32 CAM, Arduino Nano also works fine, see SelectionsToUploadToESP32CAM.JPG). Change your wifi network ssid and password accordingly.
- Power and reset the ESP32 CAM, using the serial monitor of the Arduino IDE to get the IP addess. Change "192.168.1.65" in index.html and DownloadImages.py with your IP address.
- Download and Install Python, Flask, tensorflow, ultralytics, cuda, and other necessary packages and run the DownloadImages.py. It will ouput something like "Running on http://127.0.0.1:5000". Paste http://127.0.0.1:5000 onto your browser. Note that your computer and the ESP32 CAM must use the same wi-fi network. Otherwise, it can't work.
- Run the robot car manually, while taking images for YOLOv8 training, the images will be saved in the "CaptureImages" folder. Separate the captured images into train and val sets, and move them to the
  .datasets/SurroundingObjects/train (or val)/images folders.
- Label images using the website makesense.ai. Save all individual label .txt files in the .datasets/SurroundingObjects/train (or val)/labels folders using YOLO format. Remember lableling the empty space bounding boxes in the images. By definition, the empty space bboxes must have bottom edges coinciding with the bottom of the image frames.
- Modify the Surrounding.yaml file accordingly. It would be interesting to try to detect only empty space bboxes but I haven't tried that.
- Train the model running AutonomousRobotCar.py, having modified the lines in advance. After the training finish, record where the 'best.pt' is stored
- Modified the "model = YOLO('./runs/detect/train14/weights/best.pt')" in DownloadImages.py accordingly and run it.
- Open the browser and enjoy the Autonomous Robot Car.

### License

AutonomousRobotCar is [MIT licensed](./LICENSE).
