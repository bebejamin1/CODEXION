*This project has been created as part of the 42 curriculum by bbeaurai.*

# Codexion

## Preview
<img width="800" height="435" alt="CODEXION_PREVIEW" src="https://github.com/user-attachments/assets/0bb366ed-5db5-4ca3-b697-0dc49db6c862" />



## Description

Codexion is a POSIX threads simulation inspired by the dining philosophers problem.
Several coders sit around a shared circular workspace, each needing **two USB dongles**
simultaneously to compile quantum code. After compiling, each coder debugs, then
refactors, then tries to acquire dongles again.

The simulation stops as soon as one coder **burns out** (fails to start compiling before
their deadline), or when every coder has compiled at least `number_of_compiles_required`
times.

Key challenges addressed: deadlock-free dongle acquisition, two scheduling policies
(FIFO and EDF), per-dongle cooldown, precise burnout detection within 10 ms, and
serialized logging across all threads.


## Visual
<img width="800" height="383" alt="Visual_CODEXION" src="https://github.com/user-attachments/assets/257c5758-1f13-4f19-b07d-6dccb66cf687" />


## Instructions

### Compilation

```sh
make          # build the codexion binary
make re       # force full rebuild
make clean    # remove object files
make fclean   # remove object files and the binary
```

### Usage

```sh
./codexion number_of_coders time_to_burnout time_to_compile time_to_debug \
           time_to_refactor number_of_compiles_required dongle_cooldown scheduler
```

| Argument                    | Type    | Description                                                                 |
|-----------------------------|---------|-----------------------------------------------------------------------------|
| `number_of_coders`          | int > 0 | Number of coder threads and dongles                                         |
| `time_to_burnout`           | int > 0 | Max ms a coder may wait before starting a compile before burning out        |
| `time_to_compile`           | int > 0 | Ms spent compiling (holding both dongles)                                   |
| `time_to_debug`             | int > 0 | Ms spent debugging                                                          |
| `time_to_refactor`          | int > 0 | Ms spent refactoring                                                        |
| `number_of_compiles_required` | int > 0 | Stop cleanly once every coder has compiled this many times                |
| `dongle_cooldown`           | int ≥ 0 | Ms a dongle is unavailable after being released                             |
| `scheduler`                 | string  | `fifo` (arrival order) or `edf` (earliest burnout deadline first)           |

All arguments are mandatory. Negative numbers, non-integers, or an unknown scheduler
name are rejected with an error message on stderr.

### Examples

```sh
# 4 coders, no burnout expected, FIFO scheduling
./codexion 4 800 200 200 200 5 10 fifo

# 3 coders, tight deadline to observe burnout, EDF scheduling
./codexion 3 400 200 200 200 10 50 edf

# 1 coder, single dongle on the table
./codexion 1 800 200 200 200 3 0 fifo

# Zero cooldown, many compiles
./codexion 5 1000 100 100 100 20 0 edf
```

### Expected output format

```
0 1 has taken a dongle
2 1 has taken a dongle
2 1 is compiling
202 1 is debugging
402 1 is refactoring
405 2 has taken a dongle
406 2 has taken a dongle
406 2 is compiling
606 2 is debugging
806 2 is refactoring
1505 4 burned out
```

Each line: `timestamp_in_ms  coder_number  message`

## Blocking Cases Handled

### Deadlock prevention (Coffman's conditions)

A deadlock in the classic dining philosophers problem arises when every coder holds
one dongle and waits forever for the second. Codexion avoids this through **central
arbitration**: a coder only acquires its two dongles atomically. Before touching any
dongle, the coder enqueues a request on the shared scheduler, then waits on
`sched_cond`. The scheduler grants access only when both neighbouring dongles are
free and their cooldown has expired. Because no coder ever holds one dongle while
blocked on the other, circular waiting cannot form.

Coffman's four conditions analysis:
- **Mutual exclusion** — inherent (dongles are exclusive hardware).
- **Hold and wait** — eliminated: acquisition is all-or-nothing.
- **No preemption** — not needed since coders never partially hold.
- **Circular wait** — eliminated by the central queue preventing cyclic blocking.

### Starvation prevention

Under `fifo`, requests are served strictly in arrival order, so every waiting coder
eventually reaches the front of the queue.

