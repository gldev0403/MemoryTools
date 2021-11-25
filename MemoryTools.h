#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <pthread.h>
#include <cstdio>
#include <cstring>
#include <fstream>

struct MAPS {
    unsigned long addr;
    unsigned long taddr;
    struct MAPS *next;
};

struct RESULT {
    unsigned long addr;
    struct RESULT *next;
};

struct FREEZE {
    unsigned long addr;  //地址
    const char *value;   //值
    int type;            //类型
    struct FREEZE *next; //指向下一节点的指针
};

struct COORDINATE {
    unsigned long addr;
    float value_f;
    int value_i;
    int type;
    struct COORDINATE *next;
};

#define LEN sizeof(struct MAPS)
#define FRE sizeof(struct FREEZE)
typedef struct MAPS *PMAPS;    //存储maps的链表
typedef struct RESULT *PRES;   //存储结果的链表
typedef struct FREEZE *PFREEZE;//存储冻结的数据的链表
typedef struct COORDINATE *PCOORDINATE;//存储坐标的链表

typedef int TYPE;
typedef int RANGE;
typedef int COUNT;
typedef long int OFFSET;
typedef unsigned long ADDRESS;
typedef char PACKAGENAME;

enum type {
    DWORD,
    FLOAT
};

enum Range {
    ALL,        //所有内存
    B_BAD,      //B内存
    C_ALLOC,    //Ca内存
    C_BSS,      //Cb内存
    C_DATA,     //Cd内存
    C_HEAP,     //Ch内存
    JAVA_HEAP,  //Jh内存
    A_ANONMYOUS,//A内存
    CODE_SYSTEM,//Xs内存
    STACK,      //S内存
    ASHMEM      //As内存
};

PMAPS Res = NULL;//全局buff(保存数据的地方)

PFREEZE Pfreeze = NULL;//用于存储冻结的数据
PFREEZE pEnd = NULL;
PFREEZE pNew = NULL;
int FreezeCount = 0;//冻结数据个数
int Freeze = 0;//开关
int typeNum = -1;
pthread_t pth;
char Fbm[64];//包名
long int delay = 1000;//冻结延迟,默认1000us

int ResCount = 0;//结果数量
int MemorySearchRange = 0;

int gs;
pid_t pid = 0;

int initMemoryTools(const char *bm);//获取pid

int SetSearchRange(int type);//设置搜索范围
PMAPS readmaps(int type);

PMAPS readmaps_all();//读取maps文件
PMAPS readmaps_bad();//读取maps文件
PMAPS readmaps_c_alloc();//读取maps文件
PMAPS readmaps_c_bss();//读取maps文件
PMAPS readmaps_c_data();//读取maps文件
PMAPS readmaps_c_heap();//读取maps文件
PMAPS readmaps_java_heap();//读取maps文件
PMAPS readmaps_a_anonmyous();//读取maps文件
PMAPS readmaps_code_system();//读取maps文件
PMAPS readmaps_stack();//读取maps文件
PMAPS readmaps_ashmem();//读取maps文件

void BaseAddressSearch(char *value, int type, unsigned long BaseAddr);//基址搜索
PMAPS BaseAddressSearch_DWORD(int value, unsigned long BaseAddr, PMAPS pMap);//DWORD
PMAPS BaseAddressSearch_FLOAT(float value, unsigned long BaseAddr, PMAPS pMap);//FLOAT

void RangeMemorySearch(char *from_value, char *to_value, int type);//范围搜索
PMAPS RangeMemorySearch_DWORD(int from_value, int to_value, PMAPS pMap);//DWORD
PMAPS RangeMemorySearch_FLOAT(float from_value, float to_value, PMAPS pMap);//FLOAT

void MemorySearch(const char *value, int TYPE);//类型搜索,这里value需要传入一个地址
PMAPS MemorySearch_DWORD(int value, PMAPS pMap);  //内存搜索DWORD
PMAPS MemorySearch_FLOAT(float value, PMAPS pMap);  //内存搜索FLOAT

void MemoryOffset(const char *value, long int offset, int type);//搜索偏移
PMAPS MemoryOffset_DWORD(int value, long int offset, PMAPS pBuff);//搜索偏移DWORD
PMAPS MemoryOffset_FLOAT(float value, long int offset, PMAPS pBuff);//搜索偏移FLOAT

void RangeMemoryOffset(char *from_value, char *to_value, long int offset, int type);//范围偏移
PMAPS
RangeMemoryOffset_DWORD(int from_value, int to_value, long int offset, PMAPS pBuff);//搜索偏移DWORD
PMAPS
RangeMemoryOffset_FLOAT(float from_value, float to_value, long int offset, PMAPS pBuff);//搜索偏移FLOAT

void MemoryWrite(const char *value, long int offset, int type);    //内存写入
int MemoryWrite_DWORD(int value, PMAPS pBuff, long int offset);    //内存写入DWORD
int MemoryWrite_FLOAT(float value, PMAPS pBuff, long int offset);    //内存写入FLOAT

void *SearchAddress(unsigned long addr);//搜索地址中的值,返回一个指针
int WriteAddress(unsigned long addr, void *value, int type);//修改地址中的值

int GetResultCount();//获取Res个数

void BypassGameSafe();//绕过游戏保护
//void RecBypassGameSafe(char *bm);//解除(停止使用)
void PrintResults();//打印Res里面的内容(地址)
void ClearResults();//清除链表,释放空间
void ClearMaps(PMAPS pMap);//清空maps

int isapkinstalled(char *bm);//检测应用是否安装
int isapkrunning(char *bm);//检测应用是否运行
int killprocess(char *bm);//杀掉进程
char GetProcessState(char *bm);//获取进程状态
int uninstallapk(char *bm);//静默删除软件
int installapk(char *lj);//静默卸载软件
int PutDate();//输出系统日期
int GetDate(char *date);//获取系统时间

PMAPS GetResults();//获取结果,返回头指针
int AddFreezeItem_All(const char *Value, int type, long int offset);//冻结所有结果
int AddFreezeItem(unsigned long addr, const char *value, int type, long int offset);//增加冻结数据
int AddFreezeItem_DWORD(unsigned long addr, const char *value);//DWORD
int AddFreezeItem_FLOAT(unsigned long addr, const char *value);//FLOAT
int RemoveFreezeItem(long int addr);//清除固定冻结数据
int RemoveFreezeItem_All();//清空所有冻结数据
int StartFreeze();//开始冻结
int StopFreeze();//停止冻结
int SetFreezeDelay(long int De);//设置冻结延迟
int PrintFreezeItems();//打印冻结表

int initMemoryTools(const char *PackageName) {
    DIR *dir = NULL;
    struct dirent *ptr = NULL;
    FILE *fp = NULL;
    char filepath[256];            // 大小随意，能装下cmdline文件的路径即可
    char filetext[128];            // 大小随意，能装下要识别的命令行文本即可
    dir = opendir("/proc");        // 打开路径
    if (NULL != dir) {
        while ((ptr = readdir(dir)) != NULL)    // 循环读取路径下的每一个文件/文件夹
        {
// 如果读取到的是"."或者".."则跳过，读取到的不是文件夹名字也跳过
            if ((strcmp(ptr->d_name, ".") == 0) || (strcmp(ptr->d_name, "..") == 0))
                continue;
            if (ptr->d_type != DT_DIR)
                continue;
            sprintf(filepath, "/proc/%s/cmdline", ptr->d_name);    // 生成要读取的文件的路径
            fp = fopen(filepath, "r");    // 打开文件
            if (NULL != fp) {
                fgets(filetext, sizeof(filetext), fp);    // 读取文件
                if (strcmp(filetext, PackageName) == 0) {
//puts(filepath);
//printf("packagename:%s\n",filetext);
                    break;
                }
                fclose(fp);
            }
        }
    }
    if (readdir(dir) == NULL) {
//puts("Get pid fail");
        return 0;
    }
    pid = atoi(ptr->d_name);
    closedir(dir);    // 关闭路径
    return pid;
}

