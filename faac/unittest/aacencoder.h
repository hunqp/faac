#ifndef _AAC_ENCODER_H_
#define _AAC_ENCODER_H_

#include <stdio.h>
#include <stdint.h>
#include <vector>
#include "faac.h"

typedef struct {
    faacEncHandle hEncoder;
    unsigned long maxOutputBytes;
    unsigned long inputSamples;
} AuAACEncoder_t;

extern int AAC_EncoderInit(AuAACEncoder_t *encoder, int samplerate, int numChannels, int bitsPerSample);
extern int AAC_EncodePCM(AuAACEncoder_t *encoder, uint8_t *inPcmBuffer, int inPcmBufferSize, std::vector<uint8_t>& aacBuffer);
extern void AAC_EncoderDeInit(AuAACEncoder_t *encoder);

#endif /* _AAC_ENCODER_H_ */