#pragma once

#pragma pack(push, 1)

struct rt_init
{
    unsigned char rtn_type;
    unsigned char priority;
    void* rtn;
};

#pragma pack(pop)

extern "C"
{
    extern int sprintf(char* buf, const char* format, ...);
    extern double sqrt(double x);
    extern void* memset(void* dst, int c, size_t length);
    extern int fclose(FILE* fp);
    extern double cos(double x);
    extern double sin(double x);
    extern FILE* fopen(const char* filename, const char* mode);
    extern int printf(const char* format, ...);
    extern int getch();
    extern int fprintf(FILE *fp, const char *format, ...);
    extern void* malloc(size_t size);
    extern void free(void* ptr);
    extern char* strdup(const char* string);
    extern int strncmp(const char* s1, const char* s2, size_t n);
    extern double pow(double x, double y);
    extern void* memcpy(void* dst, const void* src, size_t length);
    extern char* strcpy(char* dst, const char* src);
    extern size_t strlen(const char* s);
    extern void __InitRtns(unsigned int);
    extern int flushall();
    
    extern void __LocalInitRtns(unsigned int);
#pragma aux __LocalInitRtns "__InitRtns";
    extern rt_init __Start_XI;
#pragma aux __Start_XI "__Start_XI";
    extern rt_init __End_XI;
#pragma aux __End_XI "__End_XI";

    extern void* operator_new(unsigned int);
    extern void operator_delete(void* p);
}
