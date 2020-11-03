#include <iostream>
#include <iomanip>
#include <vector>
#include <random>

const size_t segments = 1000;

template <typename T> std::vector<T> poly(T a, T b, T c, T d) {
  std::vector<T> result;
  for (size_t i = 0;i<=segments;++i) {
    T t = T(i)/T(segments);
    
    T b0 = (T(1.0)-t)*(T(1.0)-t)*(T(1.0)-t);
    T b1 = T(3.0)*t*(T(1.0)-t)*(T(1.0)-t);
    T b2 = T(3.0)*t*t*(T(1.0)-t);
    T b3 = t*t*t;

    T r = a*b0 + b*b1 + c*b2 + d*b3;
    result.push_back(r);
  }
  return result;
}

template <typename T> T lerp(T a, T b, T t) {
  return a*(T(1.0)-t) + b*t;
}

template <typename T> std::vector<T> lerp(T a, T b, T c, T d) {
  std::vector<T> result;
  for (size_t i = 0;i<=segments;++i) {
    T t = T(i)/T(segments);
    T r = lerp<T>(lerp<T>(lerp<T>(a,b,t), lerp<T>(b,c,t), t), lerp<T>(lerp<T>(b,c,t), lerp<T>(c,d,t), t), t);
    result.push_back(r);
  }
  return result;
}

int main(int argc, char ** argv) {
  const float scale = 10;
  const size_t trials = 1000;
  size_t polyWins = 0;
  double totalComp = 0;
  
  std::random_device rd{};
  std::mt19937 gen{rd()};
  std::uniform_real_distribution<float> dist{-scale, scale};

  for (size_t i = 0;i<trials;++i) {
    
    std::vector<float> params{dist(gen),dist(gen),dist(gen),dist(gen)};
    std::cout << "Params:" << params[0] << ", " << params[1] << ", " << params[2] << ", " << params[3] << " ";
    
    std::vector<float> fVecP = poly<float>(params[0],params[1],params[2],params[3]);
    std::vector<double> dVecP = poly<double>(params[0],params[1],params[2],params[3]);
    
    double v1 = 0;
    for (size_t i = 0;i<fVecP.size();++i) {
      v1 += abs(double(fVecP[i])-dVecP[i]);
    }
    std::cout << "Poly:" << (v1/double(fVecP.size())) << std::setprecision(10) << " ";

    std::vector<float> fVecL = lerp<float>(params[0],params[1],params[2],params[3]);
    std::vector<double> dVecL = lerp<double>(params[0],params[1],params[2],params[3]);
    
    double v2 = 0;
    for (size_t i = 0;i<fVecL.size();++i) {
      v2 += abs(double(fVecL[i])-dVecL[i]);
    }
    std::cout << "Lerp:" << (v2/double(fVecL.size())) << std::setprecision(10) << " ";
         
    if (v1/double(fVecP.size()) < v2/double(fVecL.size()))
         polyWins++;
    
    double comp = 100.0-100.0*((v1/double(fVecP.size())) / (v2/double(fVecL.size())));
    
    std::cout << "Error Comparison:" << comp << std::endl;
    
    totalComp += comp;
  }
      
  std::cout << "Lerp : Poly = " <<  trials-polyWins << ":" << polyWins << std::endl;
  std::cout << "totalComp = " <<  totalComp/trials << std::endl;
  
  return EXIT_SUCCESS;
}


