import apriltag, cv2, imutils, math, sys
import numpy as np

# ================ Camera Calibration =============

class Camera_Calibration:

   def __init__(self):
      self.camera = cv2.VideoCapture(0)
      # termination criteria
      self.criteria = (cv2.TERM_CRITERIA_EPS + cv2.TERM_CRITERIA_MAX_ITER, 30, 0.001)
      # prepare object points, like (0,0,0), (1,0,0), (2,0,0) ....,(6,5,0)
      self.objp = np.zeros((6*7,3), np.float32)
      self.objp[:,:2] = np.mgrid[0:7,0:6].T.reshape(-1,2)
#      print(self.objp)S
      # Arrays to store object points and image points from all the images.
      self.objpoints = [] # 3d point in real world space
      self.imgpoints = [] # 2d points in image plane.
      
   # ------------------------------------------------------------
   def capture_chessboard(self):
      ret, self.img = self.camera.read()
#      img = cv2.imread('left01.jpg')
      self.gray = cv2.cvtColor(self.img, cv2.COLOR_BGR2GRAY)
      # Find the chess board corners
      cret, corners = cv2.findChessboardCorners(self.gray, (7,6), None)

      if cret == True:
         self.objpoints.append(self.objp)
         corners2 = cv2.cornerSubPix(self.gray,corners, (11,11), (-1,-1), self.criteria)
         self.imgpoints.append(corners)
         cv2.drawChessboardCorners(self.img, (7,6), corners2, cret)
      cv2.imshow('img', self.img)
      cv2.waitKey(1000)
      self.camera.release()
      cv2.destroyAllWindows()
#      print("ret: ", cret, corners)

   # ------------------------------------------------------------
   def calibrate(self):
      ret, self.mtx, self.distort, self.rvecs, self.tvecs = cv2.calibrateCamera(self.objpoints, self.imgpoints, self.gray.shape[::-1], None, None)
      print ('ret: ', ret)
      print ('mtx:\n', self.mtx)
      print ('distort: ', self.distort)
      print ('rvecs: ', self.rvecs)
      print ('tvecs: ', self.tvecs)
      print ('Save the calibration data to calibration_savez.npz')
      try:
       np.savez('/home/SECON2025/Downloads/apriltag-main/camera_calibration/calibration_savez.npz', mtx=self.mtx, dist=self.distort, rvecs=self.rvecs, tvecs=self.tvecs)
       print('Data saved successfully!')
      except Exception as e:
       print(f"Error while saving: {e}")
      #np.savez('/home/SECON2025/Downloads/apriltag-main/camera_calibration/calibration_savez.npz', mtx=self.mtx, dist=self.distort,rvecs=self.rvecs,tvecs=self.tvecs)

   # ------------------------------------------------------------
   def undistort(self):
      print("In undistort")
      h, w = self.img.shape[:2]
      self.newcameramtx, self.roi=cv2.getOptimalNewCameraMatrix(self.mtx, self.distort, (w,h), 1, (w,h))
      # undistort
      undst = cv2.undistort(self.img, self.mtx, self.distort, None, self.newcameramtx)
      # crop the image
      x, y, self.w, self.h = self.roi
      undst = undst[y:y+self.h, x:x+self.w]
      # cv2.imwrite('calibresult.png', dst)
      cv2.imshow('undistorted', undst)
      cv2.waitKey(1000)
      self.camera.release()
      cv2.destroyAllWindows()

   # ------------------------------------------------------------
   def remapping(self):
      print("In remapping")
      # undistort
      mapx, mapy = cv2.initUndistortRectifyMap(self.mtx, self.distort, None, self.newcameramtx, (self.w,self.h), 5)
      dstort = cv2.remap(self.img, mapx, mapy, cv2.INTER_LINEAR)
      # crop the image
      x, y, w, h = self.roi
      dst = dstort[y:y+h, x:x+w]
      cv2.imshow('Remapping', dst)
      cv2.waitKey(0) 
      cv2.destroyAllWindows()
      

   # ------------------------------------------------------------
   def projection_Error(self):
      print("In projection")
      mean_error = 0
      for i in range(len(self.objpoints)):
         imgpoints2, _ = cv2.projectPoints(self.objpoints[i], self.rvecs[i], self.tvecs[i], self.mtx, self.distort)
         error = cv2.norm(self.imgpoints[i], imgpoints2, cv2.NORM_L2)/len(imgpoints2)
         mean_error += error
      print( "total error: {}".format(mean_error/len(self.objpoints)) )

   def clean_exit(self):
    print("In exit")
    print("Exiting program...")
    self.camera.release()  # Release the camera
    cv2.destroyAllWindows()  # Close any OpenCV windows
    sys.exit()  # Exit the program


# ================ Camera Calibration =============

calib_obj = Camera_Calibration()
calib_obj.capture_chessboard()
calib_obj.calibrate()
calib_obj.undistort()
calib_obj.remapping()
calib_obj.projection_Error()