Under `edf`, the coder with the earliest `last_compile_start + time_to_burnout`
deadline is served first. This maximises the number of coders that meet their deadline.
Tie-breaking by coder id makes the policy fully deterministic and prevents indefinite
postponement.

### Dongle cooldown

When a coder releases a dongle, `available_at` is set to `now + dongle_cooldown`.
The scheduler checks `available_at` before granting the dongle. If the dongle is still
cooling down, `pthread_cond_timedwait` is used with a deadline matching `available_at`,
so threads sleep exactly as long as needed without busy-waiting.

### Precise burnout detection

A dedicated **monitor thread** loops every ~1 ms (using `usleep(1000)`), reads each
coder's `last_compile_start` under `sched_lock`, and compares it to the current
timestamp. If `now - last_compile_start >= time_to_burnout` and the coder has not
yet started the required number of compiles, burnout is declared. The log is printed
immediately under `print_lock`, well within the 10 ms tolerance required by the subject.

### Log serialization

All writes to stdout go through a single `printf`/`write` call protected by
`print_lock`. No message can interleave with another. The monitor checks
`sim_running` before printing so no stale message appears after the simulation ends.

## Thread Synchronization Mechanisms

### Thread layout

| Thread           | Count              | Role                                      |
|------------------|--------------------|-------------------------------------------|
| Coder threads    | `number_of_coders` | Compile → debug → refactor loop           |
| Monitor thread   | 1                  | Burnout detection and simulation teardown |

### Mutexes

**`sched_lock`** — the central scheduler mutex. Protects:
- `dongle.is_used` and `dongle.available_at` for every dongle
- the priority queue (waiting requests)
- `coder.nb_compiles` and `coder.last_compile_start`
- the global `sim_running` flag

Rule: always acquired *after* `print_lock` when both are needed, never held during
`usleep` or `sleep_ms` calls.

**`print_lock`** — serializes all terminal output. Acquired for the duration of a
single `printf` or `write` call, then immediately released. Always the outermost lock
when nesting occurs.

### Condition variable

**`sched_cond`** is broadcast every time dongle state changes:
- a coder releases one or both dongles
- a cooldown timer expires
- `sim_running` is set to 0

Waiting coders call `pthread_cond_timedwait` when a cooldown deadline is known, or
`pthread_cond_wait` otherwise. On each wake-up they re-evaluate whether their
assigned dongles are available; if not, they sleep again (mesa-style re-check).

### Race condition prevention — concrete example

```
Monitor thread              Coder thread
──────────────              ────────────
lock(sched_lock)            lock(sched_lock)
read last_compile_start     write last_compile_start = now
read nb_compiles            write nb_compiles++
unlock(sched_lock)          broadcast(sched_cond)
                            unlock(sched_lock)
```

Both sides hold `sched_lock` before touching these fields, so the monitor always
sees a consistent snapshot and can never read a deadline that is being updated
concurrently.

### Simulation stop propagation

When the monitor sets `sim_running = 0`, it broadcasts on `sched_cond` while still
holding `sched_lock`. Every coder that wakes from `pthread_cond_wait` or
`pthread_cond_timedwait` checks `sim_running` as the first condition in its loop and
exits cleanly. `pthread_join` in `main` then collects all threads before freeing memory.

### Priority queue (heap)

FIFO and EDF scheduling are both implemented on top of a **min-heap** (no standard
library queue used). Each entry stores the coder id and a sort key:
- FIFO: the arrival timestamp (monotonically increasing → equivalent to a queue)
- EDF: `last_compile_start + time_to_burnout`

The heap is protected by `sched_lock` and rebuilt on every insert/extract
(`O(log n)`), keeping arbitration predictable regardless of thread ordering.

## Valgrind Testing

The project links against `libpthread`. To test properly on Linux, always pass
`--fair-sched=yes` so Valgrind's scheduler gives all threads a chance to run.

### 1. Memory leak detection

```sh
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes \
         ./codexion 4 800 200 200 200 3 10 fifo
```

| Valgrind output               | What it means in this project                                             |
|-------------------------------|---------------------------------------------------------------------------|
| `definitely lost: N bytes`    | `malloc` result was never `free`d — likely a dongle or coder array not freed in cleanup |
| `indirectly lost: N bytes`    | A struct that was lost also owned heap children — e.g., the scheduler queue nodes |
| `still reachable: N bytes`    | Memory still pointed-to at exit — typically the `t_sim` root struct freed too late or not at all |
| `possibly lost: N bytes`      | Interior pointer; usually a false positive with pthreads stacks, but worth checking if large |

