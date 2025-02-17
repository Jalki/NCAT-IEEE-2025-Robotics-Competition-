from moms_apriltag import TagGenerator2
from matplotlib import pyplot as plt

tg = TagGenerator2("tag36h11")
tag = tg.generate(1)

plt.imshow(tag, cmap="gray")
plt.savefig('tag36h11_01.png')