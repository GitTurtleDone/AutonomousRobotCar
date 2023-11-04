from ultralytics import YOLO
#model = YOLO('./yolov8s.pt')
model = YOLO('./runs/detect/train12/weights/best.pt')
model.train(model='./runs/detect/train12/weights/best.pt',\
            data='./SurroundingObjects.yaml', \
            epochs=1000, batch=16,\
            imgsz = 640, patience = 200)