# 테스트 및 벤치마크 타겟 설정
add_executable(benchmark_spsc benchmark_main.cpp)

# 메인 프로젝트의 quantiprobe 라이브러리 연결
# (INTERFACE 라이브러리로 설정했으므로 헤더 경로가 자동으로 포함됨)
target_link_libraries(benchmark_spsc PRIVATE quantiprobe)

# 성능 측정을 위한 컴파일러 최적화 옵션 강제
if(MSVC)
    # MSVC: 속도 최적화(/O2), 인라인 확장(/Ob2), 빠른 부동 소수점(/fp:fast)
    target_compile_options(benchmark_spsc PRIVATE /O2 /Ob2 /fp:fast)
else()
    # GCC/Clang: 최상위 최적화(-O3), 현재 CPU 명령어 최적화(-march=native)
    # 루프 언롤링(-funroll-loops) 및 인라인 강화
    target_compile_options(benchmark_spsc PRIVATE -O3 -march=native -funroll-loops)
endif()

# (선택 사항) 나중에 Google Test 등을 추가한다면 여기에 작성
# add_executable(unit_tests test_spsc_queue.cpp)
# target_link_libraries(unit_tests PRIVATE quantiprobe GTest::gtest_main)