#include "aacencoder.h"

int AAC_EncoderInit(AuAACEncoder_t *encoder, int samplerate, int numChannels, int bitsPerSample) {
	if (!encoder) {
		return -1;
	}

	faacEncHandle hEncoder = faacEncOpen(samplerate, numChannels, &encoder->inputSamples, &encoder->maxOutputBytes);
	if (!hEncoder) {
		return -2;
	}

	faacEncConfigurationPtr config = faacEncGetCurrentConfiguration(hEncoder);
	if (!config) {
        faacEncClose(hEncoder);
		return -3;
    }

	// Configure the encoder
	switch (bitsPerSample) {
	case 16: config->inputFormat = FAAC_INPUT_16BIT;
	break;
	case 24: config->inputFormat = FAAC_INPUT_24BIT;
	break;
	case 32: config->inputFormat = FAAC_INPUT_32BIT;
	break;
	default: {
		faacEncClose(hEncoder);
		return -4;
	}
	break;
	}
	
    config->mpegVersion = MPEG4; // Use MPEG-4 AAC (common choice)
    config->aacObjectType = LOW; // Low Complexity (LC) AAC, widely compatible
    config->outputFormat = 1;    // 1 = Raw AAC (no ADTS headers), adjust if needed
    config->bitRate = 32000;     // Bitrate in bits/sec (e.g., 32kbps for mono 8kHz)
    config->quantqual = 100;     // Quality (1-1000, 100 is default)

	// Apply the configuration
    if (faacEncSetConfiguration(hEncoder, config) < 0) {
        faacEncClose(hEncoder);
        return false;
    }

	encoder->hEncoder = hEncoder;

    return 0;
}

void AAC_EncoderDeInit(AuAACEncoder_t *encoder) {
	if (!encoder || !encoder->hEncoder) {
		return;
	}
	faacEncClose(encoder->hEncoder);
}

int AAC_EncodePCM(AuAACEncoder_t *encoder, uint8_t *pcmBuffer, int pcmBufferSize, std::vector<uint8_t>& aacBuffer) {
	if (!encoder || !encoder->hEncoder) {
		return -1;
	}

	if (!pcmBuffer || pcmBuffer <= 0) {
		return -2;
	}

	int numSamples = pcmBufferSize / 2;
    if (numSamples > (int)encoder->inputSamples) {
        printf("Input PCM size exceeds encoder's maximum input samples\n");
        return -3;
    }

	if (aacBuffer.size() < encoder->maxOutputBytes) {
		aacBuffer.resize(encoder->maxOutputBytes);
	}

	int bytesWritten = faacEncEncode(encoder->hEncoder, (int32_t*)pcmBuffer, numSamples, &aacBuffer[0], encoder->maxOutputBytes);

	printf("[OPER] Convert PCM size %5d to AAC %5d\r\n", pcmBufferSize, bytesWritten);

	// At this point, aacOutput.data() contains the encoded AAC data
    // bytesWritten is the size of the AAC data in bytes
	return bytesWritten;
}
