#include <jni.h>
#include <oboe/Oboe.h>
static oboe::ManagedStream stream;
class Callback:public oboe::AudioStreamDataCallback{public:oboe::DataCallbackResult onAudioReady(oboe::AudioStream*,void*d,int32_t n)override{float*x=(float*)d;for(int i=0;i<n*2;i++)x[i]=0;return oboe::DataCallbackResult::Continue;}};
extern "C" JNIEXPORT jboolean JNICALL Java_com_aobhub_speakerpro_AudioReceiverService_nativeStart(JNIEnv*,jobject,jint){oboe::AudioStreamBuilder b;b.setDirection(oboe::Direction::Output).setPerformanceMode(oboe::PerformanceMode::LowLatency).setSharingMode(oboe::SharingMode::Exclusive).setFormat(oboe::AudioFormat::Float).setChannelCount(2);static Callback cb;b.setDataCallback(&cb);if(b.openManagedStream(stream)!=oboe::Result::OK)return JNI_FALSE;if(stream->requestStart()!=oboe::Result::OK)return JNI_FALSE;return JNI_TRUE;}
extern "C" JNIEXPORT void JNICALL Java_com_aobhub_speakerpro_AudioReceiverService_nativeStop(JNIEnv*,jobject){if(stream)stream->requestStop();stream.reset();}