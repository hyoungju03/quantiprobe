cmake_minimum_required(VERSION 3.10)
project(Quantiprobe VERSION 0.1.0 LANGUAGES CXX)

# C++20 표준 설정 (최신 기능 및 atomic 활용)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 헤더 경로 설정
include_directories(include)

# 라이브러리 생성 (인터페이스 라이브러리 혹은 정적 라이브러리)
add_library(quantiprobe INTERFACE)
target_include_directories(quantiprobe INTERFACE include)

# 테스트/벤치마크 실행 파일 생성
add_executable(benchmark_spsc tests/benchmark_main.cpp)
target_link_libraries(benchmark_spsc PRIVATE quantiprobe)

# add_subdirectory(tests)

# 최적화 옵션 강제 (성능 측정을 위해 중요)
if(MSVC)
    target_compile_options(benchmark_spsc PRIVATE /O2 /Oi)
else()
    target_compile_options(benchmark_spsc PRIVATE -O3 -march=native)
endif()