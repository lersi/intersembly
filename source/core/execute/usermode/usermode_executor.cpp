#include "usermode_executor.h" 

#include <sys/types.h>
#include <sys/mman.h>

#ifdef __MACH__
    #include <mach/vm_statistics.h>
#elif defined(__linux__)
#endif

#define NO_FD (-1)

using namespace execute;

static
int 
convert_page_size_into_flag(
    const page_size_e page_size
){
    int result = 0;

    switch(page_size)
    {
        case page_size_e::PAGE_4KB:
            #ifdef __MACH__
                result = VM_FLAGS_SUPERPAGE_NONE;
            #elif defined(__linux__)
                /* there are no flags to add */
            #elif defined(__FreeBSD__)
                /* there are no flags to add */
            #endif
            break;
        case page_size_e::PAGE_2MB:
            #ifdef __MACH__
                result = VM_FLAGS_SUPERPAGE_SIZE_2MB;
            #elif defined(__linux__)
                result = MAP_HUGETLB;
            #elif defined(__FreeBSD__)
                result = MAP_ALIGNED_SUPER;
            #endif
            break;
        case page_size_e::PAGE_1GB:
            #ifdef __MACH__
                result = VM_FLAGS_SUPERPAGE_SIZE_ANY;
            #elif defined(__linux__)
                result = MAP_HUGETLB;
            #elif defined(__FreeBSD__)
                result = MAP_ALIGNED_SUPER;
            #endif
            break;
        default:
            /** @todo athrow specific exeption for unexpected value in switch case */
            throw std::exception();
    }

    return result;
}

address_t
IUsermodeExecutor::allocate_page(
	IN address_t page_address,
    IN page_size_e size_of_page
){
    address_t result = NULL_ADDRESS;
    void * mmap_result = MAP_FAILED;
    void * page_base_address = reinterpret_cast<void *>(page_address);
    size_t size = get_size_of_page(size_of_page);
    int protection = PROT_READ | PROT_WRITE | PROT_EXEC;
    int flags = MAP_ANONYMOUS | MAP_PRIVATE;
    int os_specific_flags = convert_page_size_into_flag(size_of_page);
    int fd = NO_FD;
    off_t offset = 0;

    mmap_result = mmap(
        page_base_address,
        size,
        protection,
        flags | os_specific_flags,
        fd,
        offset
    );
    #ifdef __linux__
    /* if fails try agian without huge page flags */
    if (MAP_FAILED == mmap_result) {
        mmap_result = mmap(
            page_base_address,
            size,
            protection,
            flags,
            fd,
            offset
        );
    }
    #endif
    if (MAP_FAILED == mmap_result) {
        /** @todo add specific exceptions for each error type */
        throw std::exception();
    }
    
    result = reinterpret_cast<address_t>(mmap_result);
    return result;
}

void
IUsermodeExecutor::free_page(
    IN address_t page_address,
    IN uint64_t size
){
    if(page_address % PAGE_4KB_SIZE != 0){
        /** @todo raise proper exception */
        throw std::exception();
    }
    if(size % PAGE_4KB_SIZE != 0){
        /** @todo raise proper exception */
        throw std::exception();
    }

    if(0 != munmap(
        reinterpret_cast<void *>(page_address), 
        static_cast<size_t>(size))
    ){
        /** @todo raise proper exception */
        throw std::exception();
    }
}
