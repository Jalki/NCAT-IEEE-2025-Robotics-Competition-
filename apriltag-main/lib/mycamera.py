import numpy as np
import cv2
import glob

# ======================== Camera class==================================

class Camera:

   def __init__(self):
      with np.load('/home/SECON2025/Downloads/apriltag-main/camera_calibration/calibration_savez.npz') as X:
         mtx, dist, self.rvecs, self.tvecs = [X[i] for i in ('mtx','dist','rvecs','tvecs')]
      self.capture = cv2.VideoCapture(0)

      fx   = mtx[0,0]
      fy   = mtx[1,1]
      cx   = mtx[0,2]
      cy   = mtx[1,2]

      # manual correction , not a large adjustments
      print("Original MTX: \n", mtx,"\n", fx, fy,cx,cy)
      #fx,fy,cx,cy = fx*0.8, fy*0.8, 294,222
      fx,fy,cx,cy = fy*0.8, fy*0.8, 294,193
#      fx,fy,cx,cy = fx, fy, 340, 180

      self.distortion = dist[0]
      self.camera_params = (fx,fy,cx,cy)
      self.matrix = np.array([
            self.camera_params[0], 0, self.camera_params[2],
            0, self.camera_params[1], self.camera_params[3],
            0, 0, 1
            ]).reshape(3, 3)
#      print("Camera params: ", self.camera_params)
#      print("dist: ", self.distortion)
#      print("matrix: ", self.matrix)

      self.robot_position = [
                [1, 0, 0, 0.5],
                [0, -1, 0, 0],
                [0, 0, -1, 0],
                [0, 0, 0, 1]]

      self.camera_info = { "cameras" : [
        {
            "name": "USB Webcam",
            "type": "Small_USB_Camera",
            "port": 2,
            "robot_pose": [
                [1, 0, 0, 0.5],
                [0, -1, 0, 0],
                [0, 0, -1, 0],
                [0, 0, 0, 1]]
              }
           ]
        }
 
   # ----------------------------------------------------
   def read(self):
      ret, img = self.capture.read()

   # ----------------------------------------------------
   def start_reader(self, images_list=[], list_index=-1):
      ret, img = self.capture.read()
      if not ret:
#         images_list = ((ret, img), self)
         return ret

      height, width = img.shape[:2]
      new_mtx, roi = cv2.getOptimalNewCameraMatrix(self.matrix, self.distortion, (width, height), 1, (width, height))
#         print(new_mtx, roi)
      undistorted = cv2.undistort(img, new_mtx, self.distortion, None, self.matrix)
      crop_x, crop_y, crop_w, crop_h = roi
      undistorted = undistorted[crop_y:crop_y + crop_h, crop_x:crop_x + crop_w]

      #images_list[list_index] = ((ret, img), self)
      images_list = ((ret, img), self)
#      print(images_list)

      self.image = img
      return ret

   # -----------------------------------------------
   def show_image(self):
      cv2.imshow('img', self.image)
      cv2.waitKey(2000)
      cv2.destroyAllWindows()

   # ---------------------------------------------------
   def release(self):
      self.capture.release()


# ======================== End of Camera class==================================

#camera_obj = Camera()
#print (camera_obj.camera)
#print (camera_obj.camera_info)
#camera_obj.start_reader()
