/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <helper/time_support.h>
#include <jtag/jtag.h>
#include "target/target.h"
#include "target/target_type.h"
#include "rtos.h"
#include "helper/log.h"
#include "helper/types.h"
#include "rtos_standard_stackings.h"
#include "target/armv7m.h"
#include "target/cortex_m.h"


#define RHINO_LIST_OFFSET           36

typedef enum {
    K_SEED,
    K_RDY,
    K_PEND,
    K_SUSPENDED,
    K_PEND_SUSPENDED,
    K_SLEEP,
    K_SLEEP_SUSPENDED,
    K_DELETED,
} task_stat_t;

static int rhino_detect_rtos(struct target *target);
static int rhino_create(struct target *target);
static int rhino_update_threads(struct rtos *rtos);
static int rhino_get_thread_reg_list(struct rtos *rtos, int64_t thread_id, char **hex_reg_list);
static int rhino_get_symbol_list_to_lookup(symbol_table_elem_t *symbol_list[]);

struct rhino_thread_state {
	int value;
	const char *desc;
};

static const struct rhino_thread_state rhino_thread_states[] = {
    { K_SEED,            "Seed" },
    { K_RDY,             "Ready" },
    { K_PEND,            "Pend" },
    { K_SUSPENDED,       "Suspended" },
    { K_PEND_SUSPENDED,  "Pend_suspended" },
    { K_SLEEP,           "Sleep" },
    { K_SLEEP_SUSPENDED, "Sleep_suspended" },
    { K_DELETED,         "Deleted" }
};

#define ECOS_NUM_STATES (sizeof(rhino_thread_states)/sizeof(struct rhino_thread_state))

struct rhino_params {
	const char *target_name;
	unsigned char pointer_width;
	unsigned char thread_stack_offset;
	unsigned char thread_name_offset;
	unsigned char thread_state_offset;
	const struct rtos_register_stacking *stacking_info_cm3;
	const struct rtos_register_stacking *stacking_info_cm4f;
	const struct rtos_register_stacking *stacking_info_cm4f_fpu;
};

static const struct rhino_params rhino_params_list[] = {
    {
        "cortex_m",                        /* target_name */
        4,                                 /* pointer_width; */
        0,                                 /* thread_stack_offset; */
        72,                                /* thread_name_offset; */
        76,                                /* thread_state_offset; */
        &rtos_standard_Cortex_M3_stacking, /* stacking_info */
        &rtos_standard_Cortex_M4F_stacking,
        &rtos_standard_Cortex_M4F_FPU_stacking,
    },
    {
        "hla_target",                      /* target_name */
        4,                                 /* pointer_width; */
        0,                                 /* thread_stack_offset; */
        72,                                /* thread_name_offset; */
        76,                                /* thread_state_offset; */
        &rtos_standard_Cortex_M3_stacking, /* stacking_info */
        &rtos_standard_Cortex_M4F_stacking,
        &rtos_standard_Cortex_M4F_FPU_stacking,
    }
};

#define ECOS_NUM_PARAMS ((int)(sizeof(rhino_params_list)/sizeof(struct rhino_params)))

enum rhino_symbol_values {
	rhino_VAL_current_thread_ptr = 0,
    rhino_VAL_thread_list        = 1
};

static const char * const rhino_symbol_list[] = {
	"g_active_task",
	"g_kobj_list",
	NULL
};

const struct rtos_type rhino_rtos = {
	.name = "rhino",

	.detect_rtos = rhino_detect_rtos,
	.create = rhino_create,
	.update_threads = rhino_update_threads,
	.get_thread_reg_list = rhino_get_thread_reg_list,
	.get_symbol_list_to_lookup = rhino_get_symbol_list_to_lookup,

};

