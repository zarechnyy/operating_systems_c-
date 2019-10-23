//
//  main.cpp
//  test3
//
//  Created by Yaroslav Zarechnyy on 10/20/19.
//  Copyright © 2019 Yaroslav Zarechnyy. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <string>
#include <limits.h>
#include <vector>
#include <map>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <math.h>

//#define DIAGRAM_STEP 10
#define SIZE_STEP 1024

int size_step = 1024;
int number_of_stars = 80;
std::map<long, int> statistic;

void read_directory (std::string dirName) {
    DIR* dir = NULL;
    struct dirent entry;
    struct dirent *entryPointer = NULL;
    char pathName[PATH_MAX];

    dir = opendir(dirName.c_str());
    if (dir == NULL){
        printf("Error opening %s\n", dirName.c_str());
    }

    readdir_r(dir, &entry, &entryPointer);
    while (entryPointer != NULL) {
        struct stat entryInfo;

        int compare1 = strncmp(entry.d_name, ".", PATH_MAX);
        int compare2 = strncmp(entry.d_name, "..", PATH_MAX);
        if (compare1 == 0 || compare2 == 0) { // ignore . and .. folders
            readdir_r(dir, &entry, &entryPointer); //next file
            continue;
        }

        strncpy(pathName, dirName.c_str(), PATH_MAX);
        strncat(pathName, "/", PATH_MAX);
        strncat(pathName, entry.d_name, PATH_MAX);

        if (lstat(pathName, &entryInfo) == 0) {
            if (S_ISDIR(entryInfo.st_mode)){ //check if its a folder
                std::string dirName(pathName);
                read_directory(dirName);
            }else if (S_ISREG(entryInfo.st_mode)) { //check if its a simple file
                if (statistic.find((int)entryInfo.st_size / size_step) != statistic.end()) {
                    statistic[(int)entryInfo.st_size /size_step]++;
                }else {
                    statistic.insert(std::pair<long, int>((int)entryInfo.st_size / size_step, 1));
                }
            }
        }else {
            printf("Error statting %s", pathName);
        }
        // next file
        readdir_r(dir, &entry, &entryPointer);
    }
    closedir(dir);
}

int main(int argc, char* argv[]){

    std::string my_directory = "/Users/yaroslavzarechnyy/Desktop/";
    std::string my_directory1 = "/Users/yaroslavzarechnyy/Desktop/test";
    
    std::cout << "Enter size step" << std::endl;
    std::cin >> size_step;

    read_directory(my_directory);
    
    auto x = std::max_element(statistic.begin(), statistic.end(),
                              [](const std::pair<int, int>& p1, const std::pair<int, int>& p2) {
                                  return p1.second < p2.second; });

    std::map<long, int>::iterator at = statistic.begin();

    for (; at != statistic.end(); ++at) {
        if (at->second == 0){
            continue;
        }
        printf("%ld - %ld\n", at->first * size_step, (at->first + 1) * size_step);
        printf("[");
        double count_of_stars = double(number_of_stars) * at->second / x->second;
        count_of_stars = ceil(count_of_stars);
        for (int j = 0; j < count_of_stars; j += 1) {
            printf("*");
        }
        printf("]\n");
    }

    return 0;
}
