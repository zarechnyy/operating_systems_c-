//
//  main.cpp
//  lab3_ls
//
//  Created by Yaroslav Zarechnyy on 10/21/19.
//  Copyright © 2019 Yaroslav Zarechnyy. All rights reserved.
//

#include <string.h>
#include <vector>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h> //chdir
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

struct FileInfo{
    std::string name;
    mode_t mode;
    std::string uid;
    std::string gid;
    off_t size;
    time_t mtim;
    
    void Print(){
        printf((S_ISDIR(mode)) ? "d" : "-");
        printf((mode & S_IRUSR) ? "r" : "-");
        printf((mode & S_IWUSR) ? "w" : "-");
        printf((mode & S_IXUSR) ? "x" : "-");
        printf((mode & S_IRGRP) ? "r" : "-");
        printf((mode & S_IWGRP) ? "w" : "-");
        printf((mode & S_IXGRP) ? "x" : "-");
        printf((mode & S_IROTH) ? "r" : "-");
        printf((mode & S_IWOTH) ? "w" : "-");
        printf((mode & S_IXOTH) ? "x" : "-");
        printf("  %7s   %7s   ", uid.c_str(), gid.c_str());
        time ( &mtim);
        
        printf("  %7d  ", (int)size);
        printf("  %30s  ", name.c_str());
        printf ( "  %s" , ctime (&mtim) );
    }
};


void FindFiles(char dir[256], std::vector<FileInfo>& files)
{
    DIR *cdir;
    dirent *direntry; // struct
    struct stat fileinfo;
    chdir(dir); //change working directory
    cdir=opendir(dir);
    if(cdir!=NULL)
    {
        while((direntry=readdir(cdir))!=NULL)
        {
            if(lstat(direntry->d_name,&fileinfo)!=0){
                continue;
            }
            FileInfo tempDir;
            tempDir.name = direntry->d_name;
            tempDir.mode = fileinfo.st_mode;
            struct passwd *pw = getpwuid(fileinfo.st_uid);
            struct group  *gr = getgrgid(fileinfo.st_gid);
            tempDir.uid = std::string(pw->pw_name);
            tempDir.gid = std::string(gr->gr_name);
            tempDir.size = fileinfo.st_size;
            tempDir.mtim = fileinfo.st_mtime;
            
            files.push_back(tempDir);
        }
    }
}

void PrintAll(std::vector<FileInfo>& files){
    std::vector<FileInfo>::iterator at = files.begin();
    for (; at != files.end(); at++){
        at->Print();
    }
}

int main(int argc, char* argv[]){
    std::vector<FileInfo> names;
    std::vector<std::string> folders;
    
    char directory[256] = "/Users/yaroslavzarechnyy/Desktop/test";
    
//    std::cout << "Write path to directory" << std::endl;
//    std::cin >> directory;
    
    FindFiles((char *)directory, names);
    
    PrintAll(names);
    
    return 0;
}

