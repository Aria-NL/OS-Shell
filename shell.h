#ifndef SHELL_H
#define SHELL_H

#include <sys/types.h>

struct tree_node;

/*
 * Any value assigned to this will be displayed when asking the user for a
 * command. Do not assign any value to this if its value is NULL, as this
 * indicates the session is not interactive.
 */
extern char *prompt;

/*
 * Called once when the shell starts.
 */
void initialize(void);

/*
 * Called when a command has been read from the user.
 */
void run_command(struct tree_node *n);

/* ... */

/*
 * Handles simple commands.
 */
void commandNormal(struct tree_node *n);

/*
 * Handles pipes.
 */
void doPipe(struct tree_node *n);

/*
 * Handles sequences.
 */
void doSeq(struct tree_node *n);

/*
 * Adds a PID to the list of in-use PIDs.
 */
void addpid(pid_t pidToAdd);

/*
 * Handles Ctrl-C, so the shell does not exit when Ctrl-C is pressed.
 */
void ctrlcHandle(int signal);

#endif
