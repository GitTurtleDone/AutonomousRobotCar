import numpy as np
from flask import Flask, request, render_template
from PIL import Image
import requests
from io import BytesIO
from ultralytics import YOLO
import os


# image_url = "http://192.168.1.64/action?go=takePhoto"
# response = requests.get(image_url)
# img = Image.open(BytesIO(response.content))
# img = img.save("CNN.jpg")

app = Flask(__name__)

# Initialize a global variables
saveFolderPath = "./datasets/SurroundingObjects/val/images/"
# Use the os.listdir() function to get a list of all items (files and folders) in the directory
dirContents = os.listdir(saveFolderPath)
# Look for image files in the folder
files = [item for item in dirContents if os.path.isfile(os.path.join(saveFolderPath, item))]
# set the image_counter by counting all the files in the folder to be saved to
imageCounter = len(files) + 1
servoAngle = "135"
leftSpeed = "255"
rightSpeed = "255"

model = YOLO('./runs/detect/train12/weights/best.pt')

@app.route('/')
def index():
    return render_template('index.html', counter=imageCounter, servoAngle = servoAngle, leftSpeed = leftSpeed, rightSpeed = rightSpeed)
    # the index .html file should be stored in a "template" folder of the project file

@app.route('/favicon.ico')
def favicon():
    return ('', 204)
    # This to get rid of favicon error.


@app.route('/download', methods=['POST'])
def download_image():
    global imageCounter, saveFolderPath
    
    
    image_url = "http://192.168.1.64/action?go=takePhoto"
    response = requests.get(image_url)
    img = Image.open(BytesIO(response.content))
    # Generate the filename using the counter
    filename = saveFolderPath + f"image{imageCounter}.jpg"
    img.save(filename)
    
    imageCounter += 1

    return "Image downloaded and saved as " + filename
@app.route('/predict', methods=['POST'])
def predictImage():
    global model
    image_url = "http://192.168.1.64/action?go=takePhoto"
    response = requests.get(image_url)
    img = Image.open(BytesIO(response.content))

    # Generate the filename using the counter
    filename = f"./PredictedImages/image.jpg"
    img.save(filename)
    result = model.predict(source="./PredictedImages/image.jpg", classes=None, conf=0.5)

    # return bboxes, the last line contains coordinates in percentage
    return ([result[0].boxes.cls.cpu().numpy().tolist(),
            result[0].boxes.conf.cpu().numpy().tolist(),
            (result[0].boxes.xywhn.cpu().numpy() *100).tolist()]) 



@app.route('/getServoAngle', methods=['POST'])
def get_servoAngle():
    servoAngle_url = "http://192.168.1.64/action?go=servoAngle"
    global servoAngle, image_counter
    servoAngle = requests.get(servoAngle_url).text
  
    

    return servoAngle
@app.route('/getLeftSpeed', methods=['POST'])
def get_leftSpeed():
    leftSpeed_url = "http://192.168.1.64/action?go=leftSpeed"
    leftSpeed = requests.get(leftSpeed_url).text
  
    

    return leftSpeed
@app.route('/getRightSpeed', methods=['POST'])
def get_rightSpeed():
    rightSpeed_url = "http://192.168.1.64/action?go=rightSpeed"
    rightSpeed = requests.get(rightSpeed_url).text

    

    return rightSpeed

if __name__ == '__main__':
    app.run()

