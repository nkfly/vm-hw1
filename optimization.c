/*
 *  (C) 2010 by Computer System Laboratory, IIS, Academia Sinica, Taiwan.
 *      See COPYRIGHT in top-level directory.
 */

#include <stdlib.h>
#include "exec-all.h"
#include "tcg-op.h"
#define GEN_HELPER 1
#include "optimization.h"

extern uint8_t *optimization_ret_addr;
FILE *file_pointer;

/*
 * Shadow Stack
 */
list_t *shadow_hash_list;

static inline void shack_init(CPUState *env)
{
}

/*
 * shack_set_shadow()
 *  Insert a guest eip to host eip pair if it is not yet created.
 */
void shack_set_shadow(CPUState *env, target_ulong guest_eip, unsigned long *host_eip)
{
}

/*
 * helper_shack_flush()
 *  Reset shadow stack.
 */
void helper_shack_flush(CPUState *env)
{
}

/*
 * push_shack()
 *  Push next guest eip into shadow stack.
 */
void push_shack(CPUState *env, TCGv_ptr cpu_env, target_ulong next_eip)
{
}

/*
 * pop_shack()
 *  Pop next host eip from shadow stack.
 */
void pop_shack(TCGv_ptr cpu_env, TCGv next_eip)
{
}

/*
 * Indirect Branch Target Cache
 */
__thread int update_ibtc;

/*
 * helper_lookup_ibtc()
 *  Look up IBTC. Return next host eip if cache hit or
 *  back-to-dispatcher stub address if cache miss.
 In function helper_lookup_ibtc, you can get the related cache entry by the argument guest_eip. 
 If the lookup success, you should return the context pointer of the related translation block. 
 Otherwise, you can return the optimization_ret_addr to return to the emulation engine.
 */
struct ibtc_table ibtc;
target_ulong lasttime_guest_eip;
void *helper_lookup_ibtc(target_ulong guest_eip)
{
	//print("in");
	int index = guest_eip % IBTC_CACHE_SIZE;
	lasttime_guest_eip = guest_eip;
	if (ibtc.htable[index].tb == NULL) {
		return optimization_ret_addr;	
	}else if (ibtc.htable[index].tb != NULL && ibtc.htable[index].guest_eip != guest_eip){
		return optimization_ret_addr;
	}else if (ibtc.htable[index].tb != NULL && ibtc.htable[index].guest_eip == guest_eip){
		return ibtc.htable[index].tb;
	}
    return optimization_ret_addr;
}

/*
 * update_ibtc_entry()
 *  Populate eip and tb pair in IBTC entry.
 In function update_ibtc_entry, you should update the translation block and the guest address to the related cache entry.
  Noticeably that the function helper_lookup_ibtc is executed before the function update_ibtc_entry, 
  so the information such as guest_ip can be preserved in function helper_lookup_ibtc and used in function update_ibtc_entry later.
 */
void update_ibtc_entry(TranslationBlock *tb)
{
	int index = lasttime_guest_eip % IBTC_CACHE_SIZE;
	ibtc.htable[index].tb = tb;
	ibtc.htable[index].guest_eip = lasttime_guest_eip;
}

/*
 * ibtc_init()
 *  Create and initialize indirect branch target cache.
 In function ibtc_init, you need to allocate a space for an ibtc_table. 
 To ensure the correct of later execution, please set the allocated area to zero.
 */
static inline void ibtc_init(CPUState *env)
{
	int i = 0;
	for (; i < IBTC_CACHE_SIZE;i++){
		ibtc.htable[i].tb = NULL;
	}
}

/*
 * init_optimizations()
 *  Initialize optimization subsystem.
 */
//FILE *file_pointer = fopen("_host.log", "w");
int init_optimizations(CPUState *env)
{
    file_pointer = fopen("_host.log","w");
    //char output[100];
    //sprintf(output, "%d", IBTC_CACHE_SIZE);
    //print(output);
    shack_init(env);
    ibtc_init(env);

    //fclose(ptr);

    return 0;
}

void print( char *message){
	fprintf(file_pointer, message, strlen(message));
	fflush(file_pointer);
}

/*
 * vim: ts=8 sts=4 sw=4 expandtab
 */
