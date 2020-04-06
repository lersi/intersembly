#ifndef USERMODE_EXECUTOR_H
#define USERMODE_EXECUTOR_H

#include <core/execute/executor.h>

#include <exception>

namespace execute{

    constexpr address_t NULL_ADDRESS = reinterpret_cast<address_t>(nullptr);

    constexpr uint64_t PAGE_4KB_SIZE = 0x1000;
    constexpr uint64_t PAGE_2MB_SIZE = 512 * PAGE_4KB_SIZE;
    constexpr uint64_t PAGE_1GB_SIZE = 512 * PAGE_2MB_SIZE;

    typedef enum{
        PAGE_4KB,
        PAGE_2MB,
        PAGE_1GB
    } page_size_e;
    inline
    uint64_t 
    get_size_of_page(
        const page_size_e page_size
    ){
        switch(page_size)
        {
            case page_size_e::PAGE_4KB:
                return PAGE_4KB_SIZE;
            case page_size_e::PAGE_2MB:
                return PAGE_2MB_SIZE;
            case page_size_e::PAGE_1GB:
                return PAGE_1GB_SIZE;
            default:
                /** @todo athrow specific exeption for unexpected value in switch case */
                throw std::exception();
        }
    };

    /**
     * @brief implements some useful function for usermode operation
     * 
     */
    class IUsermodeExecutor : public IExecutor
    {
    protected:
        /**
         * @brief allocates entire page
         * 
         * @param[in] page_address the address that point to where to allocate the page. 0 for random place.
         * @param[in] size_of_page  the size of page to allocate from the operation system. default 4k page
         * 
         * @return the address of the allocated page, throws exeption on failure.
         */
        static
        address_t 
        allocate_page(
            IN address_t page_address,
            IN page_size_e size_of_page = page_size_e::PAGE_4KB
        );

        /**
         * @brief frees a page allocated by allocate_page
         * 
         * @note throws exeption on failure.
         * 
         * @param[in] page_address the address of the page to free
         * @param[in] size the size in bytes of the memoty to free, must be page aligned
         */
        static
        void
        free_page(
            IN address_t page_address,
            IN uint64_t size
        );

        
    };
}

#endif