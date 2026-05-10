from os import mkdir
mkdir("/tmp/disk/dir1")
with open("/tmp/disk/dir1/file", "w") as file:
	file.write("helloooooo")
	
mkdir("/tmp/disk/dir2")
for i in range(257):
	with open(f"/tmp/disk/dir2/file{i}", "w") as file:
		file.write("tesa"*1000*(i%10))

mkdir("/tmp/disk/dir3")
for i in range(255):
	with open(f"/tmp/disk/dir3/file{i}", "w") as file:
		file.write("tesa"*1000*(i%10))
