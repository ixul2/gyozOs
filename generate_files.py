from os import mkdir
mkdir("/tmp/disk/DIR1")
with open("/tmp/disk/DIR1/FILE", "w") as file:
	file.write("helloooooo")
	
mkdir("/tmp/disk/DIR2")
for i in range(257):
	with open(f"/tmp/disk/DIR2/FILE{i}", "w") as file:
		file.write("tesa"*1000*(i%10))

mkdir("/tmp/disk/DIR3")
for i in range(255):
	with open(f"/tmp/disk/DIR3/FILE{i}", "w") as file:
		file.write("tesa"*1000*(i%10))
