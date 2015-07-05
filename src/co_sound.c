
#include "co_sound.h"
//#include <neaacdec.h>
//#include <mad.h>
#include "co_debug.h"
#include "co_memory.h"
#include "co_fileutil.h"
#include "co_misc.h"
#include "co_param.h"
#include "co_hash16.h"


#define SOUND_PARAM_FILE PATH_DATA"/audios/audio.param"

typedef struct {
	int index;
	BOOL loop;
	float gain;
	int ch;
} SndInfo;


struct _SndObj {
	char id_str[ID_MAXLEN];
	void *file_ptr;									/* 読み込んだデータのポインタ */
	void *ptr;										/* 実データポインタ */
	int size;
	int ch;
	int rate;
}; 

static ALCdevice *device;
static ALCcontext *context;

static ALuint *g_Buffers = 0;
static int g_BufferNum;

static ALuint *g_Sources = 0;
static int g_SourceNum;
static float *source_gain = 0;

static sHASH *sndHash = 0;
static SndInfo *sndInfo = 0;
static sHASH *chHash = 0;
static int *ch_indexes = 0;

static float mGain;


static void destroyWorks(void)
{
	if(sndHash)
	{
		HashKill(sndHash);
		sndHash = 0;

		Free(sndInfo);								/* 便乗 */
		sndInfo = 0;
	}

	if(chHash)
	{
		HashKill(chHash);
		chHash = 0;
		Free(ch_indexes);
		ch_indexes = 0;
	}

	if(g_Sources)
	{
		alDeleteSources(g_SourceNum, g_Sources);
		Free(g_Sources);
		Free(source_gain);
		g_Sources = 0;
	}

	if(g_Buffers)
	{
		alDeleteBuffers(g_BufferNum, g_Buffers);
		Free(g_Buffers);
		g_Buffers = 0;
	}
}

static int setupBuffers(void)
{
	destroyWorks();
	
	sndHash = HashCreate("co_sound");

	sParam *param = ParamRead(SOUND_PARAM_FILE);
	int index;
	for(index = 0; ; index += 1)
	{
		char id_str[ID_MAXLEN];
		sprintf(id_str, "%d.name", index + 1);
		if(!ParamIsExists(param, id_str)) break;
	}

	g_Buffers = (ALuint *)sysMalloc(sizeof(ALuint *) * index, "OpenAL buffers");
	alGenBuffers(index, g_Buffers);
	g_BufferNum = index;

	sndInfo = (SndInfo *)sysMalloc(sizeof(SndInfo) * index, "OpenAL SndInfo");
	chHash = HashCreate("snd ch");
	int ch_num = 0;
	ch_indexes = (int *)sysMalloc(sizeof(int) * index, "OpenAL ch");
	for(index = 0; index < g_BufferNum; index += 1)
	{
		char id_str[ID_MAXLEN];
		sprintf(id_str, "%d.name", index + 1);
		char *name = ParamGetStr(param, id_str);
		ASSERT(name);

		sprintf(id_str, "%d.file", index + 1);
		char *fname = ParamGetStr(param, id_str);
		ASSERT(fname);

		SndObj *obj;
		char path[FNAME_MAXLEN];
#if 0
		sprintf(id_str, "%d.mp3", index + 1);
		if(ParamIsExists(param, id_str))
		{
			sprintf(path, PATH_DATA"/audios/%s.mp3", fname);
			obj = SndReadMP3(path);
		}
		else
#endif
#if 0
		sprintf(id_str, "%d.aac", index + 1);
		if(ParamIsExists(param, id_str))
		{
			sprintf(path, PATH_DATA"/audios/%s.aac", fname);
			obj = SndReadAAC(path);
		}
		else
#endif
		{
			sprintf(path, PATH_DATA"/audios/%s.wav", fname);
			obj = SndReadWAV(path);
		}
		alBufferData(g_Buffers[index], obj->ch == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16, obj->ptr, obj->size, obj->rate);
		SndDestroy(obj);

		sprintf(id_str, "%d.loop", index + 1);
		BOOL loop = ParamIsExists(param, id_str) ? ParamGetReal(param, id_str) : FALSE;
		sprintf(id_str, "%d.gain", index + 1);
		float gain = ParamIsExists(param, id_str) ? ParamGetReal(param, id_str) : 1.0f;

		sprintf(id_str, "%d.ch", index + 1);
		char *ch_name = ParamGetStr(param, id_str);
		ASSERT(ch_name);
		int *ch = (int *)HashGet(chHash, ch_name);
		if(!ch)
		{
			ch = ch_indexes + index;
			*ch = ch_num;
			HashAdd(chHash, ch_name, ch);
			ch_num += 1;
		}
		
		(sndInfo + index)->index = index;
		(sndInfo + index)->loop = loop;
		(sndInfo + index)->gain = gain;
		(sndInfo + index)->ch = *ch;
		
		HashAdd(sndHash, name, sndInfo + index);
	}
	ParamDestroy(param);

	return ch_num;
}


