# import requests

# # URL of the web server you want to access
# url = "http://192.168.1.65"  # Replace with the URL you want to access

# # Send an HTTP GET request to the server
# response = requests.get(url)

# # Check if the request was successful (status code 200)
# if response.status_code == 200:
#     print("Request was successful!")
#     print("Response content:")
#     print(response.text)
# else:
#     print(f"Request failed with status code {response.status_code}")


# URL of the video stream (replace with your video source)
# video_url = "http://192.168.1.65:81/stream"  # Replace with the URL of the video stream




# Define the URL of the image you want to download
# image_url = "http://192.168.1.65/action?go=takePhoto"  # Replace with the URL of the image you want to download

# Create a simple HTTP server that serves a page with the image
# class ImageHandler(http.server.SimpleHTTPRequestHandler):
#     def do_GET(self):
#         if self.path == '/image.jpg':
#             # Download the image and save it to a file
#             response = requests.get(image_url)
#             if response.status_code == 200:
#                 with open("downloaded_image.jpg", "wb") as f:
#                     f.write(response.content)
#             self.send_response(200)
#             self.end_headers()
#             self.wfile.write(b"Image downloaded and saved as 'downloaded_image.jpg'")
#         else:
#             super().do_GET()

# with socketserver.TCPServer((host, port), ImageHandler) as server:
#     print(f"Serving at http://{host}:{port}")
#     server.serve_forever()

# from PIL import Image
# import requests
# from io import BytesIO

# response = requests.get(image_url)
# img = Image.open(BytesIO(response.content))
# img = img.save("CNN.jpg") 



from flask import Flask, request, render_template
from PIL import Image
import requests
from io import BytesIO

# image_url = "http://192.168.1.64/action?go=takePhoto"
# response = requests.get(image_url)
# img = Image.open(BytesIO(response.content))
# img = img.save("CNN.jpg")

app = Flask(__name__)

# Initialize a global counter
image_counter = 1
servoAngle = "135"
leftSpeed = "255"
rightSpeed = "255"

@app.route('/')
def index():
    return render_template('index.html', counter=image_counter, servoAngle = servoAngle, leftSpeed = leftSpeed, rightSpeed = rightSpeed)
    # the index .html file should be stored in a "template" folder of the project file

@app.route('/favicon.ico')
def favicon():
    return ('', 204)
    # This to get rid of favicon error.


@app.route('/download', methods=['POST'])
def download_image():
    global image_counter
    image_url = "http://192.168.1.64/action?go=takePhoto"
    response = requests.get(image_url)
    img = Image.open(BytesIO(response.content))
    # Generate the filename using the counter
    filename = f"CaptureImages/image{image_counter}.jpg"
    img.save(filename)
    # image_url = request.form['image_url']
    # response = requests.get(image_url)
    # img = Image.open(BytesIO(response.content))
    # img.save("static/CNN.jpg")  # Save the image in a 'static' directory
    image_counter += 1

    return "Image downloaded and saved as " + filename
@app.route('/getServoAngle', methods=['POST'])
def get_servoAngle():
    servoAngle_url = "http://192.168.1.64/action?go=servoAngle"
    global servoAngle, image_counter
    servoAngle = requests.get(servoAngle_url).text
    # console.log(servoAngle)
    # if not isinstance(servoAngle,str):
    #     servoAngle = str(servoAngle)
    

    return servoAngle
@app.route('/getLeftSpeed', methods=['POST'])
def get_leftSpeed():
    leftSpeed_url = "http://192.168.1.64/action?go=leftSpeed"
    leftSpeed = requests.get(leftSpeed_url).text
    # console.log(servoAngle)
    # if not isinstance(servoAngle,str):
    #     servoAngle = str(servoAngle)
    

    return leftSpeed
@app.route('/getRightSpeed', methods=['POST'])
def get_rightSpeed():
    rightSpeed_url = "http://192.168.1.64/action?go=rightSpeed"
    rightSpeed = requests.get(rightSpeed_url).text
    # console.log(servoAngle)
    # if not isinstance(servoAngle,str):
    #     servoAngle = str(servoAngle)
    

    return rightSpeed

if __name__ == '__main__':
    # run app in debug mode on port 5000
    app.run()

