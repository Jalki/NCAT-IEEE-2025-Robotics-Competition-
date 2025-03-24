import cv2
import apriltag
import numpy as np
import serial
import time

def plotPoint(image, center, color):
    center = (int(center[0]), int(center[1]))
    image = cv2.line(image, (center[0] - 5, center[1]), (center[0] + 5, center[1]), color, 3)
    image = cv2.line(image, (center[0], center[1] - 5), (center[0], center[1] + 5), color, 3)
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
    return tvec[2][0], detect.center, detect.corners  # Return Z-axis, center, and corners

def check_angle(corners, tolerance = 0):
    def euclidean_distance(pt1, pt2):
        return np.linalg.norm(np.array(pt1) - np.array(pt2))
    
    d1 = euclidean_distance(corners[0], corners[1])
    d2 = euclidean_distance(corners[1], corners[2])
    d3 = euclidean_distance(corners[2], corners[3])
    d4 = euclidean_distance(corners[3], corners[0])
    
    if abs(d1 - d3) > tolerance or abs(d2 - d4) > tolerance :
        return "At an angle"
    else:
        return "Straight"

detector = apriltag.Detector()
cam = cv2.VideoCapture(0)

camera_matrix = np.array([[600, 0, 320], [0, 600, 240], [0, 0, 1]], dtype=np.float32)
dist_coeffs = np.zeros(4)
tag_size = 0.1

ser = serial.Serial('/dev/ttyAMA0', 9600, timeout=1)
time.sleep(2)

looping = True
while looping:
    result, image = cam.read()
    grayimg = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
    detections = detector.detect(grayimg)
    
    if not detections:
        print("Nothing")
    else:
        for detect in detections:
            z_depth, center, corners = estimate_depth(detect, camera_matrix, dist_coeffs, tag_size)
            img_center_x = image.shape[1] / 2
            tag_x, tag_y = center
            alignment = "Centered" if abs(tag_x - img_center_x) < 10 else ("Left" if tag_x < img_center_x else "Right")
            angle_status = check_angle(corners, tolerance=15)
            
            print(f"Tag ID: {detect.tag_id}, Depth: {z_depth:.2f} meters, Position: {alignment}, Orientation: {angle_status}")
            
            # Send Tag ID via UART
            tagMessage = f"Tag ID: {detect.tag_id}\n"
            ser.write(tagMessage.encode())
            time.sleep(0.1)
            
            # Send Z-depth value over UART
            depthMessage = f"Z:{z_depth:.2f}\n"
            ser.write(depthMessage.encode())  # Send as bytes
            time.sleep(0.1)  # Avoid flooding serial buffer
            
            message = f"Z:{z_depth:.2f}, Pos:{alignment}, Angle:{angle_status}\n"
            ser.write(message.encode())
            time.sleep(0.1)
            
            image = plotPoint(image, detect.center, (0, 255, 0))
            image = plotText(image, detect.center, (0, 255, 0), f"ID: {detect.tag_id}, Z: {z_depth:.2f}m, {alignment}, {angle_status}")
            for corner in detect.corners:
                image = plotPoint(image, corner, (255, 0, 255))
    
    cv2.imshow('Result', image)
    key = cv2.waitKey(100)
    if key == 13:
        looping = False

cv2.destroyAllWindows()
cv2.imwrite("final.png", image)
