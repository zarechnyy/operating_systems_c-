
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <pwd.h>
#include <iostream>
#include <sys/param.h>
#include <sys/mount.h>
#include <sys/vmmeter.h>
#include <libproc.h>
#include <mach/vm_statistics.h>
#include <mach/mach_types.h>
#include <mach/mach_init.h>
#include <mach/mach_host.h>
#include<mach/mach.h>
#include <mach/mach_init.h>
#include <mach/mach_error.h>
#include <mach/mach_host.h>
#include <mach/vm_map.h>
#include <unistd.h>
#include <sys/param.h>
//#include <proc/readproc.h>


using namespace std;

static unsigned long long _previousTotalTicks = 0;
static unsigned long long _previousIdleTicks = 0;


float CalculateCPULoad(unsigned long long idleTicks, unsigned long long totalTicks)
{
    unsigned long long totalTicksSinceLastTime = totalTicks-_previousTotalTicks;
    unsigned long long idleTicksSinceLastTime  = idleTicks-_previousIdleTicks;
    float ret = 1.0f-((totalTicksSinceLastTime > 0) ? ((float)idleTicksSinceLastTime)/totalTicksSinceLastTime : 0);
    _previousTotalTicks = totalTicks;
    _previousIdleTicks  = idleTicks;
    return ret;
}

float GetCPULoad()
{
    host_cpu_load_info_data_t cpuinfo;
    mach_msg_type_number_t count = HOST_CPU_LOAD_INFO_COUNT;
    if (host_statistics(mach_host_self(), HOST_CPU_LOAD_INFO, (host_info_t)&cpuinfo, &count) == KERN_SUCCESS)
    {
        unsigned long long totalTicks = 0;
        for(int i=0; i<CPU_STATE_MAX; i++) totalTicks += cpuinfo.cpu_ticks[i];
        return CalculateCPULoad(cpuinfo.cpu_ticks[CPU_STATE_SYSTEM], totalTicks);
//        return CalculateCPULoad(cpuinfo.cpu_ticks[CPU_STATE_IDLE], totalTicks);
    }
    else return -1.0f;
}




int parseLine(char* line){
    // This assumes that a digit will be found and the line ends in " Kb".
    int i = strlen(line);
    const char* p = line;
    while (*p <'0' || *p > '9') p++;
    line[i-3] = '\0';
    i = atoi(p);
    return i;
}


static double ParseMemValue(const char * b)
{
    while((*b)&&(isdigit(*b) == false)) b++;
    return isdigit(*b) ? atof(b) : -1.0;
}


size_t getMemorySize()
{
    int mib[2];
    mib[0] = CTL_HW;
    
    mib[1] = HW_MEMSIZE;            /* OSX. --------------------- */
    
    int64_t size = 0;               /* 64-bit */
    size_t len = sizeof( size );
    if (sysctl(mib, 2, &size, &len, NULL, 0) == 0)
        return (size_t)size;
    return 0L;
}

