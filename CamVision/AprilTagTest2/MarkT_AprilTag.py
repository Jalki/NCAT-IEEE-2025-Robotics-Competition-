import cv2
import apriltag
import numpy as np
import serial
import time

LINE_LENGTH = 5
CENTER_COLOR = (0, 255, 0)
CORNER_COLOR = (255, 0, 255)

def plotPoint(image, center, color):
    center = (int(center[0]), int(center[1]))
    image = cv2.line(image, (center[0] - LINE_LENGTH, center[1]), (center[0] + LINE_LENGTH, center[1]), color, 3)
    image = cv2.line(image, (center[0], center[1] - LINE_LENGTH), (center[0], center[1] + LINE_LENGTH), color, 3)
    return image

def plotText(image, center, color, text):
    center = (int(center[0]) + 4, int(center[1]) - 4)
    return cv2.putText(image, str(text), center, cv2.FONT_HERSHEY_SIMPLEX, 1, color, 3)

def estimate_depth(detect, camera_matrix, dist_coeffs, tag_size):
    object_points = np.array([[-tag_size / 2, -tag_size / 2, 0],
                              [tag_size / 2, -tag_size / 2, 0],
                              [tag_size / 2, tag_size / 2, 0],
                              [-tag_size / 2, tag_size / 2, 0]], dtype=np.float32)
    image_points = np.array(detect.corners, dtype=np.float32)
    _, rvec, tvec = cv2.solvePnP(object_points, image_points, camera_matrix, dist_coeffs)
    return tvec[2][0], detect.center  # Z-axis (depth) and center position

detector = apriltag.Detector()
cam = cv2.VideoCapture(0)

# Example camera parameters (fx, fy, cx, cy). Replace with actual calibration values.
camera_matrix = np.array([[600, 0, 320], [0, 600, 240], [0, 0, 1]], dtype=np.float32)
dist_coeffs = np.zeros(4)  # Assuming no lens distortion
tag_size = 0.1  # AprilTag is 100mm (0.1 meters)

# Initialize Serial Communication
ser = serial.Serial('/dev/ttyS0', 115200, timeout=1)  # Adjust for Raspberry Pi
time.sleep(2)  # Wait for serial connection to establish

looping = True
while looping:
    result, image = cam.read()
    grayimg = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
    detections = detector.detect(grayimg)
    
    if not detections:
        print("Nothing")
    else:
        for detect in detections:
            z_depth, center = estimate_depth(detect, camera_matrix, dist_coeffs, tag_size)
            img_center_x = image.shape[1] / 2
            img_center_y = image.shape[0] / 2
            tag_x, tag_y = center
            
            # Determine alignment
            alignment = "Centered" if abs(tag_x - img_center_x) < 30 else ("Left" if tag_x < img_center_x else "Right")
            
            print(f"Tag ID: {detect.tag_id}, Depth: {z_depth:.2f} meters, Position: {alignment}")
            
            # Send Z-depth and alignment over UART
            message = f"Z:{z_depth:.2f}, Pos:{alignment}\n"
            ser.write(message.encode())  # Send as bytes
            time.sleep(0.1)  # Avoid flooding serial buffer
            
            image = plotPoint(image, detect.center, CENTER_COLOR)
            image = plotText(image, detect.center, CENTER_COLOR, f"ID: {detect.tag_id}, Z: {z_depth:.2f}m, {alignment}")
            for corner in detect.corners:
                image = plotPoint(image, corner, CORNER_COLOR)
    
    cv2.imshow('Result', image)
    key = cv2.waitKey(100)
    if key == 13:
        looping = False

cv2.destroyAllWindows()
cv2.imwrite("final.png", image)
