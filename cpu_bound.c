#include <stdio.h>
#include <time.h>
#include <math.h>

int main() {
    printf("[cpu_bound] iniciado, consumindo CPU (20s de tempo de CPU)...\n");

    struct timespec start, now;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);

    // alvo: ~20s de tempo de CPU (não tempo de parede)
    const double target_ms = 20000.0;

    for (;;) {
        // trabalho pesado para consumir CPU
        double x = 0.0;
        for (int i = 0; i < 1000000; i++) {
            x += sqrt((double)i);
        }

        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &now);
        double elapsed_ms =
            (now.tv_sec - start.tv_sec) * 1000.0 +
            (now.tv_nsec - start.tv_nsec) / 1e6;

        if (elapsed_ms >= target_ms) break;
    }

    printf("[cpu_bound] terminou após ~20s de tempo de CPU\n");
    return 0;
}
