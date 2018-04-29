/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * RecoverySystem contains methods for interacting with the Android
 * recovery system (the separate partition that can be used to install
 * system updates, wipe user data, etc.)
 */

//#include <direct.h>
#include <linux/reboot.h>
//#include <io.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define LOG_FILE_LEN 512
#if 0
#define RECOVERY_PATH "/tmp/recovery"                                         
#define LOG_FILE_PATH "/tmp/recovery/log"
#define COMMAND_FILE_PATH "/tmp/recovery/command"
#else
#define RECOVERY_PATH "/cache/recovery"
#define LOG_FILE_PATH "/cache/recovery/log"
#define COMMAND_FILE_PATH "/cache/recovery/command"
#endif
#define SD_UPDATE_FILE "/sdcard/update.img"
#define DATA_UPDATE_FILE "/data/update.img"
/**
 * Reboot into the recovery system with the supplied argument.
 * @param arg to pass to the recovery utility.
 */
static void bootCommand(char *arg){
	FILE *command_file;
	FILE *log_file;
	char blank[LOG_FILE_LEN];
	
	if(!arg) return;
	mkdir(RECOVERY_PATH,0775); 
	if((command_file = fopen(COMMAND_FILE_PATH,"wb")) == NULL){
 		printf("Open command file error.\n");
		return;
 	}

 	if((log_file = fopen(LOG_FILE_PATH,"wb")) == NULL){
 		printf("Open log file error.\n");
		return;
	}
 
	printf("update: write command: ");			
 	fwrite(arg, strlen(arg), 1, command_file);
 	fwrite("\n", 1, 1, command_file);
 	fclose(command_file);
 	printf("done\n");
 	memset(blank, 0, LOG_FILE_LEN);
 	fwrite(blank, LOG_FILE_LEN, 1, log_file);
 	fclose(log_file);
	printf("update: reboot!\n");
 	//reboot(LINUX_REBOOT_MAGIC1, LINUX_REBOOT_MAGIC2,
	//       LINUX_REBOOT_CMD_RESTART2, "recovery");
 		
	return;
}


 /**
 * Reboots the device in order to install the given update
 * package.
 * Requires the {@link android.Manifest.permission#REBOOT} permission.
 *
 * @param packageFile  the update package to install.  Must be on
 * a partition mountable by recovery.  (The set of partitions
 * known to recovery may vary from device to device.  Generally,
 * /cache and /data are safe.)
 */
static void installPackage(char *update_file){
	char arg[512];
	char *str_update_package = "--update_package=";
	int str_update_package_len = strlen(str_update_package);
	int str_update_file_len = strlen(update_file);
	
	memset(arg, 0, 512);
	strcpy(arg, str_update_package);
	strcpy(arg + str_update_package_len, update_file);
	arg[str_update_package_len + str_update_file_len] = 0;
        bootCommand(arg);
}

static void sdUpdate(){
	installPackage(SD_UPDATE_FILE);
}

static void cacheUpdate(){
	installPackage(DATA_UPDATE_FILE);
}

/**
 * Reboots the device and wipes the user data partition.  This is
 * sometimes called a "factory reset", which is something of a
 * misnomer because the system partition is not restored to its
 * factory state.
 * Requires the {@link android.Manifest.permission#REBOOT} permission.
 *
 * @param context  the Context to use
 *
 */
void rebootWipeUserData(){
	printf("update: --wipe_data\n");
        bootCommand("--wipe_data");
}

int rebootUpdate(char *path){
	
	if(path){
		printf("find %s\n", path);                                 
        	installPackage(path);
	}

	if(access(DATA_UPDATE_FILE,F_OK) == -1){
		printf("%s does not exist! try to use %s to update\n",
			DATA_UPDATE_FILE, SD_UPDATE_FILE);
		if(access(SD_UPDATE_FILE,F_OK) == -1){
			printf("%s does not exist!\n", SD_UPDATE_FILE);
			return -1;
		}
		printf("find %s\n", SD_UPDATE_FILE);
		installPackage(SD_UPDATE_FILE);
		return 0;
	}
	
	printf("find %s\n", DATA_UPDATE_FILE);
	installPackage(DATA_UPDATE_FILE);
	return 0;
}

int main(int argc, char** argv){

	printf("update: Rockchip Update Tool\n");
	if(argc == 1) {
		rebootWipeUserData();
	} else if(argc == 2){
		if(!strcmp(argv[1], "ota") || !strcmp(argv[1], "update"))
			rebootUpdate(0);
		else if(!strcmp(argv[1], "factory") || !strcmp(argv[1], "reset"))
			rebootWipeUserData(); 
		else  return -1;
			
		return 0;
			
	} else if(argc == 3){
		if(!strcmp(argv[1], "ota") || !strcmp(argv[1], "update"))
			if(argv[2])
				return rebootUpdate(argv[2]);
	}

	return -1;
}

