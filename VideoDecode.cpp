// Test Video
// decodage video et affichage en openCV
// https://gist.github.com/yohhoy/f0444d3fc47f2bb2d0e2
#include <iostream>
#include <vector>
// FFmpeg
extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/pixdesc.h>
#include <libswscale/swscale.h>
}
// OpenCV
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
AVFormatContext* inctx=NULL;
AVCodec* vcodec = NULL;
AVStream* vstrm = NULL;
SwsContext* swsctx=NULL;
int vstrm_idx;
const AVPixelFormat dst_pix_fmt = AV_PIX_FMT_BGR24;

using namespace std;
using namespace cv;
//------------------------------------------------------------------------
int OpenVideo(const char *file)
{
int result;
result=avformat_open_input(&inctx,file, NULL, NULL);

result = avformat_find_stream_info(inctx, nullptr);
result = av_find_best_stream(inctx, AVMEDIA_TYPE_VIDEO, -1, -1, &vcodec, 0);
vstrm_idx = result;
vstrm = inctx->streams[vstrm_idx];
result = avcodec_open2(vstrm->codec, vcodec, nullptr);
printf("Video file:%s\n",(char*) file);
printf("Format:%s\n",inctx->iformat->name);
printf("Codec:%s  ->%s\n",vcodec->name,vcodec->long_name);
printf("Size:%d x %d \n",vstrm->codec->coded_width,vstrm->codec->height);
printf("fps:%d [fps]\n", av_q2d(vstrm->codec->framerate));
printf("pixfmt: %s\n", av_get_pix_fmt_name(vstrm->codec->pix_fmt));
printf("nombre de frames: %d\n",vstrm->nb_frames);
printf("length: %d [sec]\n", av_rescale_q(vstrm->duration, vstrm->time_base, {1,1000}) / 1000);
 
return result;
}
//-------------------------------------------------------------------------
int VideoDecode()
{
int ret;
const int dst_width = vstrm->codec->width;     // capture Wdth image
const int dst_height = vstrm->codec->height;
swsctx = sws_getCachedContext(   // Affectation context picture
        nullptr, 
        vstrm->codec->width, 
        vstrm->codec->height, 
        vstrm->codec->pix_fmt,
        dst_width, dst_height, 
        dst_pix_fmt, SWS_BICUBIC, 
        NULL, NULL, NULL);
if (!swsctx) {
        printf("fail to sws_getCachedContext\n");
        return 2;
    }
printf("output: %d x %d ,%s\n",dst_width,dst_height,av_get_pix_fmt_name(dst_pix_fmt));
AVFrame* frame = av_frame_alloc();
vector<uint8_t> framebuf(avpicture_get_size(dst_pix_fmt, dst_width, dst_height));
avpicture_fill(reinterpret_cast<AVPicture*>(frame), framebuf.data(), dst_pix_fmt, dst_width, dst_height);

    // decoding loop
    AVFrame* decframe = av_frame_alloc();
    unsigned nb_frames = 0;
    int got_pic = 0;
    AVPacket pkt;
    namedWindow("myWindow", 0);
    bool fin=false;
    while(!fin) 
        { // read packet from input file
        ret = av_read_frame(inctx, &pkt);
        if (ret < 0 && ret != AVERROR_EOF) {
            printf("fail to av_read_frame\n");
            fin=true;
            }
        else if (ret == 0 && pkt.stream_index != vstrm_idx) av_free_packet(&pkt);
        // decode video frame
        else
        {
            avcodec_decode_video2(vstrm->codec, decframe, &got_pic, &pkt);
            if (!got_pic) {
               printf("No pic\n");
               av_free_packet(&pkt);
            }
            else {    
        // convert frame to OpenCV matrix
              sws_scale(swsctx, decframe->data, decframe->linesize, 0, decframe->height, frame->data, frame->linesize);
        
              Mat image(dst_height, dst_width, CV_8UC3, framebuf.data(), frame->linesize[0]);
              imshow("myWindow", image);
              if (waitKey(30) == 0x1b) break;
        
               std::cout << nb_frames << '\r' << std::flush;  // dump progress

             }
        ++nb_frames;
        if(nb_frames>=vstrm->nb_frames) fin=true;
        av_free_packet(&pkt);
        }
    }
    std::cout << nb_frames << " frames decoded" << std::endl;
            av_init_packet(&pkt);
            pkt.data = nullptr;
            pkt.size = 0;
    av_frame_free(&decframe);
    av_frame_free(&frame);
return ret;
}
//-------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    const char* infile = "myvideo.mpeg";
    printf("Decodage video\n");
    printf("--------------\n");
    if(OpenVideo(infile)<0) printf("Erreur ouverture fichier video\n");
    else
     {
      printf("-------------------------\n");
      VideoDecode();
      avcodec_close(vstrm->codec);
      avformat_close_input(&inctx);
     }
    return 0;
}