int SetSearchRange(TYPE type) {
    switch (type) {
        case ALL:
            MemorySearchRange = 0;
            break;
        case B_BAD:
            MemorySearchRange = 1;
            break;
        case C_ALLOC:
            MemorySearchRange = 2;
            break;
        case C_BSS:
            MemorySearchRange = 3;
            break;
        case C_DATA:
            MemorySearchRange = 4;
            break;
        case C_HEAP:
            MemorySearchRange = 5;
            break;
        case JAVA_HEAP:
            MemorySearchRange = 6;
            break;
        case A_ANONMYOUS:
            MemorySearchRange = 7;
            break;
        case CODE_SYSTEM:
            MemorySearchRange = 8;
            break;
        case STACK:
            MemorySearchRange = 9;
            break;
        case ASHMEM:
            MemorySearchRange = 10;
            break;
        default:
            printf("\033[32;1mYou Select A NULL Type!\n");
            break;
    }
    return 0;
}

PMAPS readmaps(TYPE type) {
    PMAPS pMap = NULL;
    switch (type) {
        case ALL:
            pMap = readmaps_all();
            break;
        case B_BAD:
            pMap = readmaps_bad();
            break;
        case C_ALLOC:
            pMap = readmaps_c_alloc();
            break;
        case C_BSS:
            pMap = readmaps_c_bss();
            break;
        case C_DATA:
            pMap = readmaps_c_data();
            break;
        case C_HEAP:
            pMap = readmaps_c_heap();
            break;
        case JAVA_HEAP:
            pMap = readmaps_java_heap();
            break;
        case A_ANONMYOUS:
            pMap = readmaps_a_anonmyous();
            break;
        case CODE_SYSTEM:
            pMap = readmaps_code_system();
            break;
        case STACK:
            pMap = readmaps_stack();
            break;
        case ASHMEM:
            pMap = readmaps_ashmem();
            break;
        default:
            break;
    }
    if (pMap == NULL) {
        return 0;
    }
    return pMap;
}

PMAPS readmaps_all() {
    PMAPS pHead = NULL;
    PMAPS pNew;
    PMAPS pEnd;
    pEnd = pNew = (PMAPS) malloc(LEN);
    FILE *fp;
    int i = 0, flag = 1;
    char lj[64], buff[256];
    sprintf(lj, "/proc/%d/maps", pid);
    fp = fopen(lj, "r");
    if (fp == NULL) {
        puts("分析失败");
        return NULL;
    }
    while (!feof(fp)) {
        fgets(buff, sizeof(buff), fp);//读取一行
        if (strstr(buff, "rw") != NULL && !feof(fp)) {
            sscanf(buff, "%lx-%lx", &pNew->addr, &pNew->taddr);
//这里使用lx是为了能成功读取特别长的地址
            flag = 1;
        } else {
            flag = 0;
        }
        if (flag == 1) {
            i++;
            if (i == 1) {
                pNew->next = NULL;
                pEnd = pNew;
                pHead = pNew;
            } else {
                pNew->next = NULL;
                pEnd->next = pNew;
                pEnd = pNew;
            }
            pNew = (PMAPS) malloc(LEN);//分配内存
        }
    }
    free(pNew);//将多余的空间释放
    fclose(fp);//关闭文件指针
    return pHead;
}

PMAPS readmaps_bad() {
    PMAPS pHead = NULL;
    PMAPS pNew = NULL;
    PMAPS pEnd = NULL;
    pEnd = pNew = (PMAPS) malloc(LEN);
    FILE *fp;
    int i = 0, flag = 1;
    char lj[64], buff[256];
    sprintf(lj, "/proc/%d/maps", pid);
    fp = fopen(lj, "r");
    if (fp == NULL) {
        puts("分析失败");
        return NULL;
    }
    while (!feof(fp)) {
        fgets(buff, sizeof(buff), fp);//读取一行
        if (strstr(buff, "rw") != NULL && !feof(fp) && strstr(buff, "kgsl-3d0")) {
            sscanf(buff, "%lx-%lx", &pNew->addr, &pNew->taddr);
//这里使用lx是为了能成功读取特别长的地址
            flag = 1;
        } else {
            flag = 0;
        }
        if (flag == 1) {
            i++;
            if (i == 1) {
                pNew->next = NULL;
                pEnd = pNew;
                pHead = pNew;
            } else {
                pNew->next = NULL;
                pEnd->next = pNew;
                pEnd = pNew;
            }
            pNew = (PMAPS) malloc(LEN);//分配内存
        }
    }
    free(pNew);//将多余的空间释放
    fclose(fp);//关闭文件指针
    return pHead;
}

PMAPS readmaps_c_alloc() {
    PMAPS pHead = NULL;
    PMAPS pNew = NULL;
    PMAPS pEnd = NULL;
    pEnd = pNew = (PMAPS) malloc(LEN);
    FILE *fp;
    int i = 0, flag = 1;
    char lj[64], buff[256];
    sprintf(lj, "/proc/%d/maps", pid);
    fp = fopen(lj, "r");
    if (fp == NULL) {
        puts("分析失败");
        return NULL;
    }
    while (!feof(fp)) {
        fgets(buff, sizeof(buff), fp);//读取一行
        if (strstr(buff, "rw") != NULL && !feof(fp) && strstr(buff, "[anon:libc_malloc]")) {
            sscanf(buff, "%lx-%lx", &pNew->addr, &pNew->taddr);
//这里使用lx是为了能成功读取特别长的地址
            flag = 1;
        } else {
            flag = 0;
        }
        if (flag == 1) {
            i++;
            if (i == 1) {
                pNew->next = NULL;
                pEnd = pNew;
                pHead = pNew;
            } else {
                pNew->next = NULL;
                pEnd->next = pNew;
                pEnd = pNew;
            }
            pNew = (PMAPS) malloc(LEN);//分配内存
        }
    }
    free(pNew);//将多余的空间释放
    fclose(fp);//关闭文件指针
    return pHead;
}

PMAPS readmaps_c_bss() {
    PMAPS pHead = NULL;
    PMAPS pNew = NULL;
    PMAPS pEnd = NULL;
    pEnd = pNew = (PMAPS) malloc(LEN);
    FILE *fp;
    int i = 0, flag = 1;
    char lj[64], buff[256];
    sprintf(lj, "/proc/%d/maps", pid);
    fp = fopen(lj, "r");
    if (fp == NULL) {
        puts("分析失败");
        return NULL;
    }
    while (!feof(fp)) {
        fgets(buff, sizeof(buff), fp);//读取一行
        if (strstr(buff, "rw") != NULL && !feof(fp) && strstr(buff, "[anon:.bss]")) {
            sscanf(buff, "%lx-%lx", &pNew->addr, &pNew->taddr);
//这里使用lx是为了能成功读取特别长的地址
            flag = 1;
        } else {
            flag = 0;
        }
        if (flag == 1) {
            i++;
            if (i == 1) {
                pNew->next = NULL;
                pEnd = pNew;
                pHead = pNew;
            } else {
                pNew->next = NULL;
                pEnd->next = pNew;
                pEnd = pNew;
            }
            pNew = (PMAPS) malloc(LEN);//分配内存
        }
    }
    free(pNew);//将多余的空间释放
    fclose(fp);//关闭文件指针
    return pHead;
}

PMAPS readmaps_c_data() {
    PMAPS pHead = NULL;
    PMAPS pNew = NULL;
    PMAPS pEnd = NULL;
    pEnd = pNew = (PMAPS) malloc(LEN);
    FILE *fp;
    int i = 0, flag = 1;
    char lj[64], buff[256];
    sprintf(lj, "/proc/%d/maps", pid);
    fp = fopen(lj, "r");
    if (fp == NULL) {
        puts("分析失败");
        return NULL;
    }
    while (!feof(fp)) {
        fgets(buff, sizeof(buff), fp);//读取一行
        if (strstr(buff, "rw") != NULL && !feof(fp) && strstr(buff, "/data/app/")) {
            sscanf(buff, "%lx-%lx", &pNew->addr, &pNew->taddr);
//这里使用lx是为了能成功读取特别长的地址
            flag = 1;
        } else {
            flag = 0;
        }
        if (flag == 1) {
            i++;
            if (i == 1) {
                pNew->next = NULL;
                pEnd = pNew;
                pHead = pNew;
            } else {
                pNew->next = NULL;
                pEnd->next = pNew;
                pEnd = pNew;
            }
            pNew = (PMAPS) malloc(LEN);//分配内存
        }
    }
    free(pNew);//将多余的空间释放
    fclose(fp);//关闭文件指针
    return pHead;
}