void SndInit(void)
{
	device = alcOpenDevice(0);
	if(device)
	{
		context = alcCreateContext(device, 0);
		alcMakeContextCurrent(context);				/* ここのエラーはスルーしても構わない */

		int buffer_num = setupBuffers();

		g_Sources = (ALuint *)sysMalloc(sizeof(ALuint *) * buffer_num, "OpenAL sources");
		alGenSources(buffer_num, g_Sources);
		source_gain = (float *)sysCalloc(buffer_num, sizeof(float), "OpenAL gain");
		g_SourceNum = buffer_num;

		mGain = 1.0f;

#if 0
		for(int i = 0; i < buffer_num; i += 1)
		{
			alSourcei(g_Sources[i], AL_BUFFER, g_Buffers[i]);
		}
#endif
	}

	SYSINFO(".... sound initialize");
}

void SndFin(void)
{
	if(device)
	{
		SndStopAll();
		destroyWorks();
	
		alcMakeContextCurrent(0);				/* TIPS:解放する為にカレントコンテキストをNULLにする */
		alcDestroyContext(context);
		alcCloseDevice(device);
	}

	SYSINFO(".... sound finish");
}

BOOL SndIsActive(void)
{
	return device != 0;
}

void SndPlay(char *id_str, float gain)
{
	if(!device) return;
	
	SndInfo *info = (SndInfo *)HashGet(sndHash, id_str);
	ASSERT(info);

	int index = info->index;
	int ch = info->ch;

	alSourceStop(g_Sources[ch]);
	alSourcei(g_Sources[ch], AL_BUFFER, g_Buffers[index]);
	alSourcef(g_Sources[ch], AL_GAIN, gain * mGain);
	alSourcePlay(g_Sources[ch]);
	source_gain[ch] = gain;
}

void SndStop(char *id_str)
{
	if(!device) return;
	
	int *ch = (int *)HashGet(chHash, id_str);
	ASSERT(ch);
	int channel = *ch;
	
	alSourceStop(g_Sources[channel]);
	alSourcei(g_Sources[channel], AL_BUFFER, 0);
}

void SndStopAll(void)
{
	if(!device) return;

	for(int i = 0; i < g_SourceNum; i += 1)
	{
		alSourceStop(g_Sources[i]);
		alSourcei(g_Sources[i], AL_BUFFER, 0);
	}
}

void SndSetMasterGain(float gain)
{
	mGain = gain;
	for(int ch = 0; ch < g_SourceNum; ch += 1)
	{
		alSourcef(g_Sources[ch], AL_GAIN, source_gain[ch] * mGain);
	}
	/* 演奏中の音量もまとめて変更 */
}

float SndGetMasterGain(void)
{
	return mGain;
}


static int getNum(u_char *ptr, int num)
{
	int i, value = 0;;

	for(i = 0; i < num; i++)
	{
		value = value + ((int)*ptr << (i * 8));
		ptr++;
	}

	return value;
}

