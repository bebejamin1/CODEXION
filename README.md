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

# Explication complète du code — ligne par ligne

## Concept général

Des **coders** sont assis en cercle autour d'une table. Entre chaque paire de voisins se trouve une **clé USB (dongle)**. Pour compiler, un coder a besoin de ses **2 dongles voisins EN MÊME TEMPS**. Après compilation, il débogue, refactorise, puis recommence. S'il n'arrive pas à compiler avant le délai (`time_to_burnout`), il **brûle** et la simulation s'arrête.

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

> Coder 1 a besoin de `dongle5` **ET** `dongle1` simultanément.
> Coder 1 et Coder 3 peuvent compiler **EN MÊME TEMPS** (aucun dongle partagé).

---

## Architecture des fichiers

```
codexion/
├── main.c                ← Point d'entrée
├── inc/codexion.h        ← Toutes les structures et prototypes
├── Makefile
└── src/
    ├── validate.c        ← Vérifie les arguments
    ├── init.c            ← Alloue mémoire, initialise mutexes/coders/dongles
    ├── threads.c         ← Crée les threads, pré-enregistre la queue
    ├── routine.c         ← Boucle principale d'un coder
    ├── scheduler.c       ← Demande/libère les dongles, gère l'attente
    ├── scheduler_queue.c ← File de priorité (min-heap) + FIFO/EDF
    ├── scheduler_checks.c← Vérifie si un coder peut compiler
    ├── scheduler_time.c  ← Calcul des timeouts de cooldown
    ├── monitor.c         ← Thread surveillant burnout et fin de simulation
    ├── actions.c         ← Toutes les fonctions d'affichage
    ├── cleanup.c         ← Libère la mémoire et détruit les mutexes
    └── utils.c           ← ft_atoi, is_number, current_time_ms, sleep_ms
```

---

## Structures de données — `inc/codexion.h`

### `t_dongle`
| Champ | Type | Rôle |
|---|---|---|
| `is_used` | int | 1 si un coder l'utilise en ce moment |
| `available_at` | long long | Timestamp (ms) de disponibilité après cooldown |

### `t_coder`
| Champ | Type | Rôle |
|---|---|---|
| `id` | int | Numéro du coder (1..N) |
| `thread_id` | pthread_t | Son thread POSIX |
| `nb_compiles` | int | Nombre de compilations effectuées |
| `last_compile_start` | long long | Timestamp du dernier début de compile |
| `left_dongle` | t_dongle* | Pointeur vers le dongle gauche |
| `right_dongle` | t_dongle* | Pointeur vers le dongle droit |
| `data` | t_data* | Pointeur vers l'état global |

### `t_data` — état global de toute la simulation

```
PARAMÈTRES (argv) :
  nb_coders, time_to_burnout, time_to_compile, time_to_debug,
  time_to_refactor, nb_compiles_req, dongle_cooldown,
  scheduler_type (0=FIFO, 1=EDF), start_time

SYNCHRONISATION :
  pthread_mutex_t print_lock  → protège tout affichage stdout
  pthread_mutex_t sched_lock  → protège la queue, les dongles, sim_running
  pthread_cond_t  sched_cond  → réveille les coders en attente

FILE DE PRIORITÉ (min-heap) :
  int *wait_queue   → IDs des coders dans la heap
  int *wait_order   → wait_order[i] = ordre FIFO du coder i+1
  int  wait_count   → nombre de coders dans la queue
  int  next_order   → prochain numéro d'ordre FIFO

  t_dongle *dongles, t_coder *coders, int sim_running
```

**Ordre des verrous — toujours respecté :**
```
print_lock  ──────►  sched_lock
```
On prend toujours `print_lock` **avant** `sched_lock`. Jamais l'inverse.

---

## `main.c`

```
main()
├── validate_args(argc, argv)
├── malloc(sizeof(t_data))
├── set_data_from_args()      ← copie argv dans data
├── start_simulation()
│     ├── init_system(data)   ← alloue dongles, coders, mutexes
│     └── init_threads()      ← lance tous les threads
├── wait_threads()            ← pthread_join sur tous les threads
├── cleanup_system(data)
└── free(data)
```

---

## `src/validate.c`

- `argc != 9` → affiche l'usage et retourne 0
- `argv[1..5]` : entier **> 0** obligatoire
- `argv[6]` (nb_compiles_req) : peut valoir **0** (= infini)
- `argv[7]` (dongle_cooldown) : peut valoir **≥ 0**
- `argv[8]` : doit être exactement `"fifo"` ou `"edf"`

