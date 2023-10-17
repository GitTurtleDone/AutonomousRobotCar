from ultralytics import YOLO
model = YOLO('./yolov8s.pt')
model.train(model='./yolov8s.pt',\
            data='./VaseBookShoes.yaml', \
            epochs=100, batch=16,\
            imgsz = 640, patience = 100)