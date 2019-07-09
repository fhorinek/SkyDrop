
import os
from PIL import Image

def remap_img(data):
    n= []
    
    for y in range(48):
        for x in range(84):
            index = 84 * (y / 8) + x
            pos = y % 8
            n.append(not ord(data[index]) & (1 << pos))
            
    return n
           
pth = "in/"
raws = os.listdir(pth)
os.system("rm -rf out/*");

for raw in raws:
    print raw
    
    img = Image.new('1', (84, 48))
    
    f = open(pth + raw, "rb")
    data = f.read()
    f.close()
    
    data = remap_img(data)
    
    img.putdata(data)
    img.save("out/%s.png" % raw)
    
print "Converting to video"
os.system("rm -rf out.avi");
os.system("ffmpeg -r 10 -i out/%08d.png -q:v 1 out.avi")
print "done"