---

## `src/init.c`

```
init_system(data)
├── allocate_system()
│     ├── malloc → N t_dongle
│     ├── malloc → N t_coder
│     ├── calloc → wait_queue  (zéro-initialisé)
│     └── calloc → wait_order  (zéro-initialisé)
├── init_sync_objects()
│     ├── pthread_mutex_init(&print_lock)
│     ├── pthread_mutex_init(&sched_lock)
│     └── pthread_cond_init(&sched_cond)
├── init_coders()   [boucle i = 0..N-1]
│     ├── coders[i].id           = i + 1
│     ├── coders[i].left_dongle  = &dongles[i]
│     └── coders[i].right_dongle = &dongles[(i+1) % N]  ← circulaire
└── wait_count = 0 | next_order = 1 | sim_running = 1
```

---

## `src/threads.c`

```
init_threads(data, monitor)
├── start_time = current_time_ms()          ← T = 0
├── LOCK sched_lock
│   ├── last_compile_start = start_time pour chaque coder
│   └── enqueue_all(data)
│         ├── Enregistre d'abord les coders IMPAIRS : 1, 3, 5, ...
│         │     → ordres FIFO 1, 2, 3, ... (priorité maximale)
│         └── Puis les coders PAIRS : 2, 4, 6, ...
├── UNLOCK sched_lock
├── pthread_create ×N → coder_routine()
└── pthread_create    → monitor_thread()
```

**Pourquoi pré-enregistrer impairs en premier ?**
Sans cette étape, l'ordre dépend du scheduler OS. Des coders pairs pourraient passer avant leurs voisins impairs. Le pré-enregistrement garantit que le coder 1 a toujours la priorité 1, le coder 3 la priorité 2, etc.

---

## `src/routine.c`

```
coder_routine()  ← chaque thread exécute ceci
│
└── BOUCLE (tant que simulation active) :
      ├── 1. scheduler_request()      → attend ses 2 dongles
      ├── 2. print "X is compiling"
      ├── 3. sleep_ms(time_to_compile)
      ├── 4. count_compile()          → LOCK / nb_compiles++ / UNLOCK
      ├── 5. scheduler_release()      → libère dongles + cooldown + broadcast
      ├── 6. print "X is debugging"   → sleep_ms(time_to_debug)
      └── 7. print "X is refactoring" → sleep_ms(time_to_refactor)
```

---

## `src/scheduler.c`

### `scheduler_request`
```
LOCK sched_lock
sched_enqueue()           ← s'inscrit (no-op si déjà inscrit)

BOUCLE INFINIE :
  ├── !sim_running → dequeue, UNLOCK, return 0
  ├── try_acquire() == 1 → break (dongles acquis)
  └── pthread_cond_wait(&sched_cond, &sched_lock)

UNLOCK sched_lock
```