PMAPS readmaps_c_heap() {
    PMAPS pHead = NULL;
    PMAPS pNew = NULL;
    PMAPS pEnd = NULL;
    pEnd = pNew = (PMAPS) malloc(LEN);
    FILE *fp;
    int i = 0, flag = 1;
    char lj[64], buff[256];
    sprintf(lj, "/proc/%d/maps", pid);
    fp = fopen(lj, "r");
    if (fp == NULL) {
        puts("分析失败");
        return NULL;
    }
    while (!feof(fp)) {
        fgets(buff, sizeof(buff), fp);//读取一行
        if (strstr(buff, "rw") != NULL && !feof(fp) && strstr(buff, "[heap]")) {
            sscanf(buff, "%lx-%lx", &pNew->addr, &pNew->taddr);
//这里使用lx是为了能成功读取特别长的地址
            flag = 1;
        } else {
            flag = 0;
        }
        if (flag == 1) {
            i++;
            if (i == 1) {
                pNew->next = NULL;
                pEnd = pNew;
                pHead = pNew;
            } else {
                pNew->next = NULL;
                pEnd->next = pNew;
                pEnd = pNew;
            }
            pNew = (PMAPS) malloc(LEN);//分配内存
        }
    }
    free(pNew);//将多余的空间释放
    fclose(fp);//关闭文件指针
    return pHead;
}

PMAPS readmaps_java_heap() {
    PMAPS pHead = NULL;
    PMAPS pNew = NULL;
    PMAPS pEnd = NULL;
    pEnd = pNew = (PMAPS) malloc(LEN);
    FILE *fp;
    int i = 0, flag = 1;
    char lj[64], buff[256];
    sprintf(lj, "/proc/%d/maps", pid);
    fp = fopen(lj, "r");
    if (fp == NULL) {
        puts("分析失败");
        return NULL;
    }
    while (!feof(fp)) {
        fgets(buff, sizeof(buff), fp);//读取一行
        if (strstr(buff, "rw") != NULL && !feof(fp) && strstr(buff, "/dev/ashmem/")) {
            sscanf(buff, "%lx-%lx", &pNew->addr, &pNew->taddr);
//这里使用lx是为了能成功读取特别长的地址
            flag = 1;
        } else {
            flag = 0;
        }
        if (flag == 1) {
            i++;
            if (i == 1) {
                pNew->next = NULL;
                pEnd = pNew;
                pHead = pNew;
            } else {
                pNew->next = NULL;
                pEnd->next = pNew;
                pEnd = pNew;
            }
            pNew = (PMAPS) malloc(LEN);//分配内存
        }
    }
    free(pNew);//将多余的空间释放
    fclose(fp);//关闭文件指针
    return pHead;
}

PMAPS readmaps_a_anonmyous() {
    PMAPS pHead = NULL;
    PMAPS pNew = NULL;
    PMAPS pEnd = NULL;
    pEnd = pNew = (PMAPS) malloc(LEN);
    FILE *fp;
    int i = 0, flag = 1;
    char lj[64], buff[256];
    sprintf(lj, "/proc/%d/maps", pid);
    fp = fopen(lj, "r");
    if (fp == NULL) {
        puts("分析失败");
        return NULL;
    }
    while (!feof(fp)) {
        fgets(buff, sizeof(buff), fp);//读取一行
        if (strstr(buff, "rw") != NULL && !feof(fp) && (strlen(buff) < 42)) {
            sscanf(buff, "%lx-%lx", &pNew->addr, &pNew->taddr);
//这里使用lx是为了能成功读取特别长的地址
            flag = 1;
        } else {
            flag = 0;
        }
        if (flag == 1) {
            i++;
            if (i == 1) {
                pNew->next = NULL;
                pEnd = pNew;
                pHead = pNew;
            } else {
                pNew->next = NULL;
                pEnd->next = pNew;
                pEnd = pNew;
            }
            pNew = (PMAPS) malloc(LEN);//分配内存
        }
    }
    free(pNew);//将多余的空间释放
    fclose(fp);//关闭文件指针
    return pHead;
}

PMAPS readmaps_code_system() {
    PMAPS pHead = NULL;
    PMAPS pNew = NULL;
    PMAPS pEnd = NULL;
    pEnd = pNew = (PMAPS) malloc(LEN);
    FILE *fp;
    int i = 0, flag = 1;
    char lj[64], buff[256];
    sprintf(lj, "/proc/%d/maps", pid);
    fp = fopen(lj, "r");
    if (fp == NULL) {
        puts("分析失败");
        return NULL;
    }
    while (!feof(fp)) {
        fgets(buff, sizeof(buff), fp);//读取一行
        if (strstr(buff, "rw") != NULL && !feof(fp) && strstr(buff, "/system")) {
            sscanf(buff, "%lx-%lx", &pNew->addr, &pNew->taddr);
//这里使用lx是为了能成功读取特别长的地址
            flag = 1;
        } else {
            flag = 0;
        }
        if (flag == 1) {
            i++;
            if (i == 1) {
                pNew->next = NULL;
                pEnd = pNew;
                pHead = pNew;
            } else {
                pNew->next = NULL;
                pEnd->next = pNew;
                pEnd = pNew;
            }
            pNew = (PMAPS) malloc(LEN);//分配内存
        }
    }
    free(pNew);//将多余的空间释放
    fclose(fp);//关闭文件指针
    return pHead;
}

PMAPS readmaps_stack() {
    PMAPS pHead = NULL;
    PMAPS pNew = NULL;
    PMAPS pEnd = NULL;
    pEnd = pNew = (PMAPS) malloc(LEN);
    FILE *fp;
    int i = 0, flag = 1;
    char lj[64], buff[256];
    sprintf(lj, "/proc/%d/maps", pid);
    fp = fopen(lj, "r");
    if (fp == NULL) {
        puts("分析失败");
        return NULL;
    }
    while (!feof(fp)) {
        fgets(buff, sizeof(buff), fp);//读取一行
        if (strstr(buff, "rw") != NULL && !feof(fp) && strstr(buff, "[stack]")) {
            sscanf(buff, "%lx-%lx", &pNew->addr, &pNew->taddr);
//这里使用lx是为了能成功读取特别长的地址
            flag = 1;
        } else {
            flag = 0;
        }
        if (flag == 1) {
            i++;
            if (i == 1) {
                pNew->next = NULL;
                pEnd = pNew;
                pHead = pNew;
            } else {
                pNew->next = NULL;
                pEnd->next = pNew;
                pEnd = pNew;
            }
            pNew = (PMAPS) malloc(LEN);//分配内存
        }
    }
    free(pNew);//将多余的空间释放
    fclose(fp);//关闭文件指针
    return pHead;
}

PMAPS readmaps_ashmem() {
    PMAPS pHead = NULL;
    PMAPS pNew = NULL;
    PMAPS pEnd = NULL;
    pEnd = pNew = (PMAPS) malloc(LEN);
    FILE *fp;
    int i = 0, flag = 1;
    char lj[64], buff[256];
    sprintf(lj, "/proc/%d/maps", pid);
    fp = fopen(lj, "r");
    if (fp == NULL) {
        puts("分析失败");
        return NULL;
    }
    while (!feof(fp)) {
        fgets(buff, sizeof(buff), fp);//读取一行
        if (strstr(buff, "rw") != NULL && !feof(fp) && strstr(buff, "/dev/ashmem/") &&
            !strstr(buff, "dalvik")) {
            sscanf(buff, "%lx-%lx", &pNew->addr, &pNew->taddr);
//这里使用lx是为了能成功读取特别长的地址
            flag = 1;
        } else {
            flag = 0;
        }
        if (flag == 1) {
            i++;
            if (i == 1) {
                pNew->next = NULL;
                pEnd = pNew;
                pHead = pNew;
            } else {
                pNew->next = NULL;
                pEnd->next = pNew;
                pEnd = pNew;
            }
            pNew = (PMAPS) malloc(LEN);//分配内存
        }
    }
    free(pNew);//将多余的空间释放
    fclose(fp);//关闭文件指针
    return pHead;
}

