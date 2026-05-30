*This project has been created as part of the 42 curriculum by bbeaurai.*

# Codexion

## Description

Codexion is a POSIX threads simulation inspired by the dining philosophers
problem. Several coders sit around a shared workspace and need two USB dongles
to compile. After compiling, each coder debugs, refactors, and tries to compile
again.

The simulation stops when one coder burns out because they did not start
compiling before their deadline, or when every coder has compiled at least the
required number of times.

## Instructions

Build the project:

```sh
make
```

Run it:

```sh
./codexion number_of_coders time_to_burnout time_to_compile time_to_debug time_to_refactor number_of_compiles_required dongle_cooldown scheduler
```

Example:

```sh
./codexion 4 800 200 200 200 5 10 fifo
```

The scheduler must be either `fifo` or `edf`.

Clean generated files:

```sh
make clean
make fclean
make re
```

## Blocking Cases Handled

Deadlock prevention: coders never keep only one dongle while waiting for the
second one. A coder starts compiling only when both required dongles are
available.

Coffman conditions: mutual exclusion still exists because dongles are shared
exclusive resources, but circular waiting is avoided by central arbitration and
atomic acquisition of both dongles.

Starvation prevention: pending requests are ordered by the selected scheduler.
`fifo` keeps arrival order, while `edf` gives priority to the coder with the
earliest burnout deadline.

Cooldown handling: after a coder releases a dongle, it cannot be reused before
`dongle_cooldown` milliseconds have elapsed.

Precise burnout detection: a separate monitor thread checks deadlines and stops
the simulation when a coder burns out.

Log serialization: all output is protected by a print mutex so messages never
interleave on the same line.

## Thread Synchronization Mechanisms

Each coder is represented by a `pthread_t`. The monitor runs in a dedicated
`pthread_t` and polls every 100 µs to detect burnout within the required 10 ms
window.

Two `pthread_mutex_t` protect shared state:

- `sched_lock` guards dongle availability (`is_used`, `available_at`), the
  scheduler queue, compile counters, and the `sim_running` flag. Coders hold
  this lock only while checking or updating shared state, never during
  `sleep_ms` calls, preventing long blocking of other threads.
- `print_lock` serializes all terminal output. It is always acquired before
  `sched_lock` when both are needed, enforcing a consistent lock order and
  preventing deadlock.

One `pthread_cond_t` (`sched_cond`) is broadcast whenever dongle state changes
(release, cooldown expiry, or simulation stop), waking all waiting coders so
each can re-evaluate whether it can compile. `pthread_cond_timedwait` is used
when a dongle cooldown is known in advance, avoiding busy-waiting.

Race condition prevention example: the monitor reads `last_compile_start` and
`nb_compiles` under `sched_lock`. Coders update these fields under the same
lock before broadcasting on `sched_cond`. This guarantees the monitor never
reads a partially updated deadline.

Thread-safe communication between coders and the monitor: when the monitor
detects burnout or all-done, it sets `sim_running = 0` under `sched_lock` and
broadcasts on `sched_cond`. Waiting coders wake, observe `sim_running == 0`,
and exit their loops cleanly without further lock contention.

## Resources

- `pthread_create(3)`, `pthread_join(3)`, `pthread_mutex_*(3)`, and
  `pthread_cond_*(3)` manual pages — core threading primitives used throughout.
- `gettimeofday(2)` manual page — millisecond timestamp implementation.
- *The Little Book of Semaphores* by Allen B. Downey — dining philosophers
  problem analysis and synchronization patterns.
- *Operating Systems: Three Easy Pieces* (Arpaci-Dusseau) — chapters on
  concurrency, locks, and condition variables.
- 42 Norm documentation.
- The Codexion subject and peer-evaluation scale.

AI was used as a review and debugging assistant on the following parts:

- Identifying the spurious burnout bug where a finished coder was incorrectly
  flagged (off-by-one in the deadline check).
- Reviewing lock ordering to confirm no deadlock between `print_lock` and
  `sched_lock`.
- Comparing output format against the subject example line by line.
- Suggesting the `pthread_cond_timedwait` approach for cooldown wake-up to
  avoid busy-waiting.

All logic, architecture decisions, and code were written and understood by the
author. AI output was always reviewed, tested, and validated before use.