### `try_acquire` — acquisition atomique
```
SI coder_can_compile() ET priority_is_clear() :
  ├── left_dongle->is_used  = 1   ← atomique sous sched_lock
  ├── right_dongle->is_used = 1
  ├── last_compile_start    = now
  ├── sched_dequeue()
  ├── broadcast(sched_cond)
  ├── UNLOCK sched_lock
  ├── print_dongle_taken()        ← "X has taken a dongle" × 2 (même timestamp)
  ├── LOCK sched_lock
  └── return 1
SINON return 0
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

**Min-Heap** : la plus haute priorité est toujours à l'index 0.

```
Exemple FIFO, 4 coders pré-enregistrés :
wait_queue = [1, 3, 2, 4]   (impairs d'abord)
         1
        / \
       3   2
      /
     4
```

### `priority_before(first_id, second_id)`

**FIFO :** `wait_order[first_id-1] < wait_order[second_id-1]`

**EDF :**
```
deadline = last_compile_start + time_to_burnout
→ deadline la plus proche = plus haute priorité
→ égalité : ID le plus bas gagne
```

---

## `src/scheduler_checks.c`

### `coder_can_compile(coder)`
Retourne 1 si les **4 conditions** sont vraies :
- `left_dongle->is_used == 0`
- `right_dongle->is_used == 0`
- `now >= left_dongle->available_at`
- `now >= right_dongle->available_at`

### `shares_dongle(first_id, second_id)`
```
first_left=first_id-1, first_right=first_id%N
second_left=second_id-1, second_right=second_id%N
→ conflit si l'une des 4 combinaisons est égale

Exemple N=5 : coders 4 et 5
  coder4 : (3, 4)   |   coder5 : (4, 0)
  → right(4) == left(5) == 4  → CONFLIT ✗
```

### `priority_is_clear(my_id)`
```
Pour chaque other_id dans wait_queue :
  SI shares_dongle(my_id, other_id)
  ET priority_before(other_id, my_id)
  ET coder_can_compile(other_id)
  → return 0  (je dois attendre)

return (wait_order[my_id-1] != 0)
```

---

## `src/monitor.c`

```
monitor_thread()
└── while (monitor_cycle())
         usleep(100)   ← vérifie toutes les 100 µs

monitor_cycle()
├── LOCK sched_lock
├── !sim_running → UNLOCK, return 0
├── check_all_burnouts()
│     Pour chaque coder :
│       now - last_compile_start > time_to_burnout ?
│         → sim_running=0 → broadcast → UNLOCK
│         → print "X burned out" (rouge)
├── all_done() ? → si nb_compiles_req > 0 et tous ≥ req → true
├── si done : sim_running=0 → broadcast → UNLOCK
└── si done : LOCK print_lock → "All coders have compiled." → UNLOCK
```

Le broadcast périodique du moniteur (toutes les 100 µs) sert aussi de réveil
régulier pour les coders attendant la fin du cooldown d'un dongle.

---

## `src/actions.c`

Toutes les fonctions d'affichage :
```
LOCK print_lock
ts = current_time_ms() - start_time
si (sim_running OU msg[0]=='b') → printf + fflush
UNLOCK print_lock
```

| Fonction | Message | Note |
|---|---|---|
| `print_state()` | `"123 4 is compiling"` | `'b'` → rouge, affiché même après arrêt |
| `print_dongle_taken()` | `"123 4 has taken a dongle"` × 2 | Deux lignes dans **le même lock** → même timestamp |
| `print_compile()` | `"123 4 is compiling"` | |
| `simulation_is_running()` | — | Lock / lit sim_running / unlock |

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

| Fonction | Description |
|---|---|
| `ft_atoi(s)` | Gère espaces et signe +/−, convertit en int |
| `is_number(s)` | Vérifie que la chaîne ne contient que des chiffres |
| `current_time_ms()` | `gettimeofday` → ms depuis l'Epoch UNIX |
| `sleep_ms(ms)` | Boucle `usleep(500)` jusqu'à l'heure cible (~0.5ms de précision) |

---

## Flux d'exécution complet

```
THREAD PRINCIPAL (main)          THREADS CODERS                THREAD MONITEUR
────────────────────────         ─────────────────────         ───────────────
validate_args()
init_system()
init_threads()
  start_time = now
  pré-enregistre queue
  pthread_create ×N ──────────► coder_routine()
  pthread_create ──────────────────────────────────────────► monitor_thread()
                                  scheduler_request()              loop:
wait_threads()                      try_acquire ?              check_burnout()
  pthread_join ×(N+1)               → oui: prend dongles      all_done() ?
                                    → non: cond_wait           broadcast()
cleanup_system()                  print + compile              usleep(100)
free(data)                        sleep(compile/debug/refactor)
```

---

## Conditions d'arrêt

| Condition | Déclencheur | Message |
|---|---|---|
| **Burnout** | `now - last_compile_start > time_to_burnout` | `X burned out` (rouge) |
| **Tous compilés** | `nb_compiles_req > 0` et tous ≥ req | `All coders have compiled.` (vert) |
| **Infini** | `nb_compiles_req == 0` | Tourne jusqu'à un burnout |

---

## Prévention du deadlock — Conditions de Coffman

| Condition | Statut | Explication |
|---|---|---|
| Exclusion mutuelle | Inhérente | Les dongles sont exclusifs |
| **Hold and Wait** | **Éliminé ✓** | Acquisition atomique des 2 dongles sous `sched_lock` |
| Pas de préemption | Non nécessaire | Jamais de hold partiel |
| **Attente circulaire** | **Éliminée ✓** | File de priorité centralisée |

---

## Politique de priorité — FIFO vs EDF

**FIFO** : priorité = ordre d'arrivée dans la file. Pré-enregistrement impairs→pairs garantit que les coders impairs compilent en premier au round 1.

**EDF** : priorité = urgence du burnout (`last_compile_start + time_to_burnout`). En cas d'égalité, l'ID le plus bas gagne. Maximise le nombre de coders qui respectent leur deadline.
