from PIL import Image
from glob import glob

# Obtain all raw images transferred by FireBeetle ESP32 as text (.txt) files.
path = "<_enter_path_>\\weather_station_data_center\\env_notifications"
images = glob(path + "/*.txt")

# Convert each text (TXT) file to a JPG file and save the generated JPG files to the images folder.
for img in images:
    loc = path + "/images/" + img.split("\\")[8].split(".")[0] + ".jpg"
    raw = open(img, 'rb').read()
    size = (320,240)
    file = Image.frombuffer('L', size, raw, 'raw', 'L', 0, 1)
    file.save(loc)
    #print("Converted: " + loc)