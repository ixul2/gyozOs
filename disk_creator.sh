size=$1

image=external_disk.img

rm $image
fallocate -l 10000000 $image
fdisk $image << EOF
n
p
1
8192

t
c
w
EOF

l=$(sudo losetup -f)
echo $l

sudo losetup $l -o 4194304 $image
sudo mkfs.vfat $l -n RECOVERY
sleep 1
#mkdir -p /mnt/recovery
#sudo mount $l /mnt/recovery
#cp file.txt /mnt/recovery/
#sync
sudo umount /mnt/recovery
sudo losetup -d $l
sleep 1