void PrintResults() {
    PMAPS temp = Res;
    int i;
    int handle;
    char lj[64];//路径
    int *buf_D = (int *) malloc(sizeof(int));//缓冲区
    float *buf_F = (float *) malloc(sizeof(float));//缓冲区
    sprintf(lj, "/proc/%d/mem", pid);
    handle = open(lj, O_RDWR);
    lseek(handle, 0, SEEK_SET);

    for (i = 0; i < ResCount; i++) {
        if (typeNum == 0) {
            pread64(handle, buf_D, 4, temp->addr);
            printf("Address:0x%lX Value:%d Type:DWORD\n", temp->addr, *buf_D);
        } else if (typeNum == 1) {
            pread64(handle, buf_F, 4, temp->addr);
            printf("Address:0x%lX Value:%f Type:FLOAT\n", temp->addr, *buf_F);
        } else {
            printf("addr:0x%lX,taddr:%lX\n", temp->addr, temp->taddr);
        }
        temp = temp->next;//指向下一个节点
    }
}

void ClearResults()//清空
{
    PMAPS pHead = Res;
    PMAPS pTemp = pHead;
    int i;
    for (i = 0; i < ResCount; i++) {
        pTemp = pHead;
        pHead = pHead->next;
        free(pTemp);
    }
}

void BaseAddressSearch(char *value, TYPE type, ADDRESS BaseAddr) {
    PMAPS pHead = NULL;
    PMAPS pMap = NULL;
    switch (MemorySearchRange) {
        case ALL:
            pMap = readmaps(ALL);
            break;
        case B_BAD:
            pMap = readmaps(B_BAD);
            break;
        case C_ALLOC:
            pMap = readmaps(C_ALLOC);
            break;
        case C_BSS:
            pMap = readmaps(C_BSS);
            break;
        case C_DATA:
            pMap = readmaps(C_DATA);
            break;
        case C_HEAP:
            pMap = readmaps(C_HEAP);
            break;
        case JAVA_HEAP:
            pMap = readmaps(JAVA_HEAP);
            break;
        case A_ANONMYOUS:
            pMap = readmaps(A_ANONMYOUS);
            break;
        case CODE_SYSTEM:
            pMap = readmaps(CODE_SYSTEM);
            break;
        case STACK:
            pMap = readmaps(STACK);
            break;
        case ASHMEM:
            pMap = readmaps(ASHMEM);
            break;
        default:
            break;
    }
    if (pMap == NULL) {
        puts("map error");
        return (void) 0;
    }
    switch (type) {
        case DWORD:
            pHead = BaseAddressSearch_DWORD(atoi(value), BaseAddr, pMap);
            break;
        case FLOAT:
            pHead = BaseAddressSearch_FLOAT(atof(value), BaseAddr, pMap);
            break;
        default:
            printf("\033[32;1mYou Select A NULL Type!\n");
            break;
    }
    if (pHead == NULL) {
        puts("search error");
        return (void) 0;
    }
    ResCount = gs;
    Res = pHead;//Res指针指向链表
}

PMAPS BaseAddressSearch_DWORD(int value, ADDRESS BaseAddr, PMAPS pMap) {
    gs = 0;
//printf("BaseAddress:%lX\n",BaseAddr);
    if (pid == 0) {
        puts("can not get pid");
        return 0;
    }
    PMAPS e, n;
    e = n = (PMAPS) malloc(LEN);
    PMAPS pBuff = n;
    int iCount = 0;
    unsigned long c, ADDR;
    int handle;
    char lj[64];
    sprintf(lj, "/proc/%d/mem", pid);
    handle = open(lj, O_RDWR);//打开mem文件
    lseek(handle, 0, SEEK_SET);
    void *BUF[8];
    PMAPS pTemp = pMap;
    while (pTemp != NULL) {
        c = (pTemp->taddr - pTemp->addr) / 4096;
        for (int j = 0; j < c; j++) {
            ADDR = pTemp->addr + j * 4096 + BaseAddr;
            pread64(handle, BUF, 8, ADDR);
            if (*(int *) &BUF[0] == value) {
                iCount++;
                gs += 1;
                ResCount += 1;
                n->addr = ADDR;
//printf("addr:%lx,val:%d,buff=%d\n",n->addr,value,buff[i]);
                if (iCount == 1) {
                    n->next = NULL;
                    e = n;
                    pBuff = n;
                } else {
                    n->next = NULL;
                    e->next = n;
                    e = n;
                }
                n = (PMAPS) malloc(LEN);
            }
        }
        pTemp = pTemp->next;
    }
    close(handle);
    return pBuff;
}

PMAPS
BaseAddressSearch_FLOAT(float value, ADDRESS BaseAddr, PMAPS pMap) {
    gs = 0;
    if (pid == 0) {
        puts("can not get pid");
        return 0;
    }
    PMAPS e, n;
    e = n = (PMAPS) malloc(LEN);
    PMAPS pBuff = n;
    unsigned long c, ADDR;
    int handle;
    int iCount = 0;
    char lj[64];
    sprintf(lj, "/proc/%d/mem", pid);
    handle = open(lj, O_RDWR);//打开mem文件
    lseek(handle, 0, SEEK_SET);
    void *BUF[8];
    PMAPS pTemp = pMap;
    while (pTemp != NULL) {
        c = (pTemp->taddr - pTemp->addr) / 4096;
        for (int j = 0; j < c; j++) {
            ADDR = pTemp->addr + j * 4096 + BaseAddr;
            pread64(handle, BUF, 8, ADDR);
            if (*(float *) &BUF[0] == value) {
                iCount++;
                gs += 1;
                ResCount += 1;
                n->addr = ADDR;
//printf("addr:%lx,val:%d,buff=%d\n",n->addr,value,buff[i]);
                if (iCount == 1) {
                    n->next = NULL;
                    e = n;
                    pBuff = n;
                } else {
                    n->next = NULL;
                    e->next = n;
                    e = n;
                }
                n = (PMAPS) malloc(LEN);
            }
        }
        pTemp = pTemp->next;
    }
    close(handle);
    return pBuff;
}

void RangeMemorySearch(char *from_value, char *to_value, TYPE type)//范围搜索
{
    PMAPS pHead = NULL;
    PMAPS pMap = NULL;
    switch (MemorySearchRange) {
        case ALL:
            pMap = readmaps(ALL);
            break;
        case B_BAD:
            pMap = readmaps(B_BAD);
            break;
        case C_ALLOC:
            pMap = readmaps(C_ALLOC);
            break;
        case C_BSS:
            pMap = readmaps(C_BSS);
            break;
        case C_DATA:
            pMap = readmaps(C_DATA);
            break;
        case C_HEAP:
            pMap = readmaps(C_HEAP);
            break;
        case JAVA_HEAP:
            pMap = readmaps(JAVA_HEAP);
            break;
        case A_ANONMYOUS:
            pMap = readmaps(A_ANONMYOUS);
            break;
        case CODE_SYSTEM:
            pMap = readmaps(CODE_SYSTEM);
            break;
        case STACK:
            pMap = readmaps(STACK);
            break;
        case ASHMEM:
            pMap = readmaps(ASHMEM);
            break;
        default:
            printf("\033[32;1mYou Select A NULL Type!\n");
            break;
    }
    if (pMap == NULL) {
        puts("map error");
        return (void) 0;
    }
    switch (type) {
        case DWORD:
            if (atoi(from_value) > atoi(to_value))
                pHead = RangeMemorySearch_DWORD(atoi(to_value), atoi(from_value), pMap);
            else
                pHead = RangeMemorySearch_DWORD(atoi(from_value), atoi(to_value), pMap);
            break;
        case FLOAT:
            if (atof(from_value) > atof(to_value))
                pHead = RangeMemorySearch_FLOAT(atof(to_value), atof(from_value), pMap);
            else
                pHead = RangeMemorySearch_FLOAT(atof(from_value), atof(to_value), pMap);
            break;
        default:
            printf("\033[32;1mYou Select A NULL Type!\n");
            break;
    }
    if (pHead == NULL) {
        puts("RangeSearch Error");
        return (void) 0;
    }
    ResCount = gs;
    Res = pHead;//Res指针指向链表
}

