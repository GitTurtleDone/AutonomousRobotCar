from ultralytics import YOLO
def main():
    model = YOLO('./runs/detect/train5/weights/best.pt')
    result = model.predict(source="http://192.168.1.64/action?go=takePhoto", show=True)
    #print (result[0].boxes)
    

if __name__ == "__main__":
    main()



# import cv2
# from ultralytics import YOLO

# # Load the YOLOv8 model
# model = YOLO('./runs/detect/train5/weights/best.pt')

# # Open the video file
# video_path = "path/to/your/video/file.mp4"
# cap = cv2.VideoCapture(video_path)

# # Loop through the video frames
# while cap.isOpened():
#     # Read a frame from the video
#     success, frame = cap.read()

#     if success:
#         # Run YOLOv8 inference on the frame
#         results = model(frame)

#         # Visualize the results on the frame
#         annotated_frame = results[0].plot()

#         # Display the annotated frame
#         cv2.imshow("YOLOv8 Inference", annotated_frame)

#         # Break the loop if 'q' is pressed
#         if cv2.waitKey(1) & 0xFF == ord("q"):
#             break
#     else:
#         # Break the loop if the end of the video is reached
#         break

# # Release the video capture object and close the display window
# cap.release()
# cv2.destroyAllWindows()
