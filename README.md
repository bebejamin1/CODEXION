*This project has been created as part of the 42 curriculum by bbeaurai.*

# Codexion

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

- `pthread_create(3)`, `pthread_join(3)`, `pthread_mutex_*(3)`, `pthread_cond_*(3)` man pages — core threading primitives used throughout.
- `gettimeofday(2)` man page — millisecond timestamp implementation.
- *The Little Book of Semaphores* — Allen B. Downey — dining philosophers analysis and synchronization patterns.
- *Operating Systems: Three Easy Pieces* — Arpaci-Dusseau — chapters on concurrency, locks, and condition variables.
- Valgrind User Manual — `https://valgrind.org/docs/manual/` — Memcheck, Helgrind, DRD tools.
- 42 Norm documentation.
- The Codexion subject (version 1.4) and peer-evaluation scale.

**AI usage:** Claude Code was used as a review and debugging assistant on the following parts:

- Identifying a spurious burnout bug where a finished coder was incorrectly flagged (off-by-one in the deadline check).
- Reviewing lock ordering to confirm no deadlock between `print_lock` and `sched_lock`.
- Comparing output format against the subject example line by line.
- Suggesting the `pthread_cond_timedwait` approach for cooldown wake-up to avoid busy-waiting.
- Generating the initial structure of this README, then reviewed and validated by the author.

All logic, architecture decisions, and code were written and understood by the author. AI output was always reviewed, tested, and validated before use.


---

# Complete Code Walkthrough — Line by Line

## General Concept

**Coders** sit in a circle around a table. Between each pair of neighbours there is a **USB dongle**. To compile, a coder needs its **2 neighbouring dongles AT THE SAME TIME**. After compiling, it debugs, refactors, then starts over. If it fails to compile before the deadline (`time_to_burnout`), it **burns out** and the simulation stops.

```
      Coder 1
     /       \
 dongle5   dongle1
   /             \
Coder 5       Coder 2
   \             /
 dongle4   dongle2
     \       /
      Coder 4 — dongle3 — Coder 3
```

> Coder 1 needs `dongle5` **AND** `dongle1` simultaneously.
> Coder 1 and Coder 3 can compile **AT THE SAME TIME** (no shared dongle).

---

## File Architecture

```
codexion/
├── main.c                ← Entry point
├── inc/codexion.h        ← All structures and prototypes
├── Makefile
└── src/
    ├── validate.c        ← Validates arguments
    ├── init.c            ← Allocates memory, initialises mutexes/coders/dongles
    ├── threads.c         ← Creates threads, pre-registers the queue
    ├── routine.c         ← Main loop of a coder
    ├── scheduler.c       ← Requests/releases dongles, handles waiting
    ├── scheduler_queue.c ← Priority queue (min-heap) + FIFO/EDF
    ├── scheduler_checks.c← Checks whether a coder can compile
    ├── scheduler_time.c  ← Computes cooldown timeouts
    ├── monitor.c         ← Thread watching for burnout and simulation end
    ├── actions.c         ← All display functions
    ├── cleanup.c         ← Frees memory and destroys mutexes
    └── utils.c           ← ft_atoi, is_number, current_time_ms, sleep_ms
```

---

## Data Structures — `inc/codexion.h`

### `t_dongle`
| Field | Type | Role |
|---|---|---|
| `is_used` | int | 1 if a coder is currently using it |
| `available_at` | long long | Timestamp (ms) of availability after cooldown |

### `t_coder`
| Field | Type | Role |
|---|---|---|
| `id` | int | Coder number (1..N) |
| `thread_id` | pthread_t | Its POSIX thread |
| `nb_compiles` | int | Number of compilations completed |
| `last_compile_start` | long long | Timestamp of the last compile start |
| `left_dongle` | t_dongle* | Pointer to the left dongle |
| `right_dongle` | t_dongle* | Pointer to the right dongle |
| `data` | t_data* | Pointer to global state |

### `t_data` — global simulation state

```
PARAMETERS (argv):
  nb_coders, time_to_burnout, time_to_compile, time_to_debug,
  time_to_refactor, nb_compiles_req, dongle_cooldown,
  scheduler_type (0=FIFO, 1=EDF), start_time

SYNCHRONISATION:
  pthread_mutex_t print_lock  → protects all stdout output
  pthread_mutex_t sched_lock  → protects the queue, dongles, sim_running
  pthread_cond_t  sched_cond  → wakes up waiting coders

PRIORITY QUEUE (min-heap):
  int *wait_queue   → coder IDs in the heap
  int *wait_order   → wait_order[i] = FIFO order of coder i+1
  int  wait_count   → number of coders in the queue
  int  next_order   → next FIFO order number

  t_dongle *dongles, t_coder *coders, int sim_running
```