PMAPS
RangeMemorySearch_DWORD(int from_value, int to_value, PMAPS pMap)//DWORD
{
    if (pid == 0) {
        puts("can not get pid");
        return NULL;
    }
    gs = 0;
    PMAPS pTemp = NULL;
    pTemp = pMap;
    PMAPS n, e;
    e = n = (PMAPS) malloc(LEN);
    PMAPS pBuff;
    pBuff = n;
    int handle;//句柄
    int iCount = 0;//链表长度
    int c;
    char lj[64];//路径
    int buff[1024] = {0};//缓冲区
    memset(buff, 0, 4);
    sprintf(lj, "/proc/%d/mem", pid);
    handle = open(lj, O_RDWR);//打开mem文件
    lseek(handle, 0, SEEK_SET);
    while (pTemp != NULL)//读取maps里面的地址
    {
        c = (pTemp->taddr - pTemp->addr) / 4096;
        for (int j = 0; j < c; j++) {
            pread64(handle, buff, 0x1000, pTemp->addr + j * 4096);
            for (int i = 0; i < 1024; i++) {
                if (buff[i] >= from_value && buff[i] <= to_value)//判断值是否在这两者之间
                {
                    iCount++;
                    gs += 1;
                    ResCount += 1;
                    n->addr = (pTemp->addr) + (j * 4096) + (i * 4);
                    if (iCount == 1) {
                        n->next = NULL;
                        e = n;
                        pBuff = n;
                    } else {
                        n->next = NULL;
                        e->next = n;
                        e = n;
                    }
                    n = (PMAPS) malloc(LEN);
                }
            }
        }
        pTemp = pTemp->next;
    }
    free(n);
    close(handle);
    return pBuff;
}

PMAPS RangeMemorySearch_FLOAT(float from_value, float to_value, PMAPS pMap)//FLOAT
{
    if (pid == 0) {
        puts("can not get pid");
        return NULL;
    }
    gs = 0;
    PMAPS pTemp = NULL;
    pTemp = pMap;
    PMAPS n, e;
    e = n = (PMAPS) malloc(LEN);
    PMAPS pBuff;
    pBuff = n;
    int handle;//句柄
    int iCount = 0;//链表长度
    int c;
    char lj[64];//路径
    float buff[1024] = {0};//缓冲区
    sprintf(lj, "/proc/%d/mem", pid);
    handle = open(lj, O_RDWR);//打开mem文件
    lseek(handle, 0, SEEK_SET);
    while (pTemp->next != NULL) {
        c = (pTemp->taddr - pTemp->addr) / 4096;
        for (int j = 0; j < c; j += 1) {
            pread64(handle, buff, 0x1000, pTemp->addr + (j * 4096));
            for (int i = 0; i < 1024; i += 1) {
                if (buff[i] >= from_value && buff[i] <= to_value)//判断。。。
                {
                    iCount++;
                    gs += 1;
                    ResCount += 1;
                    n->addr = (pTemp->addr) + (j * 4096) + (i * 4);
                    if (iCount == 1) {
                        n->next = NULL;
                        e = n;
                        pBuff = n;
                    } else {
                        n->next = NULL;
                        e->next = n;
                        e = n;
                    }
                    n = (PMAPS) malloc(LEN);
                }
//printf("buff[%d]=%f\n",l,buff[l]);
//usleep(1);
            }
//memset(buff,0,4);
        }
        pTemp = pTemp->next;
    }
    free(n);
    close(handle);
    return pBuff;
}

void MemorySearch(const char *value, TYPE type) {
    PMAPS pHead = NULL;
    PMAPS pMap = NULL;
    int fromValue_i, toValue_i;
    float fromValue_f, toValue_f;
    switch (MemorySearchRange) {
        case ALL:
            pMap = readmaps(ALL);
            break;
        case B_BAD:
            pMap = readmaps(B_BAD);
            break;
        case C_ALLOC:
            pMap = readmaps(C_ALLOC);
            break;
        case C_BSS:
            pMap = readmaps(C_BSS);
            break;
        case C_DATA:
            pMap = readmaps(C_DATA);
            break;
        case C_HEAP:
            pMap = readmaps(C_HEAP);
            break;
        case JAVA_HEAP:
            pMap = readmaps(JAVA_HEAP);
            break;
        case A_ANONMYOUS:
            pMap = readmaps(A_ANONMYOUS);
            break;
        case CODE_SYSTEM:
            pMap = readmaps(CODE_SYSTEM);
            break;
        case STACK:
            pMap = readmaps(STACK);
            break;
        case ASHMEM:
            pMap = readmaps(ASHMEM);
            break;
        default:
            printf("\033[32;1mYou Select A NULL Type!\n");
            break;
    }
    if (pMap == NULL) {
        puts("map error");
        return (void) 0;
    }
    switch (type) {
        case DWORD:
            typeNum = 0;
            if (strstr(value, "~") != NULL) {
                sscanf(value, "%d~%d", &fromValue_i, &toValue_i);
                if (fromValue_i > toValue_i)
                    pHead = RangeMemorySearch_DWORD(toValue_i, fromValue_i, pMap);
                else
                    pHead = RangeMemorySearch_DWORD(fromValue_i, toValue_i, pMap);
            } else {
                pHead = MemorySearch_DWORD(atoi(value), pMap);
            }
            break;
        case FLOAT:
            typeNum = 1;
            if (strstr(value, "~") != NULL) {
                sscanf(value, "%f~%f", &fromValue_f, &toValue_f);
                if (fromValue_f > toValue_f)
                    pHead = RangeMemorySearch_FLOAT(toValue_f, fromValue_f, pMap);
                else
                    pHead = RangeMemorySearch_FLOAT(fromValue_f, toValue_f, pMap);
            } else {
                pHead = MemorySearch_FLOAT(atof(value), pMap);
            }
            break;
        default:
            printf("\033[32;1mYou Select A NULL Type!\n");
            break;
    }
    if (pHead == NULL) {
        puts("search error");
        return (void) 0;
    }
    ResCount = gs;
    Res = pHead;//Res指针指向链表
}

PMAPS MemorySearch_DWORD(int value, PMAPS pMap) {
    if (pid == 0) {
        puts("can not get pid");
        return NULL;
    }
    gs = 0;
    PMAPS pTemp = NULL;
    pTemp = pMap;
    PMAPS n, e;
    e = n = (PMAPS) malloc(LEN);
    PMAPS pBuff;
    pBuff = n;
    int handle;//句柄
    int iCount = 0;//链表长度
    int c;
    char lj[64];//路径
    int buff[1024] = {0};//缓冲区
    memset(buff, 0, 4);
    sprintf(lj, "/proc/%d/mem", pid);
    handle = open(lj, O_RDWR);//打开mem文件
    lseek(handle, 0, SEEK_SET);
    while (pTemp != NULL)//读取maps里面的地址
    {
        c = (pTemp->taddr - pTemp->addr) / 4096;
        for (int j = 0; j < c; j++) {
            pread64(handle, buff, 0x1000, pTemp->addr + j * 4096);
            for (int i = 0; i < 1024; i++) {
                if (buff[i] == value) {
                    iCount++;
                    gs += 1;
                    ResCount += 1;
                    n->addr = (pTemp->addr) + (j * 4096) + (i * 4);
//printf("addr:%lx,val:%d,buff=%d\n",n->addr,value,buff[i]);
                    if (iCount == 1) {
                        n->next = NULL;
                        e = n;
                        pBuff = n;
                    } else {
                        n->next = NULL;
                        e->next = n;
                        e = n;
                    }
                    n = (PMAPS) malloc(LEN);
                }
            }
        }
        pTemp = pTemp->next;
    }
    free(n);
    close(handle);
    return pBuff;
}

