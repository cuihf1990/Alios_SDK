/*
 * Copyright (C) 2017 YunOS Project. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __YOS_CLI_H__
#define __YOS_CLI_H__


#define CLI_UART     0
#define MAX_COMMANDS    64
#define INBUF_SIZE      128
#define OUTBUF_SIZE     2048

#ifndef FUNCPTR
typedef void (*FUNCPTR)(void);
#endif

/** Structure for registering CLI commands */
struct cli_command {
    /** The name of the CLI command */
    const char *name;
    /** The help text associated with the command */
    const char *help;
    /** The function that should be invoked for this command. */
    void (*function) (char *pcWriteBuffer, int xWriteBufferLen, int argc,
                      char **argv);
};

struct cli_st {
    int initialized;
    const struct cli_command *commands[MAX_COMMANDS];
    unsigned int num_commands;
    int echo_disabled;

    unsigned int bp;    /* buffer pointer */
    char inbuf[INBUF_SIZE];

    char outbuf[OUTBUF_SIZE];
} ;

#define CLI_ARGS char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv

#ifdef CONFIG_YOS_CLI

#define cmd_printf(...) \
    do {\
        if (xWriteBufferLen > 0) {\
            snprintf(pcWriteBuffer, xWriteBufferLen, __VA_ARGS__);\
            xWriteBufferLen-= os_strlen(pcWriteBuffer);\
            pcWriteBuffer+= os_strlen(pcWriteBuffer);\
        }\
    } while(0)


/** Register a CLI command
 *
 * This function registers a command with the command-line interface.
 *
 * \param[in] command The structure to register one CLI command
 * \return 0 on success
 * \return 1 on failure
 */
int cli_register_command(const struct cli_command *command);

/** Unregister a CLI command
 *
 * This function unregisters a command from the command-line interface.
 *
 * \param[in] command The structure to unregister one CLI command
 * \return 0 on success
 * \return 1 on failure
 */
int cli_unregister_command(const struct cli_command *command);

/** Stop the CLI thread and carry out the cleanup
 *
 * \return kNoErr on success
 * \return error code otherwise.
 *
 */
int yos_cli_stop(void);

/** Register a batch of CLI commands
 *
 * Often, a module will want to register several commands.
 *
 * \param[in] commands Pointer to an array of commands.
 * \param[in] num_commands Number of commands in the array.
 * \return 0 on success
 * \return 1 on failure
 */
int cli_register_commands(const struct cli_command *commands, int num_commands);

/** Unregister a batch of CLI commands
 *
 * \param[in] commands Pointer to an array of commands.
 * \param[in] num_commands Number of commands in the array.
 * \return 0 on success
 * \return 1 on failure
 */
int cli_unregister_commands(const struct cli_command *commands,
                            int num_commands);

/* Get a CLI msg
 *
 * If an external input task wants to use the CLI, it can use
 * cli_get_cmd_buffer() to get a command buffer that it can then
 * submit to the CLI later using cli_submit_cmd_buffer().
 *
 * \param buff Pointer to a char * to place the buffer pointer in.
 * \return 0 on success
 * \return error code otherwise.
 */
int cli_getchar(char *inbuf);
int cli_getchars(char *inbuf, int len);
int cli_get_all_chars_len(void);
int cli_getchars_prefetch(char *inbuf, int len);

/* Send CLI output msg
 *
 * \param buff Pointer to a char * buffer.
 * \return 0 on success
 * \return error code otherwise.
 */
int cli_printf(const char *buff, ...);

/* library CLI APIs
 */
int yos_cli_init(void);

#else /* CONFIG_YOS_CLI */

#define cmd_printf(...) do {} while(0)

static inline int cli_register_command(const struct cli_command *command)
{
    return 0;
}

static inline int cli_unregister_command(const struct cli_command *command)
{
    return 0;
}

static inline int cli_register_commands(const struct cli_command *commands, int num_commands)
{
    return 0;
}

static inline int cli_unregister_commands(const struct cli_command *commands, int num_commands)
{
    return 0;
}

static inline int yos_cli_init(void)
{
    return 0;
}

static inline int yos_cli_stop(void)
{
    return 0;
}
#endif


#endif