static u_int getIntValue(u_int value)
{
#ifdef __BIG_ENDIAN__
	value = (value << 24) | ((value << 8) & 0xff0000) | ((value >> 8) & 0xff00) | (value >> 24);
#endif
	return value;
}

SndObj *SndReadWAV(char *fname)
{
	void *ptr;
	char *p;
	u_char *src;
	short *data = 0;
	char *chunk;
	int size;
	int c_size;
	int d_size = 0;
	int flag = FALSE;
	int type, ch, rate;

	ptr = MmFileLoadB(fname);
	
	p = (char *)ptr;
	if(strncmp(p, "RIFF", 4))
	{
		PRINTF("Not RIFF File.\n");
	}
	p += 4;

//	size = *(int *)p;							// データサイズ
	size = getIntValue(*(u_int *)p);
	p += sizeof(u_int);

	if(strncmp(p, "WAVE", 4))
	{
		PRINTF("Not WAVE File.\n");
	}
	size -= 4;
	p += 4;

	while(size > 0)
	{
		chunk = p;								// チャンク識別子
		p += 4;
//		c_size = *(int *)p;						// チャンクサイズ
		c_size = getIntValue(*(int *)p);
		p += sizeof(u_int);
		size -= (c_size + 4 + 4);
		src = (u_char *)p;						// データトップ
		p += c_size;							// 次のチャンクへ

		if(!strncmp(chunk, "fmt ", 4))
		{
			PRINTF("'fmt ' chunk = %d\n", c_size);

			type = getNum(src, 2);				// データタイプ
			src += 2;
			ch = getNum(src, 2);				// チャンネル数
			src += 2;
			rate = getNum(src, 4);				// サンプリングレート
			src += 4;

//			if((type == 1) && (ch == 1) && (rate == 48000))
			if(type == 1)
			{
				flag = TRUE;					// フォーマットが正しい
			}
		}
		else
		if(!strncmp(chunk, "data", 4))
		{
			// データチャンク
			//----------------
			PRINTF("'chunk ' chunk = %d\n", c_size);

			data = (short *)src;
			d_size = c_size;
		}
		else
		{
			// 不必要なチャンクはスキップ
			//----------------------------
			PRINTF("'%4s' chunk = %d\n", chunk, c_size);
		}
	}

	SndObj *obj = 0;
	if(flag && data)
	{
		char id_str[FNAME_MAXLEN];

		obj = (SndObj *)fsMalloc(sizeof(SndObj), "WAV headder");
		GetPathName(fname, 0, id_str, TRUE);
		strcpy(obj->id_str, id_str);
//		obj->ptr = appMalloc(d_size, "WAV data");
		obj->file_ptr = ptr;
		obj->ptr = data;
		ASSERT(obj->ptr);
		obj->size = d_size;
#ifdef __BIG_ENDIAN__
		u_short *src = (u_short *)data;
		u_short *dst = (u_short *)obj->ptr;
		for(int i = 0; i < d_size / 2; i += 1)
		{
			*dst = (*src << 8) | (*src >> 8);
			src += 1;
			dst += 1;
		}
#else
//		memcpy(obj->ptr, data, d_size);
#endif
		obj->ch = ch;
		obj->rate = rate;
	}

//	Free(ptr);
	
	return obj;
}


#if 0

#define AACCMAXHANNELS 8
#define AACREADSIZE	   ( FAAD_MIN_STREAMSIZE * AACCMAXHANNELS )

