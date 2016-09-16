// mythProxyProtocol.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include "mythStreamDecoder.hh"
#include "srs_librtmp.h"
srs_rtmp_t rtmp;
int InitSrsRTMP(const char* rtmpurl) {
	do {
		rtmp = srs_rtmp_create(rtmpurl);

		if (srs_rtmp_handshake(rtmp) != 0) {
			break;
		}

		if (srs_rtmp_connect_app(rtmp) != 0) {
			break;
		}
		int ret = srs_rtmp_publish_stream(rtmp);
		if (ret != 0) {
			break;
		}
		return 0;
	} while (0);

	return 1;
}

int main(int argc, char* argv[])
{
	int pts = 0, dts = 0;
	mythStreamDecoder* decoder = mythStreamDecoder::CreateNew("120.204.70.218", 1017);
	decoder->start();
	InitSrsRTMP("rtmp://localhost/live/stream");
	int time = SDL_GetTicks();
	for (;;){
		PacketQueue* pkt = decoder->get();
		if (pkt){
			int time2 = SDL_GetTicks();
			pts = dts += (time2 - time);
			printf("Push length:%6d£¬timespan=%6dms\n", pkt->h264PacketLength,(time2 - time));
			time = time2;
			int ret = srs_h264_write_raw_frames(rtmp, (char*) pkt->h264Packet, pkt->h264PacketLength, dts, pts);
			if (ret != 0) {
				if (srs_h264_is_dvbsp_error(ret)) {
					printf("ignore drop video error, code=%d\n", ret);
				}
				else if (srs_h264_is_duplicated_sps_error(ret)) {
					printf("ignore duplicated sps, code=%d\n", ret);
				}
				else if (srs_h264_is_duplicated_pps_error(ret)) {
					printf("ignore duplicated pps, code=%d\n", ret);
				}
				else {
					printf("send h264 raw data failed. code=%d\n", ret);
					break;
				}
			}
		}
		SDL_Delay(1);
	}
	return 0;
}

