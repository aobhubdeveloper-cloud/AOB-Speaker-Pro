#include "aob/dsp.hpp"
#include <cassert>
int main(){aob::DspEngine d; aob::DspSettings s;s.output_db=6;d.configure(s);float x[4]={.1f,-.1f,.2f,-.2f};d.process(x,2,2);assert(x[0]>.1f);}