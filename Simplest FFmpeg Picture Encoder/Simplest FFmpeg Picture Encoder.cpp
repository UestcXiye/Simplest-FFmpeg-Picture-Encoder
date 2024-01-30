// Simplest FFmpeg Picture Encoder.cpp : 定义控制台应用程序的入口点。
//

/**
* 最简单的基于 FFmpeg 的图像编码器
* Simplest FFmpeg Picture Encoder
*
* 源程序：
* 雷霄骅 Lei Xiaohua
* leixiaohua1020@126.com
* 中国传媒大学/数字电视技术
* Communication University of China / Digital TV Technology
* http://blog.csdn.net/leixiaohua1020
*
* 修改：
* 刘文晨 Liu Wenchen
* 812288728@qq.com
* 电子科技大学/电子信息
* University of Electronic Science and Technology of China / Electronic and Information Science
* https://blog.csdn.net/ProgramNovice
*
* 本程序实现了 YUV420P 像素数据编码为 JPEG 图片。
* 是最简单的 FFmpeg 编码方面的教程。
* 通过学习本例子可以了解 FFmpeg 的编码流程。
*
*/


#include "stdafx.h"

#include <stdio.h>
#include <stdlib.h>

// 解决报错：无法解析的外部符号 __imp__fprintf，该符号在函数 _ShowError 中被引用
#pragma comment(lib, "legacy_stdio_definitions.lib")
extern "C"
{
	// 解决报错：无法解析的外部符号 __imp____iob_func，该符号在函数 _ShowError 中被引用
	FILE __iob_func[3] = { *stdin, *stdout, *stderr };
}

#define __STDC_CONSTANT_MACROS

#ifdef _WIN32
// Windows
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
};
#else
// Linux...
#ifdef __cplusplus
extern "C"
{
#endif
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#ifdef __cplusplus
};
#endif
#endif

int main(int argc, char* argv[])
{
	AVFormatContext* pFormatCtx;
	AVOutputFormat* fmt;
	AVStream* video_stream;
	AVCodecContext* pCodecCtx;
	AVCodec* pCodec;

	uint8_t* picture_buf;
	AVFrame* picture;
	AVPacket pkt;

	int y_size;
	int got_picture = 0;
	int size;
	int ret = 0;

	FILE *fp_in = fopen("school_1382x928.yuv", "rb"); // 输入 YUV 文件
	const int in_width = 1382, in_height = 928; // YUV's width and height
	const char* out_file = "school.jpg"; // 输出图像

	av_register_all();

	// Method 1
	pFormatCtx = avformat_alloc_context();
	// Guess format
	fmt = av_guess_format("mjpeg", NULL, NULL);

	// Method 2 (More simple)
	// avformat_alloc_output_context2(&pFormatCtx, NULL, NULL, out_file);
	// fmt = pFormatCtx->oformat;

	pFormatCtx->oformat = fmt;
	// Output URL
	if (avio_open(&pFormatCtx->pb, out_file, AVIO_FLAG_READ_WRITE) < 0)
	{
		printf("Can't open output file.\n");
		return -1;
	}

	video_stream = avformat_new_stream(pFormatCtx, 0);
	if (video_stream == NULL)
	{
		printf("Can't create video stream.\n");
		return -1;
	}

	pCodecCtx = video_stream->codec;
	pCodecCtx->codec_id = fmt->video_codec;
	pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
	pCodecCtx->pix_fmt = AV_PIX_FMT_YUVJ420P;

	pCodecCtx->width = in_width;
	pCodecCtx->height = in_height;

	pCodecCtx->time_base.num = 1;
	pCodecCtx->time_base.den = 25;

	// Output some information
	av_dump_format(pFormatCtx, 0, out_file, 1);

	pCodec = avcodec_find_encoder(pCodecCtx->codec_id);
	if (!pCodec)
	{
		printf("Codec not found.\n");
		return -1;
	}
	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
	{
		printf("Can't open codec.\n");
		return -1;
	}

	picture = av_frame_alloc();
	// 计算存储具有给定参数的图像的缓存区域大小
	size = avpicture_get_size(pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height);
	// 分配缓存
	picture_buf = (uint8_t *)av_malloc(size);
	if (!picture_buf)
	{
		printf("Can't malloc picture buffer.\n");
		return -1;
	}

	// 根据指定的图像、提供的数组设置数据指针和线条大小参数
	avpicture_fill((AVPicture *)picture, picture_buf, pCodecCtx->pix_fmt,
		pCodecCtx->width, pCodecCtx->height);

	// Write Header
	avformat_write_header(pFormatCtx, NULL);

	y_size = pCodecCtx->width * pCodecCtx->height;
	av_new_packet(&pkt, 3 * y_size);

	// Read YUV
	if (fread(picture_buf, 1, y_size * 3 / 2, fp_in) <= 0)
	{
		printf("Can't read input file.\n");
		return -1;
	}
	picture->data[0] = picture_buf; // Y
	picture->data[1] = picture_buf + y_size; // U 
	picture->data[2] = picture_buf + y_size * 5 / 4; // V

	// Encode
	ret = avcodec_encode_video2(pCodecCtx, &pkt, picture, &got_picture);
	if (ret < 0)
	{
		printf("Encode Error.\n");
		return -1;
	}
	if (got_picture == 1)
	{
		pkt.stream_index = video_stream->index;
		ret = av_write_frame(pFormatCtx, &pkt);
	}

	av_free_packet(&pkt);

	// Write Trailer
	av_write_trailer(pFormatCtx);

	printf("Encode Successful.\n");

	if (video_stream)
	{
		avcodec_close(video_stream->codec);
		av_free(picture);
		av_free(picture_buf);
	}
	avio_close(pFormatCtx->pb);
	avformat_free_context(pFormatCtx);

	fclose(fp_in);

	system("pause");
	return 0;
}
