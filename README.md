# 프로그램
- 이 프로그램은 VueSystem에서 취득한 로그에 대해서만 동작합니다.
- 취득한 데이터를 병합 또는 분할 함

# 빌드
```shell
cmake -S. -B build

cmake --build build
```

# 프로그램 실행
## VueSystem에서 취득한 로그데이터
- BinMerge -option -1 [filePath1] -2 [filePath2] 
``` shell
    - option
        - f: timestamp 상관없이 프레임 순서로 합치는 옵션
        - t: tiemstamp 고려하여 가장 가까운 프레임 끼리 합치는 옵션
        - s<frameNum>: 파일을 하나만 입력 할 수 있다 , 파일의 길이를 원하는 프레임 수로 줄이는 옵션
    
    - 지원 버전 및 파일 수 
        - f : 3개이상 파일 입력 지원 , 3개 이상 입력은 동일 버전 파일들만 입력 가능 현재 2.1버전 지원  2,0 버전 지원 예정
        - t : 2개 파일 만 입력 가능 , 현재 2.1버전 입력 가능 2.0 버전 지원 예정
        - s : 1개 파일 만 입력 가능 , 현재 2.1버전 입력 가능 2.0 버전 및 1.2 버전 지원 예정
```      
#
- BinMerge -f/t -1 [filePath1] -2 [filePath2] -3 [filePath3] -4 [filePath4] .... -9 [filePath9] 

    총 9개의 파일을 병합할 수 있습니다.  
    2개 파일 까지는 f/f -option이 작동하지만 3개 이상은 어떤 옵션을 입력해도 force mode 로 병합이 됩니다.  
    #
- example
    ```
    ./BinMerge -f -1 ~/data/path/filename1 -2 ~/data/path/filename2 ....
    ./BinMerge -t -1 ~/data/path/filename1 -2 ~/data/path/filename2 .....
    ./BinMerge -s100 ~/data/path/filename1
    ```
- output
    ```
    - {Force 또는 Match_Time 또는 Split}_hh_mm_ss_ms.bin
        mergeOption 1 = Force
        mergeOption 2 = Match_Time
        mergeOption 3 = Split
    ```
        




## Debug Log
    - correct : 파일간의 겹치는 시간이 존재하며 합칠 수 있는 상태
    - warring : time diifernce is exist  = 두 파일간에 겹치는 시간이 존재하지 않음 
                이경우 merge를 진행해도 하나의 파일은 멈춰있음
    - ERROR : 두 파일의 첫프레임 격차가 5분이상/ option에 맞지 않은 파일 갯수를 입력 했을 경우
              프로세스 종료 

#변경사항
```
    - 11.14 
        - Binformat이 2.0으로 변경되면서 해당 버전 지원 하기 위해서 함수 추가 및 분기 추가
        - BinVer2.0에 추가된 frameHeader의 timestamp를 기준으로 가까운 프레임 끼리 합쳐지는 기능 추가
    - 11.20
        - BinVersion: 정보 출력 추가
            ex) file1이 ver2 file2가 ver1일 때 BinVersion: BinVer2 + BinVer1 로그 터미널에 출력
        - 시간이 겹치는 부분만 merge 할 경우 output 파일의 size와 frame 넘버를 기존 방식대로 적용 불가능 하여 변경 하고 merge 가 끝나면 터미널에 출력 하도록    변경함 
            File absolutePath: ver2_pandarxt-16_2min_131_later_2023_09_04_20_16_00_Force.bin
            File size(MB): 892
            Total FrameNum: 176

    - 11.24
        - Binformat 2.1
            라이다별 timeStamp을 frameHead 다음에 저장하도록 변경
            2.1<->2.1(match)
            2.0<->2.1(force)
            2.1<->1.2(force)
            총 세가지 경우의 수 추가
        - forceOptio 변경
            버전이 다른 파일을 합칠경우 default 옵션으로 실행하였어도 forceOption = false 로 변경                                                                                                                                                                                         
    - 12. 18
        - multi file merge 기능 추가
            3개이상 9개 이하의 파일/ 총20개 까지의 라이다 데이터를 병합할 수 있도록 기능 추가 

    - 02.
        - OOP 방식으로 변경 중 
        - merge 
            - force 모드는 여러개 가능 
            - matching time 모드는 2개씩 가능 
        - Split은 
            - 1개만 가능
```