SndObj *SndReadAAC(char *fname)
{
	sFILE *fp;
	int lFilesize;
	int mReadSize;
	
	unsigned char cBuffer[AACREADSIZE];
	faacDecHandle hDecoder;
	faacDecConfigurationPtr	pConfig;
	unsigned long ulSamplerate;
	unsigned char ubChannels;
	long lSpendbyte;
	faacDecFrameInfo mFrameInfo;
	void *vpDecbuffer;

	hDecoder = faacDecOpen();

	fp = FsOpen(fname);
	lFilesize = FsGetSize(fp);
	mReadSize = FsRead(fp, cBuffer, AACREADSIZE);

	if((lSpendbyte = faacDecInit(hDecoder, cBuffer, mReadSize, &ulSamplerate, &ubChannels)) < 0)
	{
		PRINTF("Error faacDecInit\n");
		faacDecClose(hDecoder);
		FsClose(fp);
		return 0;
	}
//	PRINTF("This file Samplerate:%d Channnel(s):%d\n", ulSamplerate, ubChannels);

	pConfig = faacDecGetCurrentConfiguration(hDecoder);
	pConfig->defObjectType = 0;
	pConfig->outputFormat = FAAD_FMT_16BIT;
/* 	pConfig->defSampleRate = ulSamplerate; */
/* 	pConfig->useOldADTSFormat = 0; */
	faacDecSetConfiguration(hDecoder, pConfig);

#if 1
	FsSeek(fp, 0, SEEK_SET);
	int rlen = FsRead(fp, cBuffer, AACREADSIZE);

	u_char *outPut = 0;
	int outPutSize = 0;
	while (rlen >= 4)
	{
		int nlen;
		NeAACDecFrameInfo hInfo;

		short *decode = (short *)NeAACDecDecode(hDecoder, &hInfo, cBuffer, rlen);
		if((hInfo.error == 0) && (hInfo.samples > 0))
		{
			int decodeSize = hInfo.samples * sizeof(u_short);
			outPut = (u_char *)appRealloc(outPut, outPutSize + decodeSize, "AAC buffer");
			memcpy(outPut + outPutSize, decode, decodeSize);
			outPutSize += decodeSize;
		}
		memmove(cBuffer, cBuffer + hInfo.bytesconsumed, rlen - hInfo.bytesconsumed);
		rlen = rlen - hInfo.bytesconsumed;

		nlen = FsRead(fp, cBuffer + rlen, hInfo.bytesconsumed);
		rlen += nlen;
	}
	
#else
	u_char *outPut = 0;
	int outPutSize = 0;
	while(lFilesize > 0)
	{
		memmove((void *)&cBuffer[0],(void *)&cBuffer[lSpendbyte], (AACREADSIZE - lSpendbyte));
		mReadSize += FsRead(fp, cBuffer + (AACREADSIZE) - lSpendbyte, lSpendbyte);
		lFilesize -= lSpendbyte;

		lSpendbyte = 0;

		vpDecbuffer = faacDecDecode(hDecoder, &mFrameInfo, cBuffer, mReadSize);
		if(mFrameInfo.error > 0)
		{
			/*内部エラー*/
			PRINTF("mFrameInfo.error\n");
			break;
		}
		int decodeSize = mFrameInfo.samples * sizeof(u_short);
		if(decodeSize > 0)
		{
			int newSize = outPutSize + decodeSize;
			outPut = (u_char *)appRealloc(outPut, newSize, "AAC buffer");
			memcpy(outPut + outPutSize, vpDecbuffer, decodeSize);
			outPutSize += decodeSize;
		}

		/*未デコードフレーム残量を取得*/
		lSpendbyte += mFrameInfo.bytesconsumed;
		mReadSize -= mFrameInfo.bytesconsumed;
	}
#endif
	ASSERT(outPut);

	faacDecClose(hDecoder);
	FsClose(fp);
	
	SndObj *obj;
	obj = (SndObj *)appMalloc(sizeof(SndObj), "AAC headder");
	char id_str[FNAME_MAXLEN];
	GetPathName(fname, 0, id_str, TRUE);
	strcpy(obj->id_str, id_str);
	obj->ptr = outPut;
	obj->size = outPutSize;
	obj->ch = ubChannels;
	obj->rate = ulSamplerate;

//	PRINTF("AAC read fin.\n");
	
	return obj;
}
#endif

#if 0

struct buffer {
	unsigned char const *start;
	unsigned long length;
};

static enum mad_flow input(void *data, struct mad_stream *stream)
{
	struct buffer *buffer = (struct buffer *)data;
	
	if (!buffer->length) return MAD_FLOW_STOP;

