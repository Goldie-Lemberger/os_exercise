
#include <stdint.h>

#define NO_MAPPING	(~0ULL)

uint64_t alloc_page_frame(void);
void* phys_to_virt(uint64_t phys_addr);

void page_table_update(uint64_t pt, uint64_t vpn, uint64_t ppn);
uint64_t page_table_query(uint64_t pt, uint64_t vpn);


