// SPDX-License-Identifier: GPL-2.0
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <asm/setup.h>

static char new_command_line[COMMAND_LINE_SIZE];

static int cmdline_proc_show(struct seq_file *m, void *v)
{
	seq_puts(m, new_command_line);
	seq_putc(m, '\n');
	return 0;
}

/*
 * Drops all values of an option but the last one. The boot loader's command line
 * comes first and the boot image's last; for kernel parameters the last value
 * counts, but Android's init takes the first one.
 */
static void __init keep_last_value(char *cmd, const char *option)
{
	char *prev = NULL, *p = cmd;

	while ((p = strstr(p, option))) {
		if (p != cmd && p[-1] != ' ') {
			p++;
			continue;
		}
		if (prev) {
			/* p starts a later word, so prev's word ends at a space */
			char *next = strchr(prev, ' ') + 1;

			memmove(prev, next, strlen(next) + 1);
			p -= next - prev;
		}
		prev = p++;
	}
}

static int __init proc_cmdline_init(void)
{
    char *offset_addr, *cmd = new_command_line;

    strcpy(cmd, saved_command_line);

    /*
     * Remove 'androidboot.selinux' flag from command line set
     * by bootloader when booting recovery
     */
    offset_addr = strstr(cmd, "androidboot.hardware=ap201");
    if (offset_addr) {
        size_t i, len, offset;

        len = strlen(cmd);
        offset = offset_addr - cmd;

        for (i = 1; i < (len - offset); i++) {
            if (cmd[offset + i] == ' ')
                break;
        }

        memmove(offset_addr, &cmd[offset + i + 1], len - i - offset);
    }

	/*
	 * The boot loader's environment has androidboot.selinux=permissive and
	 * is reset by every update. Let the boot image decide.
	 */
	keep_last_value(cmd, "androidboot.selinux=");

	proc_create_single("cmdline", 0, NULL, cmdline_proc_show);
	return 0;
}
fs_initcall(proc_cmdline_init);
