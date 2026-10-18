#include "fps/fps.hpp"
#include "components/fps_io.hpp"
#include <chrono>
int main(){FastInput input;FastOutput output;unsigned n=0;input.read(n);fps a;auto t0=std::chrono::steady_clock::now();fps_detail::read_coefficients(input,a,n);auto t1=std::chrono::steady_clock::now();fps r=a.log(n);auto t2=std::chrono::steady_clock::now();fps_detail::write_coefficients(output,r.data(),r.size());output.flush();auto t3=std::chrono::steady_clock::now();std::fprintf(stderr,"%.9f %.9f %.9f\n",std::chrono::duration<double,std::milli>(t1-t0).count(),std::chrono::duration<double,std::milli>(t2-t1).count(),std::chrono::duration<double,std::milli>(t3-t2).count());}
