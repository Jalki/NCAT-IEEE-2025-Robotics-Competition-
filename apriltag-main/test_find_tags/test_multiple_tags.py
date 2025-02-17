import sys, time
sys.path.insert(1,'/home/SECON2025/Downloads/apriltag-main/lib')
import tag_finder, location, drawing

# ======================== Camera class==================================

def Run_eachPressKey(tagfinder_obj):
   tagfinder_obj.capture_Camera()
   #if not tagfinder_obj.getPose(): raise SystemExit("!!! Error: no tag found")
   if not tagfinder_obj.getPose(): 
      print("===== No tag found")
      time.sleep(1)
      return 0

   draw_obj.img = tagfinder_obj.img
   for index, pose in enumerate(tagfinder_obj.Poses):
      (X,Y,Z) = pose['translation']
      (motX,motY,motZ) = pose['motor_translation']
      (pitch,yaw,roll) = pose['degree']
      Transform = pose['transform']
      print("Tag: family=", pose['tag_family'], " , ID=" , pose['tag_id'])
#      print(" ==== X=" , int(X),",Y=",int(Y),", Z=",int(Z), 
#      print(" ====Motor X=" , int(motX),",Y=",int(motY),", Z=",int(motZ), 
#           "(Yaw,Pitch,Roll):", int(yaw),int(pitch),int(roll))
      print("Transform XYZ = ", int(Transform[0]), " , ", int(Transform[1]),
                                " , ", int(Transform[2]))

   #MotorX,MotorY,MotorZ = tagfinder_obj.motorX,tagfinder_obj.motorY,tagfinder_obj.motorZ


      draw_obj.annotate_Image(10,30+index*390,X,Y,Z,yaw,pitch,roll)
      #draw_obj.draw_Bounding_box()
      draw_obj.draw_Cube()
      draw_obj.get_Destination((320,600), X, Y)

#   print(" ==== Motor X=", MotorX, ", Y=", MotorY, "Z=", MotorZ)


#   tagfinder_obj.get_Destination((-47,20))
   return  tagfinder_obj.showImage(tagfinder_obj.img,2000, 1)


#print (camera_obj.camera)
#print (camera_obj.camera_info)
#camera_obj.start_reader()
#locate_obj = location.Locator(tagfinder_obj.pose)

# ============================================

#tagfinder_obj = tag_finder.Detector(0.047)   # small tag
tagfinder_obj = tag_finder.Detector(0.160, '/home/SECON2025/Downloads/apriltag-main/test_find_tags/test_tag_transform.json')   # small tag
draw_obj      = drawing.Draw(tagfinder_obj)

keypress = ord('*') # arbitrary

while keypress != ord('a'):
   keypress = Run_eachPressKey(tagfinder_obj)

tagfinder_obj.release_Camera()
tagfinder_obj.destroyAllWindows()
