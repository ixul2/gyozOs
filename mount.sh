cp external_disk.img2 external_disk.img; make
var=$(sudo losetup --show -Pf external_disk.img)
part="p1"
file="$var$part"
sudo mount $file /tmp/disk
gnome-terminal -- bash -c "cd /tmp/disk; exec bash"
read
sudo umount /tmp/disk
sudo losetup -d $var