**Lock order — always respected:**
```
print_lock  ──────►  sched_lock
```
`print_lock` is always acquired **before** `sched_lock`. Never the other way around.

---

## `main.c`

```
main()
├── validate_args(argc, argv)
├── malloc(sizeof(t_data))
├── set_data_from_args()      ← copies argv into data
├── start_simulation()
│     ├── init_system(data)   ← allocates dongles, coders, mutexes
│     └── init_threads()      ← launches all threads
├── wait_threads()            ← pthread_join on all threads
├── cleanup_system(data)
└── free(data)
```

---

## `src/validate.c`

- `argc != 9` → prints usage and returns 0
- `argv[1..5]`: integer **> 0** required
- `argv[6]` (nb_compiles_req): may be **0** (= infinite)
- `argv[7]` (dongle_cooldown): may be **≥ 0**
- `argv[8]`: must be exactly `"fifo"` or `"edf"`

---

## `src/init.c`

```
init_system(data)
├── allocate_system()
│     ├── malloc → N t_dongle
│     ├── malloc → N t_coder
│     ├── calloc → wait_queue  (zero-initialised)
│     └── calloc → wait_order  (zero-initialised)
├── init_sync_objects()
│     ├── pthread_mutex_init(&print_lock)
│     ├── pthread_mutex_init(&sched_lock)
│     └── pthread_cond_init(&sched_cond)
├── init_coders()   [loop i = 0..N-1]
│     ├── coders[i].id           = i + 1
│     ├── coders[i].left_dongle  = &dongles[i]
│     └── coders[i].right_dongle = &dongles[(i+1) % N]  ← circular
└── wait_count = 0 | next_order = 1 | sim_running = 1
```

---

## `src/threads.c`

```
init_threads(data, monitor)
├── start_time = current_time_ms()          ← T = 0
├── LOCK sched_lock
│   ├── last_compile_start = start_time for each coder
│   └── enqueue_all(data)
│         ├── First registers ODD coders: 1, 3, 5, ...
│         │     → FIFO orders 1, 2, 3, ... (highest priority)
│         └── Then EVEN coders: 2, 4, 6, ...
├── UNLOCK sched_lock
├── pthread_create ×N → coder_routine()
└── pthread_create    → monitor_thread()
```

**Why pre-register odd coders first?**
Without this step, the order depends on the OS scheduler. Even coders could run before their odd neighbours. Pre-registration guarantees that coder 1 always has priority 1, coder 3 priority 2, etc.

---

## `src/routine.c`

```
coder_routine()  ← each thread executes this
│
└── LOOP (while simulation is active):
      ├── 1. scheduler_request()      → waits for its 2 dongles
      ├── 2. print "X is compiling"
      ├── 3. sleep_ms(time_to_compile)
      ├── 4. count_compile()          → LOCK / nb_compiles++ / UNLOCK
      ├── 5. scheduler_release()      → releases dongles + cooldown + broadcast
      ├── 6. print "X is debugging"   → sleep_ms(time_to_debug)
      └── 7. print "X is refactoring" → sleep_ms(time_to_refactor)
```

---

## `src/scheduler.c`

### `scheduler_request`
```
LOCK sched_lock
sched_enqueue()           ← registers itself (no-op if already registered)

INFINITE LOOP:
  ├── !sim_running → dequeue, UNLOCK, return 0
  ├── try_acquire() == 1 → break (dongles acquired)
  └── pthread_cond_wait(&sched_cond, &sched_lock)

UNLOCK sched_lock
```

### `try_acquire` — atomic acquisition
```
IF coder_can_compile() AND priority_is_clear():
  ├── left_dongle->is_used  = 1   ← atomic under sched_lock
  ├── right_dongle->is_used = 1
  ├── last_compile_start    = now
  ├── sched_dequeue()
  ├── broadcast(sched_cond)
  ├── UNLOCK sched_lock
  ├── print_dongle_taken()        ← "X has taken a dongle" × 2 (same timestamp)
  ├── LOCK sched_lock
  └── return 1
ELSE return 0
```

### `scheduler_release`
```
LOCK sched_lock
  left/right dongle->is_used    = 0
  left/right dongle->available_at = now + dongle_cooldown
  broadcast(sched_cond)
UNLOCK sched_lock
```

---

## `src/scheduler_queue.c`

**Min-Heap**: the highest priority is always at index 0.

```
FIFO example, 4 coders pre-registered:
wait_queue = [1, 3, 2, 4]   (odd coders first)
         1
        / \
       3   2
      /
     4
```

### `priority_before(first_id, second_id)`

**FIFO:** `wait_order[first_id-1] < wait_order[second_id-1]`

**EDF:**
```
deadline = last_compile_start + time_to_burnout
→ nearest deadline = highest priority
→ tie: lowest ID wins
```

---

## `src/scheduler_checks.c`

