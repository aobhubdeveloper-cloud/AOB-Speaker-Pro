#include "wasapi_loopback.hpp"
#include "opuspipeline.hpp"
#include <iostream>
int main(){std::cout<<"AOB Speaker Pro Windows sender\n";aob::OpusPipeline opus;if(!opus.open())return 2;WasapiLoopback cap;if(!cap.start([&](const float*pcm,uint32_t frames,uint32_t channels){(void)pcm;(void)frames;(void)channels;}))return 3;std::cout<<"System-audio loopback active. Press Enter to stop.\n";std::cin.get();cap.stop();opus.close();return 0;}