# RetroDS

## Build route now used

This project is intended to be built on GitHub Actions with the official devkitPro Docker image route.

- Do not use `devkitpro/pacman-action`.
- Do not download `apt.devkitpro.org/devkitpro-pub.gpg` in CI.
- The workflow uses `container: devkitpro/devkitarm:latest`.

After pushing to GitHub:

1. Open the repository.
2. Go to Actions.
3. Open `Build RetroDS.nds`.
4. Download the `RetroDS` artifact.
5. Extract `RetroDS.nds` and copy it to the R4 microSD.

---

# RetroDS

DS Lite 기본 메뉴의 *느낌*만 가져온 R4 / 호환 플래시카트용 홈브루 런처입니다.
공식 닌텐도 로고·아이콘·사운드·그래픽은 일체 사용하지 않으며, 흰색/연회색
배경, 얇은 회색 라인, 옅은 파란색 셀렉션이라는 색감과 듀얼 스크린 + 터치
조작감만 재해석합니다.

> **v0.1 범위** — 메뉴 UI, 십자키/터치 입력, A·B 버튼 반응까지.
> 롬 스캔, 박스아트, 에뮬레이터 실행은 v0.2 이후입니다.

> **로컬 devkitPro 설치 없이도 빌드 가능합니다.**
> GitHub에 push하면 Actions가 자동으로 RetroDS.nds를 만들어 줍니다.
> → [GitHub Actions로 빌드하기](#github-actions로-빌드하기-로컬-설치-불필요) 섹션 참고

---

## 폴더 구조

```
RetroDS/
├── Makefile             # devkitARM NDS 표준 템플릿
├── README.md
├── source/
│   ├── main.c           # 진입점 / 비디오 초기화 / 메인 루프
│   ├── menu.c, menu.h   # 8개 메뉴 항목, 카드 그리드, 상단 정보 패널
│   ├── ui.c, ui.h       # 픽셀/선/사각형/둥근 사각형/텍스트 헬퍼
│   ├── input.c, input.h # 십자키·A·B·터치 통합 입력
│   └── font.c, font.h   # 8x8 퍼블릭 도메인 ASCII 비트맵 폰트
├── include/             # (헤더 추가시 사용)
├── data/                # (바이너리 임베드시 사용)
└── build/               # make 실행 시 자동 생성
```

빌드 결과물:
- `RetroDS.elf` — 디버그용
- `RetroDS.nds` — **실제로 R4에 넣을 파일**

---

## GitHub Actions로 빌드하기 (로컬 설치 불필요)

Windows에서 devkitPro 설치 오류(MSYS2 다운로드 실패 등)가 발생하는 경우,
GitHub에 코드를 올리기만 하면 Ubuntu runner가 자동으로 빌드합니다.

### 최초 1회 설정

```sh
# 1. D:\RetroDS를 Git 저장소로 초기화
cd D:\RetroDS
git init
git add .
git commit -m "initial commit"

# 2. GitHub에 빈 저장소(Public 또는 Private) 를 만들고 연결
#    (GitHub 웹에서 New repository → 이름 입력 → Create 클릭)
git remote add origin https://github.com/<이름>/<저장소명>.git
git push -u origin main
```

### 빌드 결과물 다운로드

```
GitHub 저장소 → Actions 탭
  └── Build RetroDS.nds (방금 실행된 워크플로우 클릭)
        └── 화면 하단 Artifacts 섹션
              └── RetroDS  ← 클릭해서 ZIP 다운로드
                    └── RetroDS.nds  ← 이 파일을 R4 microSD에 복사
```

### 이후 빌드 흐름

소스를 수정하고 push할 때마다 Actions가 자동 실행됩니다.

```sh
# 소스 수정 후
git add source/menu.c
git commit -m "fix: 아이콘 위치 조정"
git push
# → Actions가 자동으로 새 RetroDS.nds를 생성
```

### 워크플로우 수동 실행

push 없이 즉시 빌드하고 싶을 때:

```
Actions 탭 → Build RetroDS.nds → Run workflow 버튼 클릭
```

### 워크플로우 설치 순서 요약

```
ubuntu-latest runner
  │
  ├─ apt install wget ca-certificates gnupg
  ├─ devkitPro GPG 키 등록 (apt trusted.gpg.d)
  ├─ devkitPro apt source list 추가
  ├─ apt update
  ├─ apt install devkitpro-pacman   ← dkp-pacman 부트스트래퍼
  ├─ dkp-pacman -S nds-dev          ← devkitARM + libnds + ndstool 전부
  ├─ DEVKITPRO / DEVKITARM / PATH 설정
  ├─ make clean (실패 무시)
  └─ make → RetroDS.nds → artifact 업로드
```

> **참고:** `nds-dev`는 apt 패키지가 아닙니다.  
> devkitPro apt 저장소에는 `devkitpro-pacman`(dkp-pacman 부트스트래퍼)만  
> apt 패키지로 존재합니다. 실제 NDS 툴체인은 `dkp-pacman -S nds-dev`로 설치합니다.

### 빌드 시간 / artifact 보존 기간

| 항목 | 값 |
|------|----|
| 첫 빌드 예상 시간 (devkitARM 다운로드 포함) | 약 3–5분 |
| 이후 빌드 예상 시간 | 약 2–3분 |
| artifact 보존 기간 | 30일 (자동 삭제) |

---

## 빌드 환경 (Windows 기준)

> **설치가 안 된다면?** devkitPro installer가 `msys could not be downloaded`
> 등의 오류로 실패하는 경우 [GitHub Actions 빌드](#github-actions로-빌드하기-로컬-설치-불필요)를 사용하세요.
> 로컬 설치 없이 동일한 RetroDS.nds를 얻을 수 있습니다.

1. **devkitPro 설치** — 공식 설치기를 사용합니다.
   - https://github.com/devkitPro/installer/releases 에서 최신
     `devkitProUpdater-x.y.z.exe` 를 받아 실행.
   - 설치 마법사에서 **NDS Development** 카테고리를 선택해
     `devkitARM`, `libnds`, `default ARM7 binaries`, `dswifi`, `maxmod`,
     `grit`, `ndstool` 까지 포함되도록 합니다.
2. 설치가 끝나면 **MSys2 (devkitPro)** 또는 일반 PowerShell 어느 쪽에서도
   빌드할 수 있습니다. 환경 변수가 자동으로 잡혀 있어야 합니다:
   - `DEVKITPRO=C:/devkitPro`
   - `DEVKITARM=C:/devkitPro/devkitARM`
   - `PATH` 에 `C:/devkitPro/devkitARM/bin`, `C:/devkitPro/tools/bin`.
3. 새 쉘을 열어 확인:
   ```sh
   echo $DEVKITARM
   arm-none-eabi-gcc --version
   ```

> devkitPro 대신 **BlocksDS** 를 쓰는 경우에도 동일한 디렉토리 구조와
> 소스가 그대로 동작합니다. 단, Makefile 의 첫 `include` 한 줄만
> BlocksDS 의 ds_rules 경로로 바꾸면 됩니다.

---

## 빌드 방법

저장소 루트(`D:\RetroDS`)에서:

```sh
make
```

`build/` 폴더와 `RetroDS.elf`, `RetroDS.nds` 가 생성됩니다.

초기화하고 다시 빌드:
```sh
make clean
make
```

---

## R4 microSD 테스트 방법

1. R4 (또는 R4i / SCDS-2 등 호환 카트)의 microSD 를 PC 에 연결합니다.
2. microSD 루트(또는 `Games/`, `Homebrew/` 같이 본인이 평소 쓰던 폴더)에
   **`RetroDS.nds`** 한 파일만 복사합니다. 별도의 `.sav` 나 데이터 폴더는
   v0.1 에서 만들지 않습니다.
3. microSD 를 R4 에 꽂고 DS Lite 에 삽입한 뒤 전원을 켜면 R4 메뉴에
   `RetroDS` 가 나타납니다. 선택하면 흰 바탕의 RetroDS 메뉴가 뜹니다.
4. 조작:
   - **십자키 ↑↓←→** : 카드 사이 이동 (4열 × 2행 그리드, 가장자리에서 wrap)
   - **A** : 선택 (v0.1 에서는 시각적 피드백만 — 6프레임 동안 눌린 상태)
   - **B** : 뒤로 (루트 메뉴이므로 v0.1 에서는 no-op)
   - **터치** : 카드를 누르면 1px 눌린 느낌, 손을 떼는 순간 활성화
     (같은 카드 위에서 떼야 적용 — 드래그하여 빠져나가면 취소)

---

## 에뮬레이터에서 먼저 돌려보기 (선택)

R4 에 굽기 전에 PC 에뮬레이터에서 확인할 수 있습니다.
- **melonDS** (권장) — 터치, 듀얼 스크린 정확도 가장 좋음.
  메뉴 `File → Open ROM…` 에서 `RetroDS.nds` 선택.
- **DeSmuME** — 빠른 확인용.

melonDS 에서는 마우스 왼쪽 버튼이 터치 입력입니다.

---

## 디자인 가드레일 (변경 시 주의)

다음은 v0.1 의 "DS Lite 느낌" 을 의도적으로 유지하기 위한 규칙입니다.
PR 단위로 흔들리면 금세 톤이 무너지기 때문에 미리 못박아둡니다.

- 배경은 **흰색 / 거의 흰색** 만 사용. 회색은 얇은 선과 보조 텍스트에만.
- 셀렉션 강조는 **연회색 배경 + 1px 옅은 파란 테두리**. 네온/채도 높은 색
  금지.
- 카드 모서리는 4px 라운드. 8px 이상 라운드는 친근하긴 하지만 DS Lite
  보다는 모바일 OS 느낌이 나서 피합니다.
- 애니메이션은 1px shift (눌린 느낌) 정도만. 슬라이드/페이드 금지.
- 텍스트는 단일 8px 폰트. 굵기·크기 변화는 색상 대비로만 표현.

---

## 라이선스 / 자산 출처

- 폰트: `source/font.c` 의 `font8x8` 는 Daniel Hepper 의 **font8x8_basic**
  퍼블릭 도메인 폰트 (오리지널 IBM PC ROM 폰트 파생) 입니다.
- 모든 아이콘 글리프는 RetroDS 가 직접 그린 추상 도형이며, 닌텐도의
  공식 아이콘/로고와의 시각적 유사성은 의도하지 않았습니다.
