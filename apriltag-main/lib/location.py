import numpy as np

# ==========================================================
class Locator:
   
   def __init__(self, pose):
      tag_size = 0.06 # 0.2032
      self.pose = pose
#      print('Pose\n', self.pose)
      self.pose[0][3] *= tag_size
      self.pose[1][3] *= tag_size
      self.pose[2][3] *= tag_size
#      print('Pose\n', self.pose)
      matrix = np.matrix(self.pose)
      pose_array = np.array(np.matrix.flatten(self.pose))
      #print("pose_array: ", self.pose[0][3])

      # Find where the camera is if the tag is at the origin
      self.tag_relative_camera_pose = self.invert(pose_array)  # invert matrix
      #print(self.get_tag_absolute_pose())
      # Find the camera position relative to the known tag position (4x4 tag_pose position)
      self.world_camera_pose = np.matmul(self.get_tag_absolute_pose(), self.tag_relative_camera_pose)

   # -------------------------------------------------------
   def invert(self, posearray):
      #m00, m01, m02, m03, m10, m11, m12, m13, m20, m21, m22, m23, m30, m31, m32, m33 = np.ravel(posearray)

      m00, m01, m02, m03, m10, m11, m12, m13, m20, m21, m22, m23, m30, m31, m32, m33 = posearray
      a2323 = m22 * m33 - m23 * m32
      a1323 = m21 * m33 - m23 * m31
      a1223 = m21 * m32 - m22 * m31
      a0323 = m20 * m33 - m23 * m30
      a0223 = m20 * m32 - m22 * m30
      a0123 = m20 * m31 - m21 * m30
      a2313 = m12 * m33 - m13 * m32
      a1313 = m11 * m33 - m13 * m31
      a1213 = m11 * m32 - m12 * m31
      a2312 = m12 * m23 - m13 * m22
      a1312 = m11 * m23 - m13 * m21
      a1212 = m11 * m22 - m12 * m21
      a0313 = m10 * m33 - m13 * m30
      a0213 = m10 * m32 - m12 * m30
      a0312 = m10 * m23 - m13 * m20
      a0212 = m10 * m22 - m12 * m20
      a0113 = m10 * m31 - m11 * m30
      a0112 = m10 * m21 - m11 * m20

      det = m00 * (m11 * a2323 - m12 * a1323 + m13 * a1223) \
		- m01 * (m10 * a2323 - m12 * a0323 + m13 * a0223) \
		+ m02 * (m10 * a1323 - m11 * a0323 + m13 * a0123) \
		- m03 * (m10 * a1223 - m11 * a0223 + m12 * a0123)
      det = 1 / det

      return np.array([ \
		det *  (m11 * a2323 - m12 * a1323 + m13 * a1223), \
		det * -(m01 * a2323 - m02 * a1323 + m03 * a1223), \
		det *  (m01 * a2313 - m02 * a1313 + m03 * a1213), \
		det * -(m01 * a2312 - m02 * a1312 + m03 * a1212), \
		det * -(m10 * a2323 - m12 * a0323 + m13 * a0223), \
		det *  (m00 * a2323 - m02 * a0323 + m03 * a0223), \
		det * -(m00 * a2313 - m02 * a0313 + m03 * a0213), \
		det *  (m00 * a2312 - m02 * a0312 + m03 * a0212), \
		det *  (m10 * a1323 - m11 * a0323 + m13 * a0123), \
		det * -(m00 * a1323 - m01 * a0323 + m03 * a0123), \
		det *  (m00 * a1313 - m01 * a0313 + m03 * a0113), \
		det * -(m00 * a1312 - m01 * a0312 + m03 * a0112), \
		det * -(m10 * a1223 - m11 * a0223 + m12 * a0123), \
		det *  (m00 * a1223 - m01 * a0223 + m02 * a0123), \
		det * -(m00 * a1213 - m01 * a0213 + m02 * a0113), \
		det *  (m00 * a1212 - m01 * a0212 + m02 * a0112)  \
	]).reshape(4, 4)

   # -------------------------------------------------------
   def get_tag_absolute_pose (self):
      pose_matrix = [[ 1, 0, 0, -7.24310],
                     [0, 1, 0, -1.26019],
                     [0, 0, 1, 0.46272],
                     [0, 0, 0, 1 ]]
      return np.array(pose_matrix)