PMAPS MemorySearch_FLOAT(float value, PMAPS pMap) {
    if (pid == 0) {
        puts("can not get pid");
        return NULL;
    }
    gs = 0;
    PMAPS pTemp = NULL;
    pTemp = pMap;
    PMAPS n, e;
    e = n = (PMAPS) malloc(LEN);
    PMAPS pBuff;
    pBuff = n;
    int handle;//句柄
    int iCount = 0;//链表长度
    int c;
    char lj[64];//路径
    float buff[1024] = {0};//缓冲区
    sprintf(lj, "/proc/%d/mem", pid);
    handle = open(lj, O_RDWR);//打开mem文件
    lseek(handle, 0, SEEK_SET);
    while (pTemp->next != NULL) {
        c = (pTemp->taddr - pTemp->addr) / 4096;
        for (int j = 0; j < c; j += 1) {
            pread64(handle, buff, 0x1000, pTemp->addr + (j * 4096));
            for (int i = 0; i < 1024; i += 1) {
                if (buff[i] == value) {
                    iCount++;
                    gs += 1;
                    ResCount += 1;
                    n->addr = (pTemp->addr) + (j * 4096) + (i * 4);
                    if (iCount == 1) {
                        n->next = NULL;
                        e = n;
                        pBuff = n;
                    } else {
                        n->next = NULL;
                        e->next = n;
                        e = n;
                    }
                    n = (PMAPS) malloc(LEN);
                }
//printf("buff[%d]=%f\n",l,buff[l]);
//usleep(1);
            }
//memset(buff,0,4);
        }
        pTemp = pTemp->next;
    }
    free(n);
    close(handle);
    return pBuff;
}

void MemoryOffset(const char *value, OFFSET offset, TYPE type) {
    PMAPS pHead = NULL;
    int fromValue_i, toValue_i;
    float fromValue_f, toValue_f;
    switch (type) {
        case DWORD:
            if (strstr(value, "~") != NULL) {
                sscanf(value, "%d~%d", &fromValue_i, &toValue_i);
                if (fromValue_i > toValue_i)
                    pHead = RangeMemoryOffset_DWORD(toValue_i, fromValue_i, offset, Res);
                else
                    pHead = RangeMemoryOffset_DWORD(fromValue_i, toValue_i, offset, Res);
            } else {
                pHead = MemoryOffset_DWORD(atoi(value), offset, Res);
            }
            break;
        case FLOAT:
            if (strstr(value, "~") != NULL) {
                sscanf(value, "%f~%f", &fromValue_f, &toValue_f);
                if (fromValue_f > toValue_f)
                    pHead = RangeMemoryOffset_FLOAT(toValue_f, fromValue_f, offset, Res);
                else
                    pHead = RangeMemoryOffset_FLOAT(fromValue_f, toValue_f, offset, Res);
            } else {
                pHead = MemoryOffset_FLOAT(atof(value), offset, Res);
            }
            break;
        default:
            printf("\033[32;1mYou Select A NULL Type!\n");
            break;
    }
    if (pHead == NULL) {
        puts("offset error");
        return (void) 0;
    }
    ResCount = gs;//全局个数
    ClearResults();//清空存储的数据(释放空间)
    Res = pHead;//指向新搜索到的空间
}

PMAPS MemoryOffset_DWORD(int value, OFFSET offset, PMAPS pBuff)//搜索偏移
{
    if (pid == 0) {
        puts("can not get pid");
        return 0;
    }
    gs = 0;//初始个数为0
    PMAPS pEnd = NULL;
    PMAPS pNew = NULL;
    PMAPS pTemp = pBuff;
    PMAPS BUFF = NULL;
    pEnd = pNew = (PMAPS) malloc(LEN);
    BUFF = pNew;
    int iCount = 0, handle;//个数与句柄
    char lj[64];//路径
    unsigned long all;//总和
    int *buf = (int *) malloc(sizeof(int));//缓冲区
    int jg;
    sprintf(lj, "/proc/%d/mem", pid);
    handle = open(lj, O_RDWR);
    lseek(handle, 0, SEEK_SET);
    while (pTemp != NULL) {
        all = pTemp->addr + offset;
        pread64(handle, buf, 4, all);
        jg = *buf;
        if (jg == value) {
            iCount++;
            gs += 1;
            pNew->addr = pTemp->addr;
            if (iCount == 1) {
                pNew->next = NULL;
                pEnd = pNew;
                BUFF = pNew;
            } else {
                pNew->next = NULL;
                pEnd->next = pNew;
                pEnd = pNew;
            }
            pNew = (PMAPS) malloc(LEN);
            if (ResCount == 1) {
                free(pNew);
                close(handle);
                return BUFF;
            }
        }
/*else
{
printf("jg:%d,value:%d\n",jg,value);
}*/
        pTemp = pTemp->next;//指向下一个节点读取数据
    }
    free(pNew);
    close(handle);
    return BUFF;
}

PMAPS MemoryOffset_FLOAT(float value, OFFSET offset, PMAPS pBuff)//搜索偏移
{
    if (pid == 0) {
        puts("can not get pid");
        return 0;
    }
    gs = 0;//初始个数为0
    PMAPS pEnd = NULL;
    PMAPS pNew = NULL;
    PMAPS pTemp = pBuff;
    PMAPS BUFF = NULL;
    pEnd = pNew = (PMAPS) malloc(LEN);
    BUFF = pNew;
    int iCount = 0, handle;//个数与句柄
    char lj[64];//路径
    unsigned long all;//总和
    float *buf = (float *) malloc(sizeof(float));//缓冲区
//int buf[16];  //出现异常使用
    float jg;
    sprintf(lj, "/proc/%d/mem", pid);
    handle = open(lj, O_RDWR);
    lseek(handle, 0, SEEK_SET);
    while (pTemp != NULL) {
        all = pTemp->addr + offset;//偏移后的地址
        pread64(handle, buf, 4, all);
        jg = *buf;
        if (jg == value) {
            iCount++;
            gs += 1;
//printf("偏移成功,addr:%lx\n",all);
            pNew->addr = pTemp->addr;
            if (iCount == 1) {
                pNew->next = NULL;
                pEnd = pNew;
                BUFF = pNew;
            } else {
                pNew->next = NULL;
                pEnd->next = pNew;
                pEnd = pNew;
            }
            pNew = (PMAPS) malloc(LEN);
            if (ResCount == 1) {
                free(pNew);
                close(handle);
                return BUFF;
            }
        }
/*else
{
printf("jg:%e,value:%e\n",jg,value);
}*/
        pTemp = pTemp->next;//指向下一个节点读取数据
    }
    free(pNew);
    close(handle);
    return BUFF;
}

void RangeMemoryOffset(char *from_value, char *to_value, OFFSET offset, TYPE type)//范围偏移
{
    PMAPS pHead = NULL;
    switch (type) {
        case DWORD:
            if (atoi(from_value) > atoi(to_value))
                pHead = RangeMemoryOffset_DWORD(atoi(to_value), atoi(from_value), offset, Res);
            else
                pHead = RangeMemoryOffset_DWORD(atoi(from_value), atoi(to_value), offset, Res);
            break;
        case FLOAT:
            if (atof(from_value) > atof(to_value))
                pHead = RangeMemoryOffset_FLOAT(atof(to_value), atof(from_value), offset, Res);
            else
                pHead = RangeMemoryOffset_FLOAT(atof(from_value), atof(to_value), offset, Res);
            break;
        default:
            printf("\033[32;1mYou Select A NULL Type!\n");
            break;
    }
    if (pHead == NULL) {
        puts("RangeOffset error");
        return (void) 0;
    }
    ResCount = gs;//全局个数
    ClearResults();//清空存储的数据(释放空间)
    Res = pHead;//指向新搜索到的空间
}

