import sys, time
sys.path.insert(1,'/home/SECON2025/Downloads/apriltag-main/lib')
import tag_finder, location, drawing, time

# ======================== Camera class==================================

def Run_eachPressKey(tagfinder_obj):
   tagfinder_obj.capture_Camera()
   #if not tagfinder_obj.getPose(): raise SystemExit("!!! Error: no tag found")
   if not tagfinder_obj.getPose(): 
      print("===== No tag found")
      time.sleep(1)
      return 0

   X,Y,Z = tagfinder_obj.X,tagfinder_obj.Y,tagfinder_obj.Z
   MotorX,MotorY,MotorZ = tagfinder_obj.motorX,tagfinder_obj.motorY,tagfinder_obj.motorZ
   yaw,pitch,roll = tagfinder_obj.Yaw[0],tagfinder_obj.Pitch[0],tagfinder_obj.Roll[0]

   draw_obj.img = tagfinder_obj.img
   draw_obj.annotate_Image(10,30,X,Y,Z,yaw,pitch,roll)
   #draw_obj.draw_Bounding_box()
   draw_obj.draw_Cube()

   print(" ==== Tag X=" , int(X),",Y=",int(Y),", Z=",int(Z))
   print(" ==== (Yaw,Pitch,Roll):", yaw,pitch,roll)
   print(" ==== Motor X=", MotorX, ", Y=", MotorY, "Z=", MotorZ)

#   tagfinder_obj.annotate_Image()
#   translate = tagfinder_obj.dt_results[0].pose_t
#   X,Y,Z = translate[0][0],translate[1][0], translate[2][0]
   radian, degree = tagfinder_obj.get_Euler(tagfinder_obj.dt_results[0].pose_R)
#   print(" ==== Euler degree (Pitch,Yaw,Roll):", degree)
#   print(" ==== Pose:\n", tagfinder_obj.pose)

   tagfinder_obj.get_Destination((-47,20))
   return  tagfinder_obj.showImage(tagfinder_obj.img,2000, 1)


#print (camera_obj.camera)
#print (camera_obj.camera_info)
#camera_obj.start_reader()
#locate_obj = location.Locator(tagfinder_obj.pose)

# ============================================

#tagfinder_obj = tag_finder.Detector(0.047)   # small tag
tagfinder_obj = tag_finder.Detector(0.160)   # small tag
draw_obj      = drawing.Draw(tagfinder_obj)

keypress = ord('*') # arbitrary

while keypress != ord('a'):
   keypress = Run_eachPressKey(tagfinder_obj)
   print("Keypress: ", keypress)

tagfinder_obj.release_Camera()
tagfinder_obj.destroyAllWindows()
