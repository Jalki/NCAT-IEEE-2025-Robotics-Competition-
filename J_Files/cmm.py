import cv2
import time
import numpy as np
import serial
import apriltag
from ultralytics import YOLO

# Instead of using a global variable, initialize the file to indicate 0 ball detections.
with open("ballexist", "w") as f:
    f.write("0")

# ----- Initialize Video Capture and Models -----
cap = cv2.VideoCapture(0)

# YOLO model (ensure 'best_ncnn_model' is available)
yolo = YOLO('best_ncnn_model')

# AprilTag detector
detector = apriltag.Detector()

# ----- Camera Calibration and Tag Parameters for AprilTag Detection -----
camera_matrix = np.array([[600, 0, 320],
                          [0, 600, 240],
                          [0,   0,   1]], dtype=np.float32)
dist_coeffs = np.zeros(4)  # Assuming no lens distortion
tag_size = 0.1  # Tag size in meters

# ----- Serial Communication -----
ser = serial.Serial('/dev/ttyAMA0', 9600, timeout=1)  # Update port if necessary
time.sleep(2)  # Allow time for serial connection to initialize

# ----- Utility Functions -----
def plotPoint(image, center, color):
    center = (int(center[0]), int(center[1]))
    image = cv2.line(image, (center[0] - 5, center[1]), (center[0] + 5, center[1]), color, 3)
    image = cv2.line(image, (center[0], center[1] - 5), (center[0], center[1] + 5), color, 3)
    return image

def plotText(image, center, color, text):
    # Offset text so that it doesn't overlap the center point exactly
    center = (int(center[0]) + 4, int(center[1]) - 4)
    return cv2.putText(image, str(text), center, cv2.FONT_HERSHEY_SIMPLEX, 1, color, 3)

def getColours(cls_num):
    base_colors = [(255, 0, 0), (0, 255, 0), (0, 0, 255)]
    color_index = cls_num % len(base_colors)
    increments = [(1, -2, 1), (-2, 1, -1), (1, -1, 2)]
    color = [base_colors[color_index][i] + increments[color_index][i] * (cls_num // len(base_colors)) % 256 
             for i in range(3)]
    return tuple(color)

def estimate_depth(detect, camera_matrix, dist_coeffs, tag_size):
    # Define the object points of the tag in its own coordinate system.
    object_points = np.array([[-tag_size / 2, -tag_size / 2, 0],
                              [ tag_size / 2, -tag_size / 2, 0],
                              [ tag_size / 2,  tag_size / 2, 0],
                              [-tag_size / 2,  tag_size / 2, 0]], dtype=np.float32)
    image_points = np.array(detect.corners, dtype=np.float32)
    _, rvec, tvec = cv2.solvePnP(object_points, image_points, camera_matrix, dist_coeffs)
    return tvec[2][0], detect.center, detect.corners  # Return depth (Z), center, and corners

def check_angle(corners, tolerance=0):
    def euclidean_distance(pt1, pt2):
        return np.linalg.norm(np.array(pt1) - np.array(pt2))
    
    d1 = euclidean_distance(corners[0], corners[1])
    d2 = euclidean_distance(corners[1], corners[2])
    d3 = euclidean_distance(corners[2], corners[3])
    d4 = euclidean_distance(corners[3], corners[0])
    
    if abs(d1 - d3) > tolerance or abs(d2 - d4) > tolerance:
        return "At an angle"
    else:
        return "Straight"

# Process interval for YOLO detections
process_interval = 0.1  # seconds
last_process_time = time.time()

# ----- Main Processing Loop -----
while True:
    ret, frame = cap.read()
    if not ret:
        continue

    # ------------------- YOLO Detection -------------------
    current_time = time.time()
    if current_time - last_process_time >= process_interval:
        yolo_results = yolo.predict(frame, stream=True)
        last_process_time = current_time
    else:
        yolo_results = []
    
    frame_width = frame.shape[1]
    # Iterate over YOLO detections
    for result in yolo_results:
        classes_names = result.names
        for box in result.boxes:
            if box.conf[0] > 0.4:  # Confidence threshold
                [x1, y1, x2, y2] = box.xyxy[0]
                x1, y1, x2, y2 = int(x1), int(y1), int(x2), int(y2)
                cls = int(box.cls[0])
                class_name = classes_names[cls]
                
                # Process only if the detected object is a ball.
                if class_name.lower() == "ball":
                    center_x = (x1 + x2) / 2
                    left_bound = frame_width * 0.25
                    right_bound = frame_width * 0.75
                    # Only increment the counter if the ball is in the center region.
                    if left_bound <= center_x <= right_bound:
                        try:
                            with open("ballexist", "r") as f:
                                current_count = int(f.read().strip())
                        except:
                            current_count = 0
                        current_count += 1
                        with open("ballexist", "w") as f:
                            f.write(str(current_count))
                    else:
                        # If the ball is outside the center region, skip further processing for this box.
                        continue

                colour = getColours(cls)
                cv2.rectangle(frame, (x1, y1), (x2, y2), colour, 2)
                cv2.putText(frame, f'{class_name} {box.conf[0]:.2f}', (x1, y1),
                            cv2.FONT_HERSHEY_SIMPLEX, 1, colour, 2)
    
    # ------------------- AprilTag Detection -------------------
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    detections = detector.detect(gray)
    
    if detections:
        for detect in detections:
            z_depth, center, corners = estimate_depth(detect, camera_matrix, dist_coeffs, tag_size)
            img_center_x = frame_width / 2
            tag_x, _ = center
            alignment = "Centered" if abs(tag_x - img_center_x) < 10 else ("Left" if tag_x < img_center_x else "Right")
            angle_status = check_angle(corners, tolerance=15)
            
            print(f"Tag ID: {detect.tag_id}, Depth: {z_depth:.2f} m, Position: {alignment}, Orientation: {angle_status}")
            
            # Send data over UART
            ser.write(f"Tag ID: {detect.tag_id}\n".encode())
            time.sleep(0.1)
            ser.write(f"Z:{z_depth:.2f}\n".encode())
            time.sleep(0.1)
            ser.write(f"Z:{z_depth:.2f}, Pos:{alignment}, Angle:{angle_status}\n".encode())
            time.sleep(0.1)
            
            # Annotate the frame for the AprilTag detection
            frame = plotPoint(frame, center, (0, 255, 0))
            frame = plotText(frame, center, (0, 255, 0), f"ID: {detect.tag_id}, Z:{z_depth:.2f}m, {alignment}, {angle_status}")
            for corner in corners:
                frame = plotPoint(frame, corner, (255, 0, 255))
    else:
        print("No AprilTag detected")
    
    # ------------------- Display Combined Results -------------------
    cv2.imshow('Result', frame)
    key = cv2.waitKey(100)
    if key == 13:  # Exit if Enter key is pressed
        break

# ----- Cleanup -----
cap.release()
cv2.destroyAllWindows()
cv2.imwrite("final.png", frame)
