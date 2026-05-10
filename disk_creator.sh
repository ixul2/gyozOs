dd if=/dev/zero of=external_disk.img bs=1M count=50
sudo fdisk external_disk.img << EOF
o
n
p
1




t
c
w
EOF
sudo losetup -d /dev/loop36
echo "no problem"
var=$(sudo losetup --find --show --partscan external_disk.img)
sudo partprobe $var
part="p1"
file="$var$part"
sudo mkfs.fat -F 32 $file
mkdir -p /tmp/disk
sudo mount $file /tmp/disk
sudo python3 generate_files.py
sync
sudo umount /tmp/disk
sudo losetup -d $var