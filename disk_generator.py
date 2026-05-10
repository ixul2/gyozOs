from pyfatfs.PyFat import PyFat
from pyfatfs.PyFatFS import PyFatFS
import os

IMG = "external_disk.img"

# remove old image
if os.path.exists(IMG):
    os.remove(IMG)

# create empty backing file
with open(IMG, "wb") as f:
    f.truncate(64 * 1024 * 1024)

# create FAT32 filesystem
pf = PyFat()
pf.mkfs(IMG, fat_type=32)
pf.close()

# reopen via filesystem API
fs = PyFatFS(IMG)

fs.makedir("/docs")

with fs.open("/hello.txt", "w") as f:
    f.write("hello world")

with fs.open("/docs/test.txt", "w") as f:
    f.write("fat32 test")

fs.close()