### `coder_can_compile(coder)`
Returns 1 if all **4 conditions** are true:
- `left_dongle->is_used == 0`
- `right_dongle->is_used == 0`
- `now >= left_dongle->available_at`
- `now >= right_dongle->available_at`

### `shares_dongle(first_id, second_id)`
```
first_left=first_id-1, first_right=first_id%N
second_left=second_id-1, second_right=second_id%N
→ conflict if any of the 4 combinations are equal

Example N=5: coders 4 and 5
  coder4: (3, 4)   |   coder5: (4, 0)
  → right(4) == left(5) == 4  → CONFLICT ✗
```

### `priority_is_clear(my_id)`
```
For each other_id in wait_queue:
  IF shares_dongle(my_id, other_id)
  AND priority_before(other_id, my_id)
  AND coder_can_compile(other_id)
  → return 0  (I must wait)

return (wait_order[my_id-1] != 0)
```

---

## `src/monitor.c`

```
monitor_thread()
└── while (monitor_cycle())
         usleep(100)   ← checks every 100 µs

monitor_cycle()
├── LOCK sched_lock
├── !sim_running → UNLOCK, return 0
├── check_all_burnouts()
│     For each coder:
│       now - last_compile_start > time_to_burnout ?
│         → sim_running=0 → broadcast → UNLOCK
│         → print "X burned out" (red)
├── all_done() ? → if nb_compiles_req > 0 and all ≥ req → true
├── if done: sim_running=0 → broadcast → UNLOCK
└── if done: LOCK print_lock → "All coders have compiled." → UNLOCK
```

The monitor's periodic broadcast (every 100 µs) also acts as a regular wake-up
for coders waiting for a dongle's cooldown to expire.

---

## `src/actions.c`

All display functions:
```
LOCK print_lock
ts = current_time_ms() - start_time
if (sim_running OR msg[0]=='b') → printf + fflush
UNLOCK print_lock
```

| Function | Message | Note |
|---|---|---|
| `print_state()` | `"123 4 is compiling"` | `'b'` → red, printed even after stop |
| `print_dongle_taken()` | `"123 4 has taken a dongle"` × 2 | Two lines within **the same lock** → same timestamp |
| `print_compile()` | `"123 4 is compiling"` | |
| `simulation_is_running()` | — | Lock / read sim_running / unlock |

---

## `src/cleanup.c`

```
cleanup_system(data)
├── pthread_mutex_destroy(&print_lock)
├── pthread_mutex_destroy(&sched_lock)
├── pthread_cond_destroy(&sched_cond)
├── free(dongles), free(coders)
└── free(wait_queue), free(wait_order)
```

---

## `src/utils.c`

| Function | Description |
|---|---|
| `ft_atoi(s)` | Handles spaces and +/− sign, converts to int |
| `is_number(s)` | Checks that the string contains only digits |
| `current_time_ms()` | `gettimeofday` → ms since UNIX Epoch |
| `sleep_ms(ms)` | `usleep(500)` loop until target time (~0.5 ms precision) |

---

## Full Execution Flow

```
MAIN THREAD (main)               CODER THREADS                 MONITOR THREAD
──────────────────               ─────────────────────         ───────────────
validate_args()
init_system()
init_threads()
  start_time = now
  pre-register queue
  pthread_create ×N ──────────► coder_routine()
  pthread_create ──────────────────────────────────────────► monitor_thread()
                                  scheduler_request()              loop:
wait_threads()                      try_acquire?               check_burnout()
  pthread_join ×(N+1)               → yes: takes dongles       all_done()?
                                    → no: cond_wait            broadcast()
cleanup_system()                  print + compile              usleep(100)
free(data)                        sleep(compile/debug/refactor)
```

---

## Stop Conditions

| Condition | Trigger | Message |
|---|---|---|
| **Burnout** | `now - last_compile_start > time_to_burnout` | `X burned out` (red) |
| **All compiled** | `nb_compiles_req > 0` and all ≥ req | `All coders have compiled.` (green) |
| **Infinite** | `nb_compiles_req == 0` | Runs until a burnout |

---

## Deadlock Prevention — Coffman's Conditions

| Condition | Status | Explanation |
|---|---|---|
| Mutual exclusion | Inherent | Dongles are exclusive |
| **Hold and Wait** | **Eliminated ✓** | Atomic acquisition of both dongles under `sched_lock` |
| No preemption | Not needed | Never a partial hold |
| **Circular wait** | **Eliminated ✓** | Centralised priority queue |

---

## Priority Policy — FIFO vs EDF

**FIFO**: priority = arrival order in the queue. Pre-registration of odd→even coders guarantees that odd-numbered coders compile first in round 1.

**EDF**: priority = burnout urgency (`last_compile_start + time_to_burnout`). On a tie, the lowest ID wins. Maximises the number of coders that meet their deadline.
