# 프로그램

- 이 프로그램은 VueSystem에서 취득한 로그에 대해서만 동작합니다.
- 취득한 2개의 파일을 합쳐줍니다

# 빌드

```shell
cmake -S. -B build

cmake --build build
```

# 프로그램 실행

## VueSystem에서 취득한 로그데이터

- BinMerge -option -1 [filename1] -2 [filename2]
  - option
    - -f: timestamp 상관없이 프레임 순서로 합치는 옵션
    - -d: tiemstamp 고려하여 가장 가까운 프레임 끼리 합치는 옵션
      두 파일간 격차가 있을 시 나중에 로깅된 파일이 정지되어 있는 시간이 있을 수 있음
  - filename1: 합쳐질 파일 첫 번째
  - filename2: 합쳐질 파일 두 번째
- example
  ```
  ./BinMerge -f -1 2023_08_25_14_00.bin -2 2023_09_25_14_00.bin
  ./BinMerge -d -1 2023_08_25_14_00.bin -2 2023_09_25_14_00.bin
  ```
  - output
    - 2023*08_25_14_00_2023_09_25_14_00*{\_Force 또는 \_Match_Time}.bin
      forceOption ture = \_Force
      forceOption false = \_Match_Time

## Log

    - correct : 파일간의 겹치는 시간이 존재하며 합칠 수 있는 상태
    - warring : time diifernce is exist  = 두 파일간에 겹치는 시간이 존재하지 않음
                이경우 merge를 진행해도 하나의 파일은 멈춰있음
    - ERROR : 두 파일의 첫프레임 격차가 5분이상으로 그 뒤에 겹치는 구간이 있더라도 merge하지 않고
                프로세스 종료

# 변경사항

    ## 11.14
    - Binformat이 2.0으로 변경되면서 해당 버전 지원 하기 위해서 함수 추가 및 분기 추가
    - BinVer2.0에 추가된 frameHeader의 timestamp를 기준으로 가까운 프레임 끼리 합쳐지는 기능 추가
    ## 11.20
    - BinVersion: 정보 출력 추가
        ex) file1이 ver2 file2가 ver1일 때 BinVersion: BinVer2 + BinVer1 로그 터미널에 출력
    - 시간이 겹치는 부분만 merge 할 경우 output 파일의 size와 frame 넘버를 기존 방식대로 적용 불가능 하여 변경 하고 merge 가 끝나면 터미널에 출력 하도록    변경함
        File absolutePath: ver2_pandarxt-16_2min_131_later_2023_09_04_20_16_00_Force.bin
        File size(MB): 892
        Total FrameNum: 176

    ## 11.24
    - Binformat 2.1
        라이다별 timeStamp을 frameHead 다음에 저장하도록 변경
        2.1<->2.1(match)
        2.0<->2.1(force)
        2.1<->1.2(force)
        총 세가지 경우의 수 추가
    - forceOptio 변경
        버전이 다른 파일을 합칠경우 default 옵션으로 실행하였어도 forceOption = false 로 변경

