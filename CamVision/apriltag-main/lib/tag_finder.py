import mycamera, apriltag, cv2, math, json
import numpy as np
import dt_apriltags
from transforms3d.euler import mat2euler

# ======================== Camera class==================================
class Detector:
   def __init__(self, tag_size, transform_file='/home/SECON2025/Downloads/apriltag-main/test_find_tags/test_tag_transform.json'):
      self.tag_size = tag_size
      self.camera_obj = mycamera.Camera()
      options = apriltag.DetectorOptions(families="tag36h11") 
      self.detector = apriltag.Detector(options)
      self.dt_detector = dt_apriltags.Detector( searchpath=['apriltags3py/apriltags/lib', 'apriltags3py/apriltags/lib'],nthreads=3, quad_decimate=2, quad_sigma=0.4, refine_edges=1, decode_sharpening=1, debug=0)
      if self.camera_obj.start_reader():
         self.img = self.camera_obj.image
         self.camera_width,self.camera_height = self.img.shape[1],self.img.shape[0]
      f = open(transform_file)
      self.tag_transform = json.load(f)
      f.close()

   # --------------------------------------------------------
   def capture_Camera(self):
      self.camera_obj.read()
      self.camera_obj.read()
      self.camera_obj.read()
      self.camera_obj.read()
      if self.camera_obj.start_reader():
         self.img = self.camera_obj.image
   # --------------------------------------------------------
   def release_Camera(self):
      self.camera_obj.release()

   # --------------------------------------------------------
   def getPose(self):
      self.gray = cv2.cvtColor(self.img, cv2.COLOR_BGR2GRAY)
      self.dt_results = self.dt_detector.detect(self.gray,True,self.camera_obj.camera_params, self.tag_size)

      self.Yaw, self.Pitch, self.Roll,self.Translation = [],[],[],[]
      self.Poses = []

      for result in self.dt_results:
         pose = {}
         radian, degree = self.get_Euler(result.pose_R)
         #print("R matrix: ", result.pose_t)
         X,Y,Z = result.pose_t*1000.0
         # Z is calibrated
         position = (X[0],Y[0],Z[0]*0.94)

         pose['translation'] = position
         pose['degree']      = degree
         pose['radian']      = radian
         pose['tag_family'] = result.tag_family.decode("utf-8")
         pose['tag_id']     = result.tag_id

         motorX = X[0] - Z[0]*math.sin(radian[1])
         motorY = Y[0] + Z[0]*math.sin(radian[0])
         motorZ = Z[0]*math.cos(radian[1]) - X[0]*math.sin(radian[1])
         pose['motor_translation'] = [motorX, motorY, motorZ]

         # transform matrix is Ax + B
         Amatrix = self.tag_transform[pose['tag_family']][str(pose['tag_id'])]['Amatrix']
#         print(pose['motor_translation'])
         pose['transform'] = np.dot(Amatrix,pose['motor_translation'])
         print(pose['transform'])
 
         self.Poses.append(pose)

#      self.getCamera_Pose(self.gray)
#      print("Pose: ", self.Poses)
      return (len(self.dt_results)>0)

   # --------------------------------------------------------
   def getCamera_Pose(self, result):
      self.pose, e0, e1 = self.detector.detection_pose(result, self.camera_obj.camera_params)
#     self.draw_Cube(1)
      np_pose = np.array(self.pose)
         # Find where the camera is if the tag is at the origin
      camera_pose = np.linalg.inv(self.pose) # relative to the ta
  #   camera_pose = invert(self.pose) # relative to the ta
      camera_pose[0][3] *= self.tag_size * 1000
      camera_pose[1][3] *= self.tag_size * 1000
      camera_pose[2][3] *= self.tag_size * 1000
      self.camera_X = camera_pose[0][3]
      self.camera_Y = camera_pose[1][3] 
      self.camera_Z = camera_pose[2][3] 
#     print("Relative to Camera:\n", camera_pose)

   # --------------------------------------------------------
   def get_Euler(self, Rmatrix):
      radian = np.array(mat2euler(Rmatrix[0:3][0:3], 'sxyz')) 
      degree = np.array(np.rad2deg(mat2euler(Rmatrix[0:3][0:3], 'sxyz'))) 
      # Pitch, Yaw, and Roll
      return radian, degree

   # --------------------------------------------------------
   def get_All_Tag_Info(self):
      X,Y,Z = self.X, self.Y, self.Z
      yaw,pitch,roll = self.Yaw[0],self.Pitch[0],self.Roll[0]
      return self.tag_family, self.tag_id, X,Y,Z, yaw,pitch,roll

   # --------------------------------------------------------
   def showImage(self, image, length, scale=1):
      try:
         resize_picture = cv2.resize(image,(image.shape[1]*scale, image.shape[0]*scale))
         #cv2.imshow('img', image)
         cv2.imshow('img', resize_picture)
      except:
         pass
      keypress = cv2.waitKey(length)
      return keypress

   # --------------------------------------------------------
   def destroyAllWindows(self):
      cv2.destroyAllWindows()

# ======================== End of Camera class=========================


#camera_obj = mycamera.Camera()
#print (camera_obj.camera)
#print (camera_obj.camera_info)
#camera_obj.start_reader()
