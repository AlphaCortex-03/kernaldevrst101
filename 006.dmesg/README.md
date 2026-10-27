# Linux `dmesg` command tutorial

## What does the `dmesg` command do?

The kernel keeps its logs in a ring buffer. This is done to keep early boot logs from getting lost until the syslog/journald daemon starts and archives them (on most distros they end up in the systemd journal, queried with `journalctl -k`, or in `/var/log/kern.log` / `/var/log/messages` depending on your distribution). If we didn't have the ring buffer, we would lose the early boot-up logs.

The `dmesg` command is used to print or control the kernel ring buffer. By default, it prints messages from the kernel ring buffer onto the console/stdout.

## Important `dmesg` commands:

1. **Clear ring buffer**:

   - `$ dmesg -c` — will clear the ring buffer after printing.
   - `$ dmesg -C` — will clear the ring buffer but does not print on the console.

2. **Don't print timestamps**:

   - `$ dmesg -t` — will not print timestamps.

3. **Restrict `dmesg` to a list of levels**:

   - `$ dmesg -l err,warn` — will print only error and warning messages.

4. **Print human-readable timestamps**:

   - `$ dmesg -T` — will print timestamps in a readable format. Note: the timestamp conversion can be inaccurate (it uses wall-clock time retroactively).

5. **Display the log level in the output**:

   - `$ dmesg -x` — will add the log level (and facility) to the output.

6. **Combining options**:

   - `$ dmesg -Tx` — will print both human-readable time and log level.

7. **Print the last N messages and follow new ones**:

   - `$ dmesg | tail` — the classic way to check the output of the module you just (r)mod'ed.
   - `$ dmesg -w` — keep printing new messages as they arrive (useful while testing insmod/rmmod).

## Practical usage with your module

```bash
sudo insmod ./hello.ko
dmesg | tail          # should show: test_hello_init : In init
sudo rmmod hello
dmesg | tail          # should show: test_hello_exit : In exit
```

![](../images/Screenshot%20from%202023-10-01%2022-23-20.png)
