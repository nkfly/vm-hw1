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

struct shadow_pair_node *head_to_shadow_pair_node;
struct shadow_pair_node *head_to_shack;
struct shadow_pair_node *head_to_cache;
int shack_index = 0;

static inline void shack_init(CPUState *env)
{
	//env->shack = (uint64_t *)malloc(SHACK_SIZE * sizeof(uint64_t));
	head_to_shadow_pair_node = NULL;
	head_to_cache = NULL;
	head_to_shack = (struct shadow_pair_node *)malloc(SHACK_SIZE*sizeof(struct shadow_pair_node));
}

/*
 * shack_set_shadow()
 *  Insert a guest eip to host eip pair if it is not yet created.
 */
void shack_set_shadow(CPUState *env, target_ulong guest_eip, unsigned long *host_eip)
{
	struct shadow_pair_node *top = (struct shadow_pair_node *)malloc(sizeof(struct shadow_pair_node));
	top->guest_eip = guest_eip;
	top->host_eip = host_eip;
	top->next = head_to_shadow_pair_node;
	head_to_shadow_pair_node = top;
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
{	struct shadow_pair_node *ptr = head_to_cache;
	while (ptr != NULL){
		if (ptr->guest_eip == next_eip){
			struct shadow_pair_node top = head_to_shack[shack_index];
			top.guest_eip = next_eip;
			top.host_eip = ptr->host_eip;
			shack_index = (shack_index+1)%SHACK_SIZE;

			return ;
		}
		ptr = ptr->next;
	}
	struct shadow_pair_node *shadow_pair_node_ptr = head_to_shadow_pair_node;
	//print("push shack");
	while (shadow_pair_node_ptr != NULL){
		if (shadow_pair_node_ptr->guest_eip == next_eip){
			
			struct shadow_pair_node top = head_to_shack[shack_index];
			top.guest_eip = next_eip;
			top.host_eip = shadow_pair_node_ptr->host_eip;

			struct shadow_pair_node *cache_node = (struct shadow_pair_node *)malloc(sizeof(struct shadow_pair_node));
			cache_node->guest_eip = next_eip;
			cache_node->host_eip = shadow_pair_node_ptr->host_eip;
			cache_node->next = head_to_cache;
			head_to_cache = cache_node;

			shack_index = (shack_index+1)%SHACK_SIZE;
			// print("yoyoman");
			return;

		}
		shadow_pair_node_ptr = shadow_pair_node_ptr->next;
	}

}

/*
 * pop_shack()
 *  Pop next host eip from shadow stack.
 */
void pop_shack(TCGv_ptr cpu_env, target_ulong next_eip)
{
	// struct shadow_pair_node *shadow_ptr = head_to_shack;
	// int i = 0;

	// while (shadow_ptr != NULL){
	// 		char a[100];
	// 		sprintf(a, "%d\n", i++);
	// 		print(a);
	// 	if (shadow_ptr->guest_eip == next_eip){
	// 		print("yo");
	// 		break;

	// 	}
	// 	shadow_ptr = shadow_ptr->next;
	// }

}

void *helper_pop_shack(target_ulong guest_eip)
{
	//struct shadow_pair_node shadow_ptr = head_to_shack;

	while (shack_index > 0){
		struct shadow_pair_node shadow_ptr = head_to_shack[shack_index-1];
		if (shadow_ptr.guest_eip == guest_eip){
			unsigned long *h_eip = shadow_ptr.host_eip;
			shack_index = shack_index-1;
			// print("ff before!\n");
			return h_eip;

		}else {
			shack_index = shack_index -1;
		}
	}
    return optimization_ret_addr;
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
	//if (ibtc.htable[index].tb == NULL) {
	//	return optimization_ret_addr;	
	//}else if (ibtc.htable[index].tb != NULL && ibtc.htable[index].guest_eip != guest_eip){
	//	return optimization_ret_addr;
	if (ibtc.htable[index].tb != NULL && ibtc.htable[index].guest_eip == guest_eip){
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