static int rhino_update_threads(struct rtos *rtos)
{
	int retval;
	int tasks_found = 0;
	int thread_list_size = 0;
	const struct rhino_params *param;

	if (rtos == NULL)
		return -1;

	if (rtos->rtos_specific_params == NULL)
		return -3;

	param = (const struct rhino_params *) rtos->rtos_specific_params;

	if (rtos->symbols == NULL) {
		LOG_ERROR("No symbols for rhino");
		return -4;
	}

	if (rtos->symbols[rhino_VAL_thread_list].address == 0) {
		LOG_ERROR("Don't have the thread list head");
		return -2;
	}

	/* wipe out previous thread details if any */
	rtos_free_threadlist(rtos);


	/* determine the number of current threads */
	uint32_t thread_list_head = rtos->symbols[rhino_VAL_thread_list].address;
	uint32_t thread_list_next = NULL;
	target_read_buffer(rtos->target,
		thread_list_head,
		param->pointer_width,
		(uint8_t *) &thread_list_next);
	while (thread_list_next && thread_list_next != thread_list_head) {
		thread_list_size++;
		retval = target_read_buffer(rtos->target,
				thread_list_next,
				param->pointer_width,
				(uint8_t *) &thread_list_next);
		if (retval != ERROR_OK)
			return retval;
	}

	/* read the current thread id */
	rtos->current_thread = 0;
	retval = target_read_buffer(rtos->target,
			rtos->symbols[rhino_VAL_current_thread_ptr].address,
			4,
			(uint8_t *)&rtos->current_thread);
	if (retval != ERROR_OK) {
		LOG_ERROR("Could not read rhino current thread from target");
		return retval;
	}

	if ((thread_list_size  == 0) || (rtos->current_thread == 0)) {
		/* Either : No RTOS threads - there is always at least the current execution though */
		/* OR     : No current thread - all threads suspended - show the current execution
		 * of idling */
		char tmp_str[] = "Current Execution";
		thread_list_size++;
		tasks_found++;
		rtos->thread_details = malloc(
				sizeof(struct thread_detail) * thread_list_size);
		rtos->thread_details->threadid = 1;
		rtos->thread_details->exists = true;
		rtos->thread_details->extra_info_str = NULL;
		rtos->thread_details->thread_name_str = malloc(sizeof(tmp_str));
		strcpy(rtos->thread_details->thread_name_str, tmp_str);

		if (thread_list_size == 0) {
			rtos->thread_count = 1;
			return ERROR_OK;
		}
	} else {
		/* create space for new thread details */
		rtos->thread_details = malloc(
				sizeof(struct thread_detail) * thread_list_size);
	}

	/* loop over all threads */
	thread_list_next = thread_list_head;

    /* Get the location of the next thread structure. */
    retval = target_read_buffer(rtos->target,
            thread_list_next,
            param->pointer_width,
            (uint8_t *) &thread_list_next);
    if (retval != ERROR_OK) {
        LOG_ERROR("Error reading next thread pointer in rhino thread list");
        return retval;
    }

	while ((thread_list_next && thread_list_next != thread_list_head) && (tasks_found < thread_list_size)) {

		#define ECOS_THREAD_NAME_STR_SIZE (200)
		char tmp_str[ECOS_THREAD_NAME_STR_SIZE];
		unsigned int i = 0;
		uint32_t name_ptr = 0;
        uint32_t thread_ptr;

        /*thread head is 36 bytes before list*/
        thread_ptr = thread_list_next - RHINO_LIST_OFFSET;

		/* Save the thread pointer */
		rtos->thread_details[tasks_found].threadid = thread_ptr;

		/* read the name pointer */
		retval = target_read_buffer(rtos->target,
				thread_ptr + param->thread_name_offset,
				param->pointer_width,
				(uint8_t *)&name_ptr);

		if (retval != ERROR_OK) {
			LOG_ERROR("Could not read rhino thread name pointer from target");
			return retval;
		}

		/* Read the thread name */
		retval =
			target_read_buffer(rtos->target,
				name_ptr,
				ECOS_THREAD_NAME_STR_SIZE,
				(uint8_t *)&tmp_str);
		if (retval != ERROR_OK) {
			LOG_ERROR("Error reading thread name from rhino target");
			return retval;
		}
		tmp_str[ECOS_THREAD_NAME_STR_SIZE-1] = '\x00';

		if (tmp_str[0] == '\x00')
			strcpy(tmp_str, "No Name");

		rtos->thread_details[tasks_found].thread_name_str =
			malloc(strlen(tmp_str)+1);
		strcpy(rtos->thread_details[tasks_found].thread_name_str, tmp_str);

		/* Read the thread status */
		int64_t thread_status = 0;
		retval = target_read_buffer(rtos->target,
				thread_ptr + param->thread_state_offset,
				4,
				(uint8_t *)&thread_status);
		if (retval != ERROR_OK) {
			LOG_ERROR("Error reading thread state from rhino target");
			return retval;
		}

		for (i = 0; (i < ECOS_NUM_STATES) && (rhino_thread_states[i].value != thread_status); i++) {
			/*
			 * empty
			 */
		}

		const char *state_desc;
		if  (i < ECOS_NUM_STATES)
			state_desc = rhino_thread_states[i].desc;
		else
			state_desc = "Unknown state";

		rtos->thread_details[tasks_found].extra_info_str = malloc(strlen(
					state_desc)+8);
		sprintf(rtos->thread_details[tasks_found].extra_info_str, "State: %s", state_desc);

		rtos->thread_details[tasks_found].exists = true;

		tasks_found++;

		/* Get the location of the next thread structure. */
		retval = target_read_buffer(rtos->target,
				thread_list_next,
				param->pointer_width,
				(uint8_t *) &thread_list_next);
		if (retval != ERROR_OK) {
			LOG_ERROR("Error reading next thread pointer in rhino thread list");
			return retval;
		}

	};

	rtos->thread_count = tasks_found;
	return 0;
}

