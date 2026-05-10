typedef struct drive_info drive_info;
struct drive_info{
	int channel;
	int drive;
};

void setupDrive(void);