	mad_stream_buffer(stream, buffer->start, buffer->length);

	buffer->length = 0;

	return MAD_FLOW_CONTINUE;
}

static inline signed int scale(mad_fixed_t sample)
{
	/* round */
	sample += (1L << (MAD_F_FRACBITS - 16));

	/* clip */
	if (sample >= MAD_F_ONE)
	{
		sample = MAD_F_ONE - 1;
	}
	else
	if(sample < -MAD_F_ONE)
	{
		sample = -MAD_F_ONE;
	}

	/* quantize */
	return sample >> (MAD_F_FRACBITS + 1 - 16);
}

static int mp3_ch;
static int mp3_rate;
static int mp3_length;
static u_char *mp3_buffer;

static enum mad_flow output(void *data, struct mad_header const *header, struct mad_pcm *pcm)
{
	unsigned int nchannels, nsamples;
	mad_fixed_t const *left_ch, *right_ch;

	/* pcm->samplerate contains the sampling frequency */

	nchannels = pcm->channels;
	nsamples  = pcm->length;
	left_ch   = pcm->samples[0];
	right_ch  = pcm->samples[1];

	mp3_ch = nchannels;
	mp3_rate = pcm->samplerate;

	int new_data = nsamples * sizeof(u_short) * nchannels;
	mp3_buffer = (u_char *)appRealloc(mp3_buffer, mp3_length + new_data, "MP3");
	u_short *buffer = (u_short *)(mp3_buffer + mp3_length);
	while (nsamples)
	{
		signed int sample;

		/* output sample(s) in 16-bit signed little-endian PCM */

		sample = scale(*left_ch);
		left_ch += 1;
		*buffer = sample;
		buffer += 1;
		if(nchannels == 2)
		{
			sample = scale(*right_ch);
			right_ch += 1;
			*buffer = sample;
			buffer += 1;
		}
		nsamples -= 1;
	}
	mp3_length += new_data;

	return MAD_FLOW_CONTINUE;
}

static enum mad_flow error(void *data, struct mad_stream *stream, struct mad_frame *frame)
{
	struct buffer *buffer = (struct buffer *)data;

	PRINTF("decoding error 0x%04x (%s) at byte offset %u\n", stream->error, mad_stream_errorstr(stream), stream->this_frame - buffer->start);

	/* return MAD_FLOW_BREAK here to stop decoding (and propagate an error) */

	return MAD_FLOW_CONTINUE;
}

static int decode(unsigned char const *start, unsigned long length)
{
	struct buffer buffer;
	struct mad_decoder decoder;
	int result;

	/* initialize our private message structure */

	buffer.start  = start;
	buffer.length = length;

	/* configure input, output, and error functions */

	mad_decoder_init(&decoder, &buffer,
					 input, 0 /* header */, 0 /* filter */, output,
					 error, 0 /* message */);

	/* start decoding */

	mp3_buffer = 0;
	mp3_length = 0;
	result = mad_decoder_run(&decoder, MAD_DECODER_MODE_SYNC);

	/* release the decoder */

	mad_decoder_finish(&decoder);

	return result;
}


SndObj *SndReadMP3(char *fname)
{
	unsigned char const *fdm;

	fdm = (unsigned char const *)MmFileLoadB(fname);
	int fsize = MmFileGetSize();
	
	decode(fdm, fsize);
	Free((void *)fdm);

	SndObj *obj;
	obj = (SndObj *)appMalloc(sizeof(SndObj), "AAC headder");
	char id_str[FNAME_MAXLEN];
	GetPathName(fname, 0, id_str, TRUE);
	strcpy(obj->id_str, id_str);

	obj->ptr = mp3_buffer;
	obj->size = mp3_length;
	obj->ch = mp3_ch;
	obj->rate = mp3_rate;

	return obj;
}

#endif

void SndDestroy(SndObj *obj)
{
	Free(obj->file_ptr);
	Free(obj);
}

char *SndObjGetName(SndObj *obj)
{
	return obj->id_str;
}