static int rhino_get_thread_reg_list(struct rtos *rtos, int64_t thread_id, char **hex_reg_list)
{
    int retval;
    const struct rhino_params *param;
    int64_t  stack_ptr  = 0;

    *hex_reg_list = NULL;
    if (rtos == NULL)
        return -1;

    if (thread_id == 0)
        return -2;

    if (rtos->rtos_specific_params == NULL)
        return -1;

    param = (const struct rhino_params *) rtos->rtos_specific_params;

    /* Read the stack pointer */
    retval = target_read_buffer(rtos->target,
            thread_id + param->thread_stack_offset,
            param->pointer_width,
            (uint8_t *)&stack_ptr);
    if (retval != ERROR_OK) {
        LOG_ERROR("Error reading stack frame from rhino thread");
        return retval;
    }
    LOG_DEBUG("rhino: Read stack pointer at 0x%" PRIx64 ", value 0x%" PRIx64 "\r\n",
                                        thread_id + param->thread_stack_offset,
                                        stack_ptr);

    /* Check for armv7m with *enabled* FPU, i.e. a Cortex-M4F */
    int cm4_fpu_enabled = 0;
    struct armv7m_common *armv7m_target = target_to_armv7m(rtos->target);
    if (is_armv7m(armv7m_target)) {
        if (armv7m_target->fp_feature == FPv4_SP) {
            /* Found ARM v7m target which includes a FPU */
            uint32_t cpacr;

            retval = target_read_u32(rtos->target, FPU_CPACR, &cpacr);
            if (retval != ERROR_OK) {
                LOG_ERROR("Could not read CPACR register to check FPU state");
                return -1;
            }

            /* Check if CP10 and CP11 are set to full access. */
            if (cpacr & 0x00F00000) {
                /* Found target with enabled FPU */
                cm4_fpu_enabled = 1;
            }
        }
    }

    if (cm4_fpu_enabled == 1) {
        /* Read the LR to decide between stacking with or without FPU */
        uint32_t LR_svc = 0;
        retval = target_read_buffer(rtos->target,
                stack_ptr + 0x20,
                param->pointer_width,
                (uint8_t *)&LR_svc);
        if (retval != ERROR_OK) {
            LOG_OUTPUT("Error reading stack frame from FreeRTOS thread\r\n");
            return retval;
        }
        if ((LR_svc & 0x10) == 0)
            return rtos_generic_stack_read(rtos->target, param->stacking_info_cm4f_fpu, stack_ptr, hex_reg_list);
        else
            return rtos_generic_stack_read(rtos->target, param->stacking_info_cm4f, stack_ptr, hex_reg_list);
    } else
        return rtos_generic_stack_read(rtos->target, param->stacking_info_cm3, stack_ptr, hex_reg_list);
}


static int rhino_get_symbol_list_to_lookup(symbol_table_elem_t *symbol_list[])
{
	unsigned int i;
	*symbol_list = calloc(
			ARRAY_SIZE(rhino_symbol_list), sizeof(symbol_table_elem_t));

	for (i = 0; i < ARRAY_SIZE(rhino_symbol_list); i++)
		(*symbol_list)[i].symbol_name = rhino_symbol_list[i];

	return 0;
}

static int rhino_detect_rtos(struct target *target)
{
	if ((target->rtos->symbols != NULL) &&
			(target->rtos->symbols[rhino_VAL_thread_list].address != 0)) {
		/* looks like rhino */
		return 1;
	}
	return 0;
}

static int rhino_create(struct target *target)
{
	int i = 0;
	while ((i < ECOS_NUM_PARAMS) &&
		(0 != strcmp(rhino_params_list[i].target_name, target->type->name))) {
		i++;
	}
	if (i >= ECOS_NUM_PARAMS) {
		LOG_ERROR("Could not find target in rhino compatibility list");
		return -1;
	}

	target->rtos->rtos_specific_params = (void *) &rhino_params_list[i];
	target->rtos->current_thread = 0;
	target->rtos->thread_details = NULL;
	return 0;
}
