from os import mkdir
mkdir("/tmp/disk/DIR1")
with open("/tmp/disk/DIR1/FILE", "w") as file:
	file.write("helloooooo")
	
mkdir("/tmp/disk/DIR2")
for i in range(100):
	with open(f"/tmp/disk/DIR2/FILE{i}", "w") as file:
		file.write(str(i))