PMAPS RangeMemoryOffset_DWORD(int from_value, int to_value, OFFSET offset, PMAPS pBuff)//搜索偏移DWORD
{
    if (pid == 0) {
        puts("can not get pid");
        return 0;
    }
    gs = 0;//初始个数为0
    PMAPS pEnd = NULL;
    PMAPS pNew = NULL;
    PMAPS pTemp = pBuff;
    PMAPS BUFF = NULL;
    pEnd = pNew = (PMAPS) malloc(LEN);
    BUFF = pNew;
    int iCount = 0, handle;//个数与句柄
    char lj[64];//路径
    unsigned long all;//总和
    int *buf = (int *) malloc(sizeof(int));//缓冲区
    int jg;
    sprintf(lj, "/proc/%d/mem", pid);
    handle = open(lj, O_RDWR);
    lseek(handle, 0, SEEK_SET);
    while (pTemp != NULL) {
        all = pTemp->addr + offset;
        pread64(handle, buf, 4, all);
        jg = *buf;
        if (jg >= from_value && jg <= to_value) {
            iCount++;
            gs += 1;
            pNew->addr = pTemp->addr;
            if (iCount == 1) {
                pNew->next = NULL;
                pEnd = pNew;
                BUFF = pNew;
            } else {
                pNew->next = NULL;
                pEnd->next = pNew;
                pEnd = pNew;
            }
            pNew = (PMAPS) malloc(LEN);
            if (ResCount == 1) {
                free(pNew);
                close(handle);
                return BUFF;
            }
        }
/*else
{
printf("jg:%d,value:%d\n",jg,value);
}*/
        pTemp = pTemp->next;//指向下一个节点读取数据
    }
    free(pNew);
    close(handle);
    return BUFF;
}

PMAPS
RangeMemoryOffset_FLOAT(float from_value, float to_value, OFFSET offset, PMAPS pBuff)//搜索偏移FLOAT
{
    if (pid == 0) {
        puts("can not get pid");
        return 0;
    }
    gs = 0;//初始个数为0
    PMAPS pEnd = NULL;
    PMAPS pNew = NULL;
    PMAPS pTemp = pBuff;
    PMAPS BUFF = NULL;
    pEnd = pNew = (PMAPS) malloc(LEN);
    BUFF = pNew;
    int iCount = 0, handle;//个数与句柄
    char lj[64];//路径
    unsigned long all;//总和
    float *buf = (float *) malloc(sizeof(float));//缓冲区
//int buf[16];  //出现异常使用
    float jg;
    sprintf(lj, "/proc/%d/mem", pid);
    handle = open(lj, O_RDWR);
    lseek(handle, 0, SEEK_SET);
    while (pTemp != NULL) {
        all = pTemp->addr + offset;//偏移后的地址
        pread64(handle, buf, 4, all);
        jg = *buf;
        if (jg >= from_value && jg <= to_value) {
            iCount++;
            gs += 1;
//printf("偏移成功,addr:%lx\n",all);
            pNew->addr = pTemp->addr;
            if (iCount == 1) {
                pNew->next = NULL;
                pEnd = pNew;
                BUFF = pNew;
            } else {
                pNew->next = NULL;
                pEnd->next = pNew;
                pEnd = pNew;
            }
            pNew = (PMAPS) malloc(LEN);
            if (ResCount == 1) {
                free(pNew);
                close(handle);
                return BUFF;
            }
        }
/*else
{
printf("jg:%e,value:%e\n",jg,value);
}*/
        pTemp = pTemp->next;//指向下一个节点读取数据
    }
    free(pNew);
    close(handle);
    return BUFF;
}

void MemoryWrite(const char *value, OFFSET offset, TYPE type) {
    switch (type) {
        case DWORD:
            MemoryWrite_DWORD(atoi(value), Res, offset);
            break;
        case FLOAT:
            MemoryWrite_FLOAT(atof(value), Res, offset);
            break;
        default:
            printf("\033[32;1mYou Select A NULL Type!\n");
            break;
    }
//ClearResults();//清空list
}

int MemoryWrite_DWORD(int value, PMAPS pBuff, OFFSET offset) {
    if (pid == 0) {
        puts("can not get pid");
        return 0;
    }
    PMAPS pTemp = NULL;
    char lj[64];
    int handle;
    pTemp = pBuff;
    sprintf(lj, "/proc/%d/mem", pid);
    handle = open(lj, O_RDWR);
    lseek(handle, 0, SEEK_SET);
    int i;
    for (i = 0; i < ResCount; i++) {
        pwrite64(handle, &value, 4, pTemp->addr + offset);
        if (pTemp->next != NULL)
            pTemp = pTemp->next;
    }
    close(handle);
    return 0;
}

int MemoryWrite_FLOAT(float value, PMAPS pBuff, OFFSET offset) {
    if (pid == 0) {
        puts("can not get pid");
        return 0;
    }
    PMAPS pTemp = NULL;
    char lj[64];
    int handle;
    pTemp = pBuff;
    sprintf(lj, "/proc/%d/mem", pid);
    handle = open(lj, O_RDWR);
    lseek(handle, 0, SEEK_SET);
    int i;
    for (i = 0; i < ResCount; i++) {
        pwrite64(handle, &value, 4, pTemp->addr + offset);
        if (pTemp->next != NULL)
            pTemp = pTemp->next;
    }
    close(handle);
    return 0;
}

void *SearchAddress(ADDRESS addr)//返回一个void指针,可以自行转换类型
{
    if (pid == 0) {
        puts("can not get pid");
        return 0;
    }
    char lj[64];
    int handle;
    void *buf = malloc(8);
    sprintf(lj, "/proc/%d/mem", pid);
    handle = open(lj, O_RDWR);
    lseek(handle, 0, SEEK_SET);
    pread64(handle, buf, 8, addr);
    close(handle);
    return buf;
}

int WriteAddress(ADDRESS addr, void *value, TYPE type) {
    if (pid == 0) {
        puts("can not get pid");
        return 0;
    }
    char lj[64];
    int handle;
    sprintf(lj, "/proc/%d/mem", pid);
    handle = open(lj, O_RDWR);
    lseek(handle, 0, SEEK_SET);
    switch (type) {
        case DWORD:
            pwrite64(handle, (int *) value, 4, addr);
            break;
        case FLOAT:
            pwrite64(handle, (float *) value, 4, addr);
            break;
        default:
            printf("\033[32;1mYou Select A NULL Type!\n");
            break;
    }
    close(handle);
    return 0;
}

int GetResultCount() {
    int grc = gs;
    return grc;
}

int isapkinstalled(PACKAGENAME *bm) {
    char LJ[128];
    sprintf(LJ, "/data/data/%s/", bm);
    DIR *dir;
    dir = opendir(LJ);
    if (dir == NULL) {
        return 0;
    } else {
        return 1;
    }
}

int isapkrunning(PACKAGENAME *bm) {
    DIR *dir = NULL;
    struct dirent *ptr = NULL;
    FILE *fp = NULL;
    char filepath[50];            // 大小随意，能装下cmdline文件的路径即可
    char filetext[128];            // 大小随意，能装下要识别的命令行文本即可
    dir = opendir("/proc/");        // 打开路径
    if (dir != NULL) {
        while ((ptr = readdir(dir)) != NULL)    // 循环读取路径下的每一个文件/文件夹
        {
// 如果读取到的是"."或者".."则跳过，读取到的不是文件夹名字也跳过
            if ((strcmp(ptr->d_name, ".") == 0) || (strcmp(ptr->d_name, "..") == 0))
                continue;
            if (ptr->d_type != DT_DIR)
                continue;
            sprintf(filepath, "/proc/%s/cmdline", ptr->d_name);    // 生成要读取的文件的路径
            fp = fopen(filepath, "r");    // 打开文件
            if (NULL != fp) {
                fgets(filetext, sizeof(filetext), fp);    // 读取文件
                if (strcmp(filetext, bm) == 0) {
                    closedir(dir);
                    return 1;
                }
                fclose(fp);
            }
        }
    }
    closedir(dir);    // 关闭路径
    return 0;
}

int uninstallapk(PACKAGENAME *bm) {
    char ml[128];
    sprintf(ml, "pm uninstall %s", bm);
    system(ml);
//clrscr();
    return 0;
}

int installapk(char *lj) {
    char ml[128];
    sprintf(ml, "pm install %s", lj);
    system(ml);
//clrscr();
    return 0;
}