int main(void) {
    int err = 0;
    struct kinfo_proc *proc_list = NULL;
    size_t length = 0;
    
//    int pid_count = proc_listpids(PROC_ALL_PIDS, 0, nullptr, 0);
//    unsigned long pidsBufSize = sizeof(pid_t) * (unsigned long)pid_count;
//    pid_t * pids = (pid_t*)malloc(pidsBufSize);
//    bzero(pids, pidsBufSize);
//    proc_listpids(PROC_ALL_PIDS, 0, pids, (int)pidsBufSize);
//    char pathBuffer[PROC_PIDPATHINFO_MAXSIZE];
//    for (int i = 0; i < pid_count; i++) {
//        bzero(pathBuffer, PROC_PIDPATHINFO_MAXSIZE);
//
//        proc_pidpath(pids[i], pathBuffer, sizeof(pathBuffer));
//            printf("pid %d = %s\n",pids[i],pathBuffer);
//        proc_name(pids[i], pathBuffer, sizeof(pathBuffer));
//            printf("name = %s\n",pathBuffer);
//        rusage_info_current ru;
//        if (proc_pid_rusage(pids[i], RUSAGE_INFO_CURRENT, (void **)&ru) == 0) {
//            printf("name = %s\n",ru.ri_uuid);
//        }
//    }
    
    static const int name[] = { CTL_KERN, KERN_PROC, KERN_PROC_ALL, 0 };
    
    // Call sysctl with a NULL buffer to get proper length
    err = sysctl((int *)name, (sizeof(name) / sizeof(*name)) - 1, NULL, &length, NULL, 0);
    if (err) {
        perror(NULL);
        free(proc_list);
        return EXIT_FAILURE;
    }
    
    // Allocate buffer
    proc_list = (kinfo_proc *)malloc(length*sizeof(kinfo_proc));
    if (!proc_list) {
        perror(NULL);
        free(proc_list);
        return EXIT_FAILURE;
    }
    
    
    // Get the actual process list
    err = sysctl((int *)name, (sizeof(name) / sizeof(*name)) - 1, proc_list, &length, NULL, 0);
    if (err) {
        perror(NULL);
        free(proc_list);
        return EXIT_FAILURE;
    }
    
    
    int proc_count = int(length / sizeof(struct kinfo_proc));
    
    // use getpwuid_r() if you want to be thread-safe
    
    for (int i = 0; i < proc_count; i++) {
        uid_t uid = proc_list[i].kp_eproc.e_ucred.cr_uid;
        struct passwd *user = getpwuid(uid);
        const char *username = user ? user->pw_name : "user name not found";
        uid_t user_uid = user ? user->pw_uid : -100;
        
//        proc_t test;
        
//        struct task_basic_info t_info;
//        mach_msg_type_number_t t_info_count = TASK_BASIC_INFO_COUNT;
        
//        task_info(task_name_t target_task, <#task_flavor_t flavor#>, <#task_info_t task_info_out#>, <#mach_msg_type_number_t *task_info_outCnt#>)
//        if (KERN_SUCCESS != task_info(mach_task_self(),
//                                      TASK_BASIC_INFO, (task_info_t)&t_info,
//                                      &t_info_count))
//        {
//            return -1;
//        }
//        cout << "virtual_size: \t" << t_info.virtual_size << endl;

        
        cout << "PID: \t" << proc_list[i].kp_proc.p_pid << endl;
        cout << "process name: \t" << proc_list[i].kp_proc.p_comm << endl;
        cout << "username: \t" << username << endl;
        cout << "user id: \t" << user_uid << endl;
//        cout << "priority: \t" << int(proc_list[i].kp_proc.p_priority) << endl;
        cout << "parent PID: \t" << proc_list[i].kp_eproc.e_ppid << endl;
        cout << "status: \t" << int(proc_list[i].kp_proc.p_stat) << endl;
//        p_priority p_stat p_pctcpu
        cout << "process priority: \t" << int(proc_list[i].kp_proc.p_priority) << endl;
        cout << "cpu usage: \t" << int(proc_list[i].kp_proc.p_cpticks) << endl;
//        cout << "cpu usage: \t" << int(proc_list[i].kp_proc.p_pctcpu) << endl;

        cout << "ram usage: \t" << int(proc_list[i].kp_eproc.e_xsize) << endl;

//        e_xsize
//        printf("ID: %d - NAME: %s  CPU TIME: %d     \n",proc_list[i].kp_proc.p_pid, proc_list[i].kp_proc.p_comm, proc_list[i].kp_proc.p_pid );

        cout << "CPU: \t" << int(proc_list[i].kp_proc.p_estcpu) << endl;
        cout << "process group id: \t" << proc_list[i].kp_eproc.e_pgid << endl;

//        cout << "smthasdda: \t" << proc_list[i].kp_proc.p_rtime.tv_sec << endl;
        cout << endl;
    }
//    p_estcpu
    free(proc_list);
    
    cout << endl << "Physical memory: " << getMemorySize()/1048576 << " Mb" << endl;
    
    
    int mib1[2];
    mib1[0] = CTL_HW;
    
    mib1[1] = HW_NCPU;
    
    int cpu_numbers;
    size_t len1 = sizeof(cpu_numbers);
    if (sysctl(mib1, 2, &cpu_numbers, &len1, NULL, 0) == 0)
        cout << "CPU number: " << cpu_numbers << endl;
    
    int mib2[2];
    mib2[0] = CTL_HW;
    
    mib2[1] = HW_PAGESIZE;
    
    int page_size;
    size_t len2 = sizeof(page_size);
    if (sysctl(mib2, 2, &page_size, &len2, NULL, 0) == 0)
        cout << "Software page size: " << page_size << endl;
    
    
     int mib6[2];
     mib6[0] = CTL_KERN;

     mib6[1] = KERN_CLOCKRATE;

     clockinfo smth;
     size_t len6 = sizeof(smth);
     if (sysctl(mib6, 2, &smth, &len6, NULL, 0) == 0)
     cout << "KERN_CLOCKRATE: " << smth.hz << endl;
    
    
    int mib3[2];
    mib3[0] = CTL_KERN;
    
    mib3[1] = KERN_MAXPROC;
    
    int maxproc;
    size_t len3 = sizeof(maxproc);
    if (sysctl(mib3, 2, &maxproc, &len3, NULL, 0) == 0)
        cout << "MAX Processes number: " << maxproc << endl;
    
    int mib4[2];
    mib4[0] = CTL_KERN;
    
    mib4[1] = KERN_OSTYPE;
    
    string OSType;
    size_t len4 = sizeof(OSType);
    if (sysctl(mib4, 2, &OSType, &len4, NULL, 0) == 0)
        cout << "OS Type: " << OSType << endl;
    
    
//    struct statfs stats;
//    if (0 == statfs("/", &stats))
//    {
//        unsigned long long myFreeSwap = (uint64_t)stats.f_bsize * stats.f_bfree;
//        cout << myFreeSwap;
//    }

//
//    FILE * file = popen("/usr/bin/vm_stat", "r");
//    int result = -1;
//    double pagesUsed = 0.0, totalPages = 0.0;
//    char buf[512];

//    while (fgets(buf, sizeof(buf), file) != NULL){
//        if (strncmp(line, "VmSize:", 5) == 0){
//            result = parseLine(line);
//            cout << result;
//            break;
//        }
        
        vm_size_t page_size_1;
        mach_port_t mach_port;
        mach_msg_type_number_t count;
        vm_statistics64_data_t vm_stats;
        
        mach_port = mach_host_self();
        count = sizeof(vm_stats) / sizeof(natural_t);
        if (KERN_SUCCESS == host_page_size(mach_port, &page_size_1) &&
            KERN_SUCCESS == host_statistics64(mach_port, HOST_VM_INFO,
                                              (host_info64_t)&vm_stats, &count))
        {
            long long free_memory = (int64_t)vm_stats.free_count * (int64_t)page_size;
            
            long long used_memory = ((int64_t)vm_stats.active_count +
                                     (int64_t)vm_stats.inactive_count +
                                     (int64_t)vm_stats.wire_count) *  (int64_t)page_size;
            printf("free memory: %lld\nused memory: %lld\n", free_memory, used_memory);
        }
        
    
    cout<< "CPU load: " << GetCPULoad();
        
//        if (strncmp(buf, "Pages", 5) == 0)
//        {
//            double val = ParseMemValue(buf);
//            if (val >= 0.0)
//            {
//                if ((strncmp(buf, "Pages wired", 11) == 0)||(strncmp(buf, "Pages active", 12) == 0)) pagesUsed += val;
//                totalPages += val;
//            }
//        }
//        else if (strncmp(buf, "Mach Virtual Memory Statistics", 30) != 0) break;  // Stop at "Translation Faults", we don't care about anything at or below that
//
//    }
//    fclose(file);

    
//     int mib5[2];
//     mib5[0] = CTL_VM;
//
//     mib5[1] = VM_METER;
//
//     string smth1;
//     size_t len5 = sizeof(smth);
//
//     if (sysctl(mib5, 2, &smth, &len5, NULL, 0) == 0)
//         cout << "smth: " << smth1 << endl;
//    else
//        cout << "Could not collect VM info, errno %d - %s", errno, strerror(errno);
    
//    int vmmib[2] = {CTL_VM,VM_METER};
//    struct vmtotal myVirtualMemoryInfo;
//    size_t vmlen = sizeof(myVirtualMemoryInfo);
//    if (sysctl(vmmib, 2, &myVirtualMemoryInfo, &vmlen, NULL, NULL) == -1) {
////        fountainLog(QLOG_ERR, "Could not collect VM info, errno %d - %s", errno, strerror(errno));
//        printf("failed");
//
//    } else {
//        int totalSWAP = myVirtualMemoryInfo.t_vm;
//        cout<< "VM: " << myVirtualMemoryInfo.t_vm << endl;
//        cout<< "totalSWAP: " << static_cast<double>(totalSWAP) / 1024.0 << endl;
//
////        totalSWAP = myVirtualMemoryInfo.t_vm;
////        totalSWAP = static_cast<double>(totalSWAP) / 1024.0;
//    }
//
    
    
    
    return EXIT_SUCCESS;
}

