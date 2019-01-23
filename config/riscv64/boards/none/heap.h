#ifndef HEAP_H
#define HEAP_H

#define configTOTAL_HEAP_SIZE		( ( size_t ) ( 20 * 1024 ) )
#define configUSE_MALLOC_FAILED_HOOK	1
#define configSUPPORT_DYNAMIC_ALLOCATION 1

#define vTaskSuspendAll(...) ((void)0)
#define xTaskResumeAll(...) ((void)0)
#define mtCOVERAGE_TEST_MARKER(...) ((void)0)
#define traceMALLOC(...) ((void)0)
#define traceFREE(...) ((void)0)

#define configASSERT(cond) do { if (!(cond)) { sysExit(-2); } } while(0)


void *malloc(size_t size);
void free(void *ptr);
void *calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t size);

void vApplicationMallocFailedHook(void);

/* A few RISCV definitions for FreeRTOS */
#define portCHAR                char
#define portFLOAT               float
#define portDOUBLE              double
#define portLONG                long
#define portSHORT               short
#define portBASE_TYPE           long

#if __riscv_xlen == 64
    #define portSTACK_TYPE          uint64_t
    #define portPOINTER_SIZE_TYPE   uint64_t
#else
    #define portSTACK_TYPE          uint32_t
    #define portPOINTER_SIZE_TYPE   uint32_t
#endif

#if __riscv_xlen == 64
    #define portBYTE_ALIGNMENT      8
#else
    #define portBYTE_ALIGNMENT      4
#endif

#if portBYTE_ALIGNMENT == 32
        #define portBYTE_ALIGNMENT_MASK ( 0x001f )
#endif

#if portBYTE_ALIGNMENT == 16
        #define portBYTE_ALIGNMENT_MASK ( 0x000f )
#endif

#if portBYTE_ALIGNMENT == 8
        #define portBYTE_ALIGNMENT_MASK ( 0x0007 )
#endif

#if portBYTE_ALIGNMENT == 4
        #define portBYTE_ALIGNMENT_MASK ( 0x0003 )
#endif

#if portBYTE_ALIGNMENT == 2
        #define portBYTE_ALIGNMENT_MASK ( 0x0001 )
#endif

#if portBYTE_ALIGNMENT == 1
        #define portBYTE_ALIGNMENT_MASK ( 0x0000 )
#endif

#ifndef portBYTE_ALIGNMENT_MASK
        #error "Invalid portBYTE_ALIGNMENT definition"
#endif

typedef portSTACK_TYPE StackType_t;
typedef long BaseType_t;
typedef unsigned long UBaseType_t;


#endif //HEAP_H
