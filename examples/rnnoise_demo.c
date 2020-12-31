
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include "rnnoise-nu.h"

#define FRAME_SIZE 480

int main(int argc, char *argv[])
{
    int i, ci;
    int first = 1;
    int channels = 1;
    float x[FRAME_SIZE];
    short *tmp;
    int sample_rate;
    RNNModel *model = NULL;
    DenoiseState **sts;
    float max_attenuation;

    FILE *fidr = fopen(argv[1], "rb");
    FILE *fidw = fopen(argv[2], "wb");

    sample_rate = atoi(argv[3]); // 第一个参数采样率
    if (sample_rate <= 0) sample_rate = 48000;    
    max_attenuation = pow(10, -atof(argv[4])/10); // 最大衰减

    sts = malloc(channels * sizeof(DenoiseState *));
    if (!sts) {
        perror("malloc");
        return 1;
    }
    tmp = malloc(channels * FRAME_SIZE * sizeof(short));
    if (!tmp) {
        perror("malloc");
        return 1;
    }
    for (i = 0; i < channels; i++) {
        sts[i] = rnnoise_create(model);
        rnnoise_set_param(sts[i], RNNOISE_PARAM_MAX_ATTENUATION, max_attenuation);
        rnnoise_set_param(sts[i], RNNOISE_PARAM_SAMPLE_RATE, sample_rate);
    }
    while (1) {
    fread(tmp, sizeof(short), channels * FRAME_SIZE, fidr);
    if (feof(fidr)) break;

    for (ci = 0; ci < channels; ci++) {
        for (i=0;i<FRAME_SIZE;i++) x[i] = tmp[i*channels+ci];
        rnnoise_process_frame(sts[ci], x, x);
        for (i=0;i<FRAME_SIZE;i++) tmp[i*channels+ci] = x[i];
    }

    if (!first) fwrite(tmp, sizeof(short), channels * FRAME_SIZE, fidw);
        first = 0;
    }

    for (i = 0; i < channels; i++)
        rnnoise_destroy(sts[i]);
    free(tmp);
    free(sts);

    fclose(fidr);
    fclose(fidw);

    return 0;
}



