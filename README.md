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

Each coder is represented by a `pthread_t`. The monitor is also a separate
thread.

`pthread_mutex_t` is used to protect shared simulation state, scheduler data,
dongle state, and log output. This prevents race conditions when coders request
or release dongles and when the monitor reads compile counters and deadlines.

`pthread_cond_t` is used to wake waiting coders when dongles are released, when a
cooldown expires, or when the simulation stops.

The scheduler queue coordinates access to dongles. It ensures that two
neighbouring coders cannot duplicate the same dongle and that a coder only logs
`is compiling` after two `has taken a dongle` messages.

## Resources

- `pthread_create`, `pthread_join`, `pthread_mutex_*`, and `pthread_cond_*`
  manual pages.
- `gettimeofday(2)` manual page for millisecond timestamps.
- 42 Norm documentation.
- The Codexion subject and peer-evaluation scale.
- AI was used as a review and debugging assistant to identify edge cases,
  compare behavior with the subject, and improve tests and documentation. The
  project logic and code remain reviewed and understood by the author.