Common sources:
- `t_coder` array or `t_dongle` array allocated in `init.c` but not freed in `cleanup.c`
- scheduler queue nodes (heap array) not freed before program exit
- `pthread_attr_t` initialised but `pthread_attr_destroy` never called

### 2. Thread error detection (data races, invalid locks)

```sh
valgrind --tool=helgrind --fair-sched=yes \
         ./codexion 4 800 200 200 200 3 10 fifo
```

| Helgrind report                          | What it means                                                                |
|------------------------------------------|------------------------------------------------------------------------------|
| `Possible data race on …`                | A variable is read or written without holding the expected mutex             |
| `Lock order violated`                    | Two mutexes were acquired in different orders in different threads — potential deadlock |
| `Thread #N: lock already held`           | Double-lock on a non-recursive mutex → undefined behaviour / deadlock        |
| `Mutex is still locked at thread exit`   | A coder thread exited while still owning `sched_lock` or `print_lock`        |

Common sources:
- Reading `sim_running` or `nb_compiles` outside `sched_lock`
- Printing a log line without holding `print_lock`
- Acquiring `sched_lock` then `print_lock` in one place and the reverse order elsewhere

### 3. Undefined behaviour and invalid memory accesses

```sh
valgrind --tool=memcheck --track-origins=yes --fair-sched=yes \
         ./codexion 4 800 200 200 200 3 10 fifo
```

| Valgrind output                           | What it means                                                               |
|-------------------------------------------|-----------------------------------------------------------------------------|
| `Invalid read of size N`                  | Reading freed or out-of-bounds memory — likely a coder struct accessed after `free` in teardown |
| `Invalid write of size N`                 | Writing to freed/out-of-bounds memory — heap corruption in the priority queue |
| `Use of uninitialised value`              | A field (e.g., `last_compile_start`) read before being set — missing `memset` in `init.c` |
| `Conditional jump depends on uninitialised` | EDF key computed from an unset deadline field                             |

### 4. DRD — alternative race detector

```sh
valgrind --tool=drd --fair-sched=yes \
         ./codexion 4 800 200 200 200 3 10 fifo
```

DRD is lighter than Helgrind and reports:
- `Conflicting load/store` — same race class as Helgrind's `Possible data race`
- `Mutex not locked by calling thread` — `pthread_mutex_unlock` called by a thread that does not own the mutex
- `Attempted to destroy a locked mutex` — `pthread_mutex_destroy` called in cleanup before all threads have exited

### 5. Combined recommended invocation for peer evaluation

```sh
valgrind --leak-check=full --show-leak-kinds=all \
         --track-origins=yes --fair-sched=yes \
         --error-exitcode=1 \
         ./codexion 3 800 200 200 200 3 0 fifo
echo "Exit: $?"
```

A non-zero exit from `--error-exitcode=1` means Valgrind found at least one error.
Use this in a script to fail automatically if any memory or thread issue is detected.

### Notes on false positives with pthreads

`valgrind --tool=helgrind` sometimes reports spurious races on internal glibc pthread
structures (e.g., the `pthread_cond_t` internals). These show file paths such as
`/usr/include/bits/pthreadtypes.h` and can safely be suppressed with a `.supp` file:

```sh
valgrind --tool=helgrind --suppressions=pthread.supp --fair-sched=yes \
         ./codexion 4 800 200 200 200 3 10 fifo
```

Always verify that suppressed errors are genuinely glibc internals and not your own
code before ignoring them.

## Resources

- Visual : https://codexionvisualizer.dev/
- Structure : https://zestedesavoir.com/tutoriels/755/le-langage-c-1/1043_aggregats-memoire-et-fichiers/4279_structures/
- Thread/Mutex : https://www.codequoi.com/threads-mutex-et-programmation-concurrente-en-c/
- The `tester.sh` script was created by [Overtek](https://github.com/Overtekk).


**AI usage:** AI was used to:

- Identify bugs
- Generate the initial structure of this README, which was then reviewed and approved by the author.

All logic, architectural decisions, and code were written and understood by the author. The results generated by the AI were always reviewed, tested, and validated before use.
