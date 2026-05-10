from os import mkdir
mkdir("mnt/recovery/dir1")
with open("mnt/recovery/dir1/file", "w") as file:
	file.write("helloooooo")
	
mkdir("mnt/recovery/dir2")
for i in range(257):
	with open(f"mnt/recovery/dir2/file{i}", "w") as file:
		file.write("tesa"*1000*i)

mkdir("mnt/recovery/dir3")
for i in range(255):
	with open(f"mnt/recovery/dir3/file{i}", "w") as file:
		file.write("tesa"*1000*i)
