#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>


#include "faac.h"


//#define DEBUG(fmt, args...)
#define DEBUG(fmt, args...) 	printf(fmt, ##args)


void print_usage(const char *process)
{
	printf("sample: \n"
		   "\t %s -h\n"
		   "\t %s --help\n"
		   "\t %s -i ./audio/test_8000_16_1.pcm -r 8000 -b 16 -c 1 -o out_8khz_1ch.aac\n"
		   "\t %s --input_pcmfile=./audio/test_44100_16_2.pcm --sample_rate=44100 --sample_bits=16 --channels=2 --output_aacfile=out_44.1khz_2ch.aac\n",
		   process, process, process, process);
}


int main(int argc, char *argv[])
{
	/* 输入/输出文件 */
	FILE *fpPcm = NULL;
	FILE *fpAac = NULL;
	char pcmFileName[128] = {0};
	char aacFileName[128] = {0};

	/* PCM参数 */
	unsigned long u64PcmSampleRate = 0; // 采样率
	unsigned int  u32PcmSampleBits = 0; // 采样位数
	unsigned int  u32PcmChannels   = 0; // 声道数

	/* aac编码器相关 */
	faacEncHandle           pFaacEncHandle = NULL;
	faacEncConfigurationPtr pFaacEncConf   = NULL;

	/* 编码相关参数 */
	unsigned long u64PcmInSampleCnt = 0; // 打开编码器时传出的参数，编码传入的PCM采样数（不是字节）
	unsigned long u64AacOutMaxBytes = 0; // 打开编码器时传出的参数，编码传出最大字节数
	unsigned char *pu8PcmInBuf   = NULL; // 读取pcm并传递进去编码的缓存指针，后面根据编码器传出参数malloc分配
	unsigned char *pu8AacEncBuf  = NULL; // 编码得到的aac缓存，后面根据编码器传出参数malloc分配


	/* 判断输入参数 */
	if(argc == 1)
	{
		print_usage(argv[0]);
		return -1;
	}	

	/* 解析命令行参数 */
	char option = 0;
	int option_index = 0;
	char *short_options = "hi:r:b:c:o:";
	struct option long_options[] =
	{
		{"help",          no_argument,       NULL, 'h'},
		{"input_pcmfile", required_argument, NULL, 'i'},
		{"sample_rate",   required_argument, NULL, 'r'},
		{"sample_bits",   required_argument, NULL, 'b'},
		{"channels",      required_argument, NULL, 'c'},
		{"output_aacfile",required_argument, NULL, 'o'},
		{NULL,            0,                 NULL,  0 },
	};
	while((option = getopt_long_only(argc, argv, short_options, long_options, &option_index)) != -1)
	{
		switch(option)
		{
			case 'h':
				print_usage(argv[0]);
				return 0;
			case 'i':
				strncpy(pcmFileName, optarg, 128);
				break;
			case 'r':
				u64PcmSampleRate = atoi(optarg);
				break;
			case 'c':
				u32PcmChannels = atoi(optarg);
				break;
			case 'b':
				u32PcmSampleBits = atoi(optarg);
				break;
			case 'o':
				strncpy(aacFileName, optarg, 128);
				break;
			defalut:
				printf("Unknown argument!\n");
				break;
		}
 	}
	printf("\n**************************************\n"
		   "input: \n"
		   "\t file name: %s\n"
		   "\t sample rate: %lu Hz\n"
		   "\t sample bits: %d bits\n"
		   "\t channels: %d\n"
		   "\t bits per second: %lu bps\n"
		   "output: \n"
		   "\t file name: %s\n"
		   "**************************************\n\n",
		   pcmFileName, u64PcmSampleRate, u32PcmSampleBits, u32PcmChannels,
		   u64PcmSampleRate*u32PcmSampleBits*u32PcmChannels, aacFileName);

	/* 先打开输入/输出文件 */
	fpPcm = fopen(pcmFileName, "rb");
	if(fpPcm == NULL)
	{
		char errMsg[256] = {0};
		sprintf(errMsg, "open file(%s) error", pcmFileName);
		perror(errMsg);
		return -1;
	}
	fpAac = fopen(aacFileName, "wb");
	if(fpAac == NULL)
	{
		char errMsg[256] = {0};
		sprintf(errMsg, "open file(%s) error", aacFileName);
		perror(errMsg);
		return -1;
	}

	/* AAC编码 1/6：传递参数进去打开编码器 */
	pFaacEncHandle = faacEncOpen(u64PcmSampleRate, u32PcmChannels, &u64PcmInSampleCnt, &u64AacOutMaxBytes);
	if(pFaacEncHandle == NULL)
	{
		printf("faacEncOpen(...) error!\n");
		goto error_exit;
	}

	/* 根据上面打开编码器传出的参数分配对应大小的缓存 */
	pu8PcmInBuf = (unsigned char*)malloc(u64PcmSampleRate*u32PcmSampleBits/8);
	pu8AacEncBuf = (unsigned char*)malloc(u64AacOutMaxBytes);

	/* AAC编码 2/6：设置编码器配置 */
	// 设置编码器配置 a/c: 先获取当前配置
	pFaacEncConf = faacEncGetCurrentConfiguration(pFaacEncHandle);
	// 设置编码器配置 b/c: 填充配置
	/*
		PCM Sample Input Format
		0	FAAC_INPUT_NULL			invalid, signifies a misconfigured config
		1	FAAC_INPUT_16BIT		native endian 16bit
		2	FAAC_INPUT_24BIT		native endian 24bit in 24 bits		(not implemented)
		3	FAAC_INPUT_32BIT		native endian 24bit in 32 bits		(DEFAULT)
		4	FAAC_INPUT_FLOAT		32bit floating point
    */
	pFaacEncConf->inputFormat = FAAC_INPUT_16BIT;
#if 0
	/* 下面参数不用设置，保存默认即可 */
	pFaacEncConf->aacObjectType = LOW; 	// MAIN:1  LOW:2  SSR:3  LTP:4
	pFaacEncConf->mpegVersion = MPEG4; 	// MPEG2:0  MPEG4:1
	pFaacEncConf->useTns = 1; 			/* Use Temporal Noise Shaping */
	pFaacEncConf->shortctl = 0; 		// SHORTCTL_NORMAL:0  SHORTCTL_NOSHORT:1  SHORTCTL_NOLONG:2
	pFaacEncConf->allowMidside = 1; 	/* Allow mid/side coding */
	pFaacEncConf->quantqual = 0; 		/* Quantizer quality */
	pFaacEncConf->outputFormat = 1; 	// 0:Raw  1:ADTS
	pFaacEncConf->bandWidth = 32000;//0 /* AAC file frequency bandwidth */
	pFaacEncConf->bitRate = 48000;//0 	/* bitrate / channel of AAC file */
#endif
	// 设置编码器配置 c/c: 重新设置编码器的配置信息
	faacEncSetConfiguration(pFaacEncHandle, pFaacEncConf);

	/* 循环操作 */
	while(1)
	{
		unsigned int u32PcmInSampleCnt = 0;
		int s32ReadPcmBytes = 0;
		int s32EncAacBytes = 0;

		/* AAC编码 3/6：从文件里读出指定大小（大小由编码器传出参数决定）PCM数据 */
		s32ReadPcmBytes = fread(pu8PcmInBuf, 1, u64PcmInSampleCnt*u32PcmSampleBits/8, fpPcm);
		if(s32ReadPcmBytes <= 0)
		{
			break;
		}
		DEBUG("Read PCM bytes: %d\n", s32ReadPcmBytes);

		// 编码传递进去的是采样数，不是字节数
		u32PcmInSampleCnt = s32ReadPcmBytes/(u32PcmSampleBits/8);
		DEBUG("Encode PCM sample count: %d\n", u32PcmInSampleCnt);

		/* AAC编码 4/6：将PCM数据（pucPcmInBuf）传进去编码得到aac数据（pucAacEncBuf）传出 */
		s32EncAacBytes = faacEncEncode(pFaacEncHandle, (int32_t*)pu8PcmInBuf, u32PcmInSampleCnt, 
														pu8AacEncBuf, (unsigned int)u64AacOutMaxBytes);
		DEBUG("Encode return aac bytes: %d\n", s32EncAacBytes);
		
		/* AAC编码 5/6：将解码得到的数据写入到AAC文件中 */
		fwrite(pu8AacEncBuf, 1, s32EncAacBytes, fpAac);
	}


	/* 记得释放内存 */
	free(pu8PcmInBuf);
	free(pu8AacEncBuf);

	/* AAC编码 6/6：用完了就关闭编码器 */
	faacEncClose(pFaacEncHandle);

error_exit:

	fclose(fpPcm);
	fclose(fpAac);

	printf("\n\033[32m%s ==> %s Success!\033[0m\n", pcmFileName, aacFileName);

	return 0;
}