int killprocess(PACKAGENAME *bm) {
    int pid = initMemoryTools(bm);
    if (pid == 0) {
        return -1;
    }
    char ml[32];
    sprintf(ml, "kill %d", pid);
    system(ml);//杀掉进程
    return 0;
}

char GetProcessState(PACKAGENAME *bm) {
/*
D 无法中断的休眠状态（通常 IO 的进程）；
R 正在运行，在可中断队列中；
S 处于休眠状态，静止状态；
T 停止或被追踪，暂停执行；
W 进入内存交换（从内核2.6开始无效）；
X 死掉的进程；
Z 僵尸进程不存在但暂时无法消除；
W: 没有足够的记忆体分页可分配WCHAN 正在等待的进程资源；
<: 高优先级进程
N: 低优先序进程
L: 有记忆体分页分配并锁在记忆体内 (即时系统或捱A I/O)，即,有些页被锁进内存
s 进程的领导者（在它之下有子进程）；
l 多进程的（使用 CLONE_THREAD, 类似 NPTL pthreads）；
+ 位于后台的进程组；
*/
    int pid = initMemoryTools(bm);//获取pid
    if (pid == 0) {
        return 0;//无进程
    }
    FILE *fp;
    char lj[64];
    char buff[64];
    char zt;
    char zt1[16];
    sprintf(lj, "/proc/%d/status", pid);
    fp = fopen(lj, "r");//打开status文件
    if (fp == NULL) {
        return 0;//无进程
    }
//puts("loop");
    while (!feof(fp)) {
        fgets(buff, sizeof(buff), fp);//读取
        if (strstr(buff, "State"))//筛选
        {
            sscanf(buff, "State: %c", &zt);//选取
//printf("state:%c\n",zt);
//sleep(1);
//puts("emmmm");
            break;//跳出循环
        }
    }
//putchar(zt);
//puts(zt2);
    fclose(fp);
//puts("loopopp");
    return zt;//返回状态
}

int PutDate() {
    return system("date +%F-%T");
}

int GetDate(char *date) {
    FILE *fp;//文件指针
    system("date +%F-%T > log.txt");//执行
    if ((fp = fopen("log.txt", "r")) == NULL) {
        return 0;
    }
    fscanf(fp, "%s", date);//读取
    remove("log.txt");//删除
    return 1;
}

void BypassGameSafe() {
    system("echo 0 > /proc/sys/fs/inotify/max_user_watches");
//char ml[80];
//sprintf(ml,"chmod 444 /data/data/%s/files",bm);
//system(ml);
}

/*void RecBypassGameSafe(char *bm)
{
char ml[80];
sprintf(ml,"chmod 771 /data/data/%s/files",bm);
system(ml);
}*/

void *FreezeThread(void *args) {
    int handle;
    if (pid == 0) {
        return (void *) 0;
    }

    char lj[64];
    int buf_i;
    float buf_f;
    sprintf(lj, "/proc/%d/mem", pid);
    handle = open(lj, O_RDWR);
    if (handle == -1) {
        return (void *) 0;
    }

    lseek(handle, 0, SEEK_SET);
    PFREEZE pTemp = Pfreeze;
    while (Freeze == 1) {
        for (int i = 0; i < FreezeCount; i++) {
            switch (pTemp->type) {
                case DWORD:
                    buf_i = atoi(pTemp->value);
                    pwrite64(handle, &buf_i, 4, pTemp->addr);
                    break;
                case FLOAT:
                    buf_f = atof(pTemp->value);
                    pwrite64(handle, &buf_f, 4, pTemp->addr);
                    break;
                default:
                    break;
            }
            pTemp = pTemp->next;
            usleep(delay);
        }
        pTemp = Pfreeze;//重新指向头指针
    }
    return NULL;
}

PMAPS GetResults()//获取搜索出的结果
{
    if (Res == NULL) {
        return NULL;
    } else {
        return Res;//返回头指针
    }
}

int AddFreezeItem_All(const char *Value, TYPE type, OFFSET offset) {
    if (ResCount == 0) {
        return -1;
    }
    PMAPS pTemp = Res;
    for (int i = 0; i < ResCount; i++) {
        switch (type) {
            case DWORD:
                AddFreezeItem(pTemp->addr, Value, DWORD, offset);
                break;
            case FLOAT:
                AddFreezeItem(pTemp->addr, Value, FLOAT, offset);
                break;
            default:
                puts("You Choose a NULL type");
                break;
        }
        pTemp = pTemp->next;
    }
    return 0;
}

int AddFreezeItem(ADDRESS addr, const char *value, TYPE type, OFFSET offset) {
    switch (type) {
        case DWORD:
            AddFreezeItem_DWORD(addr + offset, value);
            break;
        case FLOAT:
            AddFreezeItem_FLOAT(addr + offset, value);
            break;
        default:
            puts("You Choose a NULL type");
            break;
    }
    return 0;
}

int AddFreezeItem_DWORD(ADDRESS addr, const char *value) {
    if (FreezeCount == 0)//如果没有数据
    {
        Pfreeze = pEnd = pNew = (PFREEZE) malloc(FRE);//分配新空间
        pNew->next = NULL;
        pEnd = pNew;
        Pfreeze = pNew;
        pNew->addr = addr;//储存地址
        pNew->type = DWORD;
        pNew->value = value;
        FreezeCount += 1;
    } else {
        pNew = (PFREEZE) malloc(FRE);//分配空间
        pNew->next = NULL;
        pEnd->next = pNew;
        pEnd = pNew;
        pNew->addr = addr;
        pNew->type = DWORD;
        pNew->value = value;
        FreezeCount += 1;
    }
    return 0;
}

int AddFreezeItem_FLOAT(ADDRESS addr, const char *value) {
    if (FreezeCount == 0)//如果没有数据
    {
        Pfreeze = pEnd = pNew = (PFREEZE) malloc(FRE);//分配新空间
        pNew->next = NULL;
        pEnd = pNew;
        Pfreeze = pNew;
        pNew->addr = addr;//储存地址
        pNew->type = FLOAT;
        pNew->value = value;
        FreezeCount += 1;
    } else {
        pNew = (PFREEZE) malloc(FRE);//分配空间
        pNew->next = NULL;
        pEnd->next = pNew;
        pEnd = pNew;
        pNew->addr = addr;
        pNew->type = FLOAT;
        pNew->value = value;
        FreezeCount += 1;
    }
    return 0;
}

int RemoveFreezeItem(ADDRESS addr) {
    PFREEZE pTemp = Pfreeze;
    PFREEZE p1 = NULL;
    PFREEZE p2 = NULL;
    for (int i = 0; i < FreezeCount; i++) {
        p1 = pTemp;
        p2 = pTemp->next;
        if (pTemp->addr == addr) {
            p1->next = p2;
            free(pTemp);
            FreezeCount -= 1;
//printf("冻结个数:%d\n",FreezeCount);
//break;//防止地址重复冻结，所以不加，当然也可以加上
        }
        pTemp = p2;
    }
    return 0;
}

int RemoveFreezeItem_All() {
    PFREEZE pHead = Pfreeze;
    PFREEZE pTemp = pHead;
    int i;
    for (i = 0; i < FreezeCount; i++) {
        pTemp = pHead;
        pHead = pHead->next;
        free(pTemp);
        FreezeCount -= 1;
    }
    free(Pfreeze);
    FreezeCount -= 1;
    return 0;
}

int StartFreeze() {
    if (Freeze == 1)//已经开启冻结
    {
        return -1;
    }
    Freeze = 1;
    pthread_create(&pth, NULL, FreezeThread, NULL);
//创建线程(开始冻结线程)
    return 0;
}

int StopFreeze() {
    Freeze = 0;
    return 0;
}

int SetFreezeDelay(long int De) {
    delay = De;
    return 0;
}

int PrintFreezeItems() {
    PFREEZE pTemp = Pfreeze;
    for (int i = 0; i < FreezeCount; i++) {
        printf("FreezeAddr:%lx,type:%s,value:%s\n", pTemp->addr,
               pTemp->type == DWORD ? "DWORD" : "FLOAT", pTemp->value);
        pTemp = pTemp->next;
    }
    return 0;
}
