# 프로그램
- 이 프로그램은 VueSystem에서 취득한 로그에 대해서만 동작합니다.
- 취득한 데이터를 병합 또는 분할 함

# 빌드
```shell
mkdir build
cmake -S. -B build
cmake --build build
```
<br/><br/>

# 프로그램 실행
### VueSystem에서 취득한 로그데이터 필요(Binversion 2.1 권장)
빌드 후 다음 커맨드 실행 
```    
cd build
./MergeLidar
```      

<br/><br/>

# 병합 및 분할 
### force mode
- option을 force mode로 선택
- 최대 9개의 파일을 병합할 수 있습니다.
- fileNum 버튼을 통해서 file 개수를 입력하면 해당 숫자 만큼 파일 선택 창이 만들어집니다.

### TimeCompare 
 - 두파일의 timeStamp를 비교하여 10분이내로 차이가는 파일에 한하여 같은시간 저장된 프레임 끼리 병합합니다
 - Binversion 2.1에 한하여 가능 합니다. 

### Split 
 - 하나의 파일에서 원하는 구간을 분할 하는 기능입니다

### Frame Range 
- 분할 및 병합 시 해당 파일에서 원하는 프레임 구간에서 프로그램이 실행 되도록 설정 할 수 있습니다
- TimeCompare 모드로 실행할 경우 설정하지 않는 것을 권장합니다

### output
  - Force 또는 Match_Time 

    - merge_{timestamp}.bin

    ex) merged_20240226143527.bin

- split 

    - splitd_file{timestamp}.bin
    
    ex) splited_file2024_02_01_01_28_45
    

        
### Debug Log
  option 선택시 => Force, TimeCompare, Split 출력


  병합 및 분할 완료 시 => "Merge && Split is Done" 출력  
  #

<br/><br/><br/><br/>
# 사용상법 및 동작 영상
참고 링크 :  https://vueron.atlassian.net/wiki/spaces/Vueron/pages/196378826/Merge+Lidar



<br/>
<br/><br/><br/>

# 변경사항
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

    - 02.26
        - OOP 방식으로 변경
        - merge 기능 구현
            - force 모드는 여러개 가능 
            - matching time 모드는 2개씩 가능 
        - Split 기능 구형
            - 1개만 가능
        - frame Range 추가
            - 원하는 구간에 한하여 프로그램 실행 가능
        - Qt Gui 추가
        - CMake 구성 
        - ui styleSheet 변경
        - readMe 및 컨플루언스 사용 가이드 업데이트
         
```