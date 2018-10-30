#include "ProControl.h"
#include "LibRtmpTool.h"
#include "rtmp_push_librtmp.h"
#include "spdlog/spdlog.h"
#include "ConsumerThreadTool.h"
#include <NvApplicationProfiler.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

using namespace ArgusSamples;
namespace spd = spdlog;

std::string get_selfpath() {
    char buff[1024];
    ssize_t len = ::readlink("/proc/self/exe", buff, sizeof(buff)-1);
    if (len != -1) {
        std::string str(buff);
        return str.substr(0, str.rfind('/') + 1);
    }
}

std::string selfpath = get_selfpath() + "/log/camera_push.log";
auto logger = spd::daily_logger_mt("daily_logger", selfpath.c_str(), 0, 0);
pthread_mutex_t log_lock;

void LOG(bool flag, std::string str)
{
    pthread_mutex_lock(&log_lock);
    if (flag)
    {
        logger->info(str);
    }
    else
    {
        logger->error(str + " error");
    }
    pthread_mutex_unlock(&log_lock);
}

std::queue <MediaDataStruct> *video_buf_queue;
pthread_mutex_t video_buf_queue_lock;

std::queue <MediaDataStruct> *audio_buf_queue_faac;
pthread_mutex_t audio_buf_queue_lock_faac;

LibRtmpClass *libRtmp;
bool push_flag = true;
RtmpInfo  rtmp_info;

void *fps_function(void *ptr);

int main(int argc, char *argv[])
{
    std::string function = __FUNCTION__;
    pthread_mutex_init(&log_lock, NULL);
    spd::set_level(spd::level::trace);
    spd::set_pattern("[%l][%H:%M:%S:%f] %v");
    logger->flush_on(spd::level::trace);
    LOG(true, "Progress start");

    memset(&rtmp_info, 0, sizeof(RtmpInfo));
    rtmp_info.rtmp_info[0].rtmp_url_live = "rtmp://192.168.1.129/live/rabbit";
    rtmp_info.rtmp_info[0].rtmp_url_record = "rtmp://192.168.1.129/live/robot_record";

    libRtmp = new LibRtmpClass();
    libRtmp->InitSockets();
    libRtmp->naluUnit = new NaluUnit();
    libRtmp->naluUnit->flag = 0;
    video_buf_queue = new std::queue<MediaDataStruct>;
    LOG(NULL != video_buf_queue, function + " Init video_buf_queue");
    pthread_mutex_init(&video_buf_queue_lock, NULL);
    int ret = -1;    
    
#if VIDEO_STATUS
    pthread_t thread_push_video;
    ret = pthread_create(&thread_push_video, NULL, LibRtmpPushVideoFun, NULL);
    LOG(0 == ret, function + " create push video thread");
#endif

#if AUDIO_STATUS
    pthread_t thread_push_audio;
    ret = pthread_create(&thread_push_audio, NULL, LibRtmpPushAudioFun, NULL);
    LOG(0 == ret, function + " create push audio thread");
#endif
    pthread_t thread_video_fps;
    ret = pthread_create(&thread_video_fps, NULL, LibRtmpVideoFpsFun, NULL);
    LOG(0 == ret, function + " create video fps thread");
    
    int num = atoi(argv[1]);
    NvApplicationProfiler &profiler = NvApplicationProfiler::getProfilerInstance();
    pthread_t fps_thread;
    pthread_create(&fps_thread, NULL, fps_function, NULL);
    if (!execute(num))
        return EXIT_FAILURE;

    profiler.stop();
    profiler.printProfilerData(std::cout);

    return EXIT_SUCCESS;
}

extern int fps;
extern int capture_count;
void *fps_function(void *ptr){
    while(1){
        fps = 0;
        capture_count = 0;
        usleep(1000 * 1000);
        printf("capture count %d video output frame fps : %d fps\n", capture_count, fps);
    }
}

