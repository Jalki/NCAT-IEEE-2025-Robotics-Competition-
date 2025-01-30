import sys
sys.path.insert(1,'/home/SECON2025/Downloads/apriltag-main/lib')
import tag_finder, location, drawing
import numpy as np

# ======================== Camera class==================================

def Run_eachPressKey(tagfinder_obj, draw_obj):
   tagfinder_obj.capture_Camera()
   if not tagfinder_obj.getPose(): 
      print("===== No tag found")
      return 0
#     raise SystemExit("!!! Error: no tag found")

   draw_obj.img = tagfinder_obj.img
   for index,(pose,result) in enumerate(zip(tagfinder_obj.Poses, tagfinder_obj.dt_results)):
      print("Tag: family=", pose['tag_family'], " , ID=" , pose['tag_id'])

      #camera from tag
      tagfinder_obj.getCamera_Pose(result)
      X,Y,Z = tagfinder_obj.camera_X, tagfinder_obj.camera_Y, tagfinder_obj.camera_Z
      print("Camera: ", X, Y, Z)

#   draw_obj.img = tagfinder_obj.img
      draw_obj.annotate_Image(10,30,X,Y,Z,0,0,0,(250,0,120))
#   draw_obj.annotate_Image(10,140,cam_X,cam_Y,cam_Z,yaw,pitch,roll,(0,255,0))
      draw_obj.draw_Cube()

   #X,Y,Z = translate[0][0],translate[1][0], translate[2][0]
   #print(" ==== Translation: (", X,",",Y,",",Z,")")
#   radian, degree = tagfinder_obj.get_Euler(tagfinder_obj.dt_results[0].pose_R)
   #print(" ==== Euler degree (Yaw,Pitch,Roll):", degree)
   #print(" ==== Pose:\n", tagfinder_obj.pose)

#   tagfinder_obj.get_Destination((-47,20))
      return  tagfinder_obj.showImage(tagfinder_obj.img,1000,1)
  # return 'a'

# ============================================


tagfinder_obj = tag_finder.Detector(0.047, '/home/SECON2025/Downloads/apriltag-main/test_find_tags/test_tag_transform.json')
draw_obj      = drawing.Draw(tagfinder_obj)

keypress = ord('*') # arbitrary

while keypress != ord('a'):
   keypress = Run_eachPressKey(tagfinder_obj, draw_obj)
tagfinder_obj.release_Camera()
tagfinder_obj.destroyAllWindows()
