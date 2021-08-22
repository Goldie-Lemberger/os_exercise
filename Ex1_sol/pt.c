#include <stdlib.h>
#include <stdio.h>
#include "os.h"

//Ex1 -Goldie Lemberger - 212369060 
uint64_t check_map(uint64_t pt, uint64_t index){

	uint64_t* help_pt = phys_to_virt(((pt>>12)<<12)+ index*8);

	if ((*help_pt & 0x1) == 0) {
		
		return NO_MAPPING;
	}	
		return *help_pt;		
}

uint64_t mapping(uint64_t pt, uint64_t index){
	uint64_t help_pt = alloc_page_frame();
	uint64_t *help = (phys_to_virt((((pt>>12)<<12)+8*index)));
	*(help) = (help_pt<<12)|0x1;
	return *help;
}

void page_table_update(uint64_t pt, uint64_t vpn, uint64_t ppn){
	uint64_t help_pt, mvpn;

	
	vpn = vpn>>12;
	pt=pt<<12;
	
		for(int i=0;i<4;i++){
			mvpn =(vpn>>(36-9*i)) & 0x1ff;
		
			help_pt = pt;
			
			pt =check_map(help_pt,mvpn);
		
			if (pt==NO_MAPPING) {
				if(ppn!=NO_MAPPING){
					pt = mapping(help_pt,mvpn);
					
				}
				else{return;}
			}	
			
			}
			if(ppn!=NO_MAPPING){
			
			*(uint64_t*)phys_to_virt(pt)=ppn;
			}else{
			*(uint64_t*)phys_to_virt(pt)=NO_MAPPING;
}
}

uint64_t page_table_query(uint64_t pt, uint64_t vpn){
	vpn = vpn >> 12;
	uint64_t help_pt = check_map(pt,(vpn>>36)& 0x1ff);
	
	for(int i=1; i<4;i++){
		if(help_pt == NO_MAPPING){	

			return NO_MAPPING;
		}else{	
		
			help_pt = check_map(help_pt,(vpn>>(36-9*i))& 0x1ff);	
			
			}
		}
	if(*(uint64_t*)phys_to_virt(help_pt) == NO_MAPPING){		
		return NO_MAPPING;
	}
	return *(uint64_t*)phys_to_virt(help_pt);
}


