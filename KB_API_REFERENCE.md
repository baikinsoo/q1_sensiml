# Knowledge Pack `kb_*` API 요약 (스트리밍/세그먼트/정보)

이 문서는 프로젝트에 포함된 SensiML Knowledge Pack의 `kb_*` 계열 API 중, 헤더 주석에 나타난 함수들의 **의도/호출 타이밍/입출력 의미**를 빠르게 참고하기 위한 요약입니다.

> 용어
> - **timepoint**: “한 시각의 샘플 1프레임”. 예: (ax, ay, az) 또는 (ch0, ch1)
> - **nsensors**: timepoint를 구성하는 **채널/컬럼/축 개수**. `pSample[0..nsensors-1]`을 읽는다고 이해하면 됩니다.
> - **model_index**: 여러 모델이 있을 때 선택하는 인덱스 (보통 0 하나만 쓰는 경우가 많음)

---

## 분류 후 상태 진행 / 버퍼 관리

### `int kb_reset_model(int model_index);`
- **의도**: “분류 이벤트를 한 번 받은 뒤”, 다음 입력을 받기 위해 모델 내부 상태(피처 뱅크 인덱스 등)를 **다음 단계로 전진(advance)**.
- **호출 타이밍**: 주석 기준으로는 **분류 이벤트를 처리한 직후**(prior to passing in more data) 호출 권장.
- **반환**: 주석상 **1 = success**

### `int kb_flush_model_buffer(int model_index);`
- **의도**: 모델 내부 링버퍼에 저장된 데이터를 **비움(Flush)**.
- **용도**: 스트림이 끊겼거나, 입력 의미가 바뀌었거나, “초기화 상태로 다시 시작”하고 싶을 때.
- **반환**: 헤더 일부가 잘려 있어 상세는 주석/구현 확인 필요(관례적으로 0/1 또는 1/0 성공 여부).

---

## 모델 실행 (High-level)

### `int kb_run_model(SENSOR_DATA_T *pSample, int nsensors, int model_index);`
- **의도**: 스트리밍 입력(timepoint)을 모델 파이프라인에 넣고, **세그먼트가 잡히면 특징 생성/분류까지 수행**해서 결과를 반환.
- **입력**
  - `pSample`: **단일 timepoint**의 샘플 배열 포인터 (예: `{ax, ay, az}`)
  - `nsensors`: `pSample`의 **컬럼 수(채널 수)**  
  - `model_index`: 모델 선택
- **내부 처리 흐름(주석 요약)**
  1. 센서 트랜스폼 / 센서 필터 실행
  2. 모델 내부 링버퍼에 샘플 적재
  3. 링버퍼 대상으로 세그멘테이션 실행
  4. 세그먼트 필터 / 세그먼트 트랜스폼 실행
  5. 특징(Features) 생성
  6. 특징 트랜스폼 실행
  7. 분류(Classify) 수행 후 결과 반환
- **반환(주석 명시)**
  - `>= 0`: **분류 결과 클래스 ID** (예: `0 = Unknown`)
  - `-1`: 세그먼트가 아직 식별되지 않음(윈도우/조건 미충족)
  - `-2`: 세그먼트가 필터에 의해 제거됨

---

## 커스텀 세그먼트/청크 처리

### `void kb_add_segment(uint16_t *pBuffer, int len, int nbuffs, int model_index);`
- **의도**: 사용자 제공 버퍼로 모델 링버퍼/세그먼트 저장 공간을 **커스텀 구성**.
- **입력**
  - `pBuffer`: 링버퍼로 사용할 메모리 시작 포인터
  - `len`: 링버퍼 길이(“몇 개 샘플을 저장할지”에 해당하는 길이)
  - `nbuffs`: `len` 길이의 버퍼를 `pBuffer` 내에서 **몇 개 구성할지**
  - `model_index`: 모델 선택
- **비고**: 이 경로는 보통 “연속된 샘플 블록(청크)”을 외부에서 다루고 싶을 때 사용합니다.

### `int kb_run_segment(int model_index);`
- **의도**: `kb_add_segment()`로 준비한 링버퍼/세그먼트를 기반으로, **세그먼트 처리(세그멘테이션→특징→분류)**만 수행.
- **중요 차이점(주석 명시)**: **Sensor Transform / Sensor Filter를 건너뜀**.
- **반환(주석 명시)**
  - `>= 0`: 분류 결과 클래스 ID (0이면 Unknown)
  - `-1`: 세그먼트가 아직 식별되지 않음
  - `-2`: 세그먼트가 필터에 의해 제거됨

---

## 파이프라인 제어 (Advanced / Low-level)

아래 함수들은 “하이레벨 `kb_run_model()`이 내부에서 해주는 단계”를 **단계별로 분리**해서 직접 호출/디버깅/커스터마이징할 수 있게 해줍니다.

### `int kb_data_streaming(SENSOR_DATA_T *pSample, int nsensors, int model_index);`
- **의도**: timepoint 1개를 받아 모델 내부 링버퍼에 적재.
- **반환(주석 명시)**: `1 = added`, `0 = filtered`

### `int kb_segmentation(int model_index);`
- **의도**: 링버퍼에 쌓인 데이터를 대상으로 **세그멘테이션 수행**.
- **반환(주석 명시)**: `1 = segment found`, `-1 = filtered`

### `void kb_feature_generation_reset(int model_index);`
- **의도**: 특징 생성기(Feature Generator) 뱅크 인덱스를 0으로 초기화.

### `int kb_feature_generation(int model_index);`
- **의도**: 링버퍼/세그먼트 데이터로부터 **특징 생성**.
- **반환(주석 명시)**: `1 = features generated`, `-1 = filtered`

### `uint16_t kb_feature_transform(int model_index);`
- **의도**: 생성된 특징들을 트랜스폼하여 모델의 `feature_vector`로 정리(분류 직전 단계).
- **반환(주석 명시)**: `1 if success` (원형은 `uint16_t`이므로 구현에 따라 0/1 또는 카운트일 수 있음)

### `void kb_feature_generation_increment(int model_index);`
- **의도**: feature bank를 1 증가.

### `void kb_reset_feature_banks(int model_index);`
- **의도**: feature bank 인덱스를 0으로 설정.

### `int kb_set_feature_vector(int model_index, uint8_t *feature_vector);`
- **의도**: 모델의 입력 feature vector를 외부에서 **직접 주입**.
- **반환(주석 명시)**: 설정된 특징 개수(count)

### `int kb_recognize_feature_vector(int model_index);`
- **의도**: “분류만” 수행(특징은 이미 준비되어 있다고 가정).
- **반환**: 분류 결과(클래스 ID)

### `int kb_generate_classification(int model_index);`
- **의도**: feature transform + classification을 수행하는 결합 단계.
- **반환**: 분류 결과(클래스 ID)

---

## 모델 정보 (Model Info)

### `int kb_get_model_header(int model_index, void *model_header);`
- **의도**: 모델 헤더 정보를 외부 구조체로 가져옴(분류기 타입별로 구조체 형태가 다를 수 있음).
- **반환(주석 명시)**: `1 = successful`, `0 = not supported for this classifier`

### `const uint8_t *kb_get_model_uuid_ptr(int model_index);`
- **의도**: 모델 UUID(16바이트) 포인터 반환.
- **매크로**: `#define sml_get_model_uuid_ptr kb_get_model_uuid_ptr`

### `int kb_get_segment_length(int model_index);`
- **의도**: 현재 세그먼트 크기(샘플 수) 반환.

### `void sml_get_segment_length(int model_index, int *p_seg_len);`
- **의도**: 세그먼트 길이를 포인터로 반환하는 래퍼.

### `int kb_get_segment_start(int model_index);`
- **의도**: 현재 세그먼트의 시작 인덱스(지금까지 받은 샘플 수 관점) 반환.

### `void kb_get_segment_data(int model_index, int number_samples, int index, SENSOR_DATA_T *p_sample_data);`
- **의도**: 현재 세그먼트 데이터를 복사해 가져오기(디버깅/저장용).
- **입력**
  - `number_samples`: 가져올 샘플 수
  - `index`: 세그먼트 시작으로부터의 오프셋
  - `p_sample_data`: `number_samples * number_of_columns` 크기의 출력 버퍼(컬럼 수는 모델에 의해 결정)

### `int kb_get_feature_vector_size(int model_index);`
- **의도**: 피처 벡터 바이트 크기(또는 요소 수)를 반환.

### `void kb_get_feature_vector_v2(int model_index, uint8_t *fv_arr);`
- **의도**: 현재 계산된 피처 벡터를 배열로 복사.

### `void kb_get_feature_vector(int model_index, uint8_t *fv_arr, uint8_t *p_fv_len);` (deprecated)
- **매크로**: `#define sml_get_feature_vector kb_get_feature_vector`

### `int kb_get_classification_result_info(int model_index, void * model_results);`
- **의도**: 최신 분류 결과를 구조체 형태로 채움(모델 타입별 result 구조체 필요).
- **반환(주석 명시)**: `1 = success`, `0 = not applicable`

### `int kb_get_log_level();`
- **의도**: 디버그 로깅 레벨 조회(활성 시 1~4, 비활성 시 0).

---

## PME 모델 관련 (Pattern Matching Engine 계열)

### `int kb_flush_model(int model_index);`
- **의도**: PME 모델에 저장된 패턴 수를 0으로 설정(패턴 DB 초기화).
- **deprecated 매크로**: `#define kb_flush_model flush_model`

### `int kb_get_model_pattern(int model_index, int pattern_index, void *pattern);`
- **의도**: 패턴 DB에서 특정 패턴 정보를 가져옴.
- **반환(주석 명시)**: `1 = successful`, `0 = not supported or out of bounds`

### `int kb_add_last_pattern_to_model(int model_index, uint16_t category, uint16_t influence);`
- **의도**: “가장 최근 분류된 패턴”을 지정 카테고리/영향도(influence)로 DB에 추가.
- **반환(주석 명시)**: `1 = successful`, `0 = not supported/out of bounds`

### `int kb_add_custom_pattern_to_model(int model_index, uint8_t *feature_vector, uint16_t category, uint16_t influence);`
- **의도**: 외부 feature vector를 패턴으로 추가(동적 업데이트).
- **반환(주석 명시)**
  - `0`: 동적 업데이트 미지원
  - `1`: 업데이트 성공
  - `-1`: 더 이상 업데이트 불가

### `int kb_score_model(int model_index, uint16_t category);`
- **의도**: 입력 카테고리를 기준으로 모델을 스코어링.
- **반환(주석 명시)**: `0 = not supported`, `-1 = cannot be scored anymore`, `1 = scored`

### `int kb_retrain_model(int model_index);`
- **의도**: 스코어 기반으로 재학습(retrain).
- **반환(주석 명시)**: `0 = not supported`, `1 = retrained`

---

## Cascade / Sliding 계열

> Cascade는 “여러 피처 뱅크/자식 모델”을 함께 돌리거나, 슬라이딩 여부/리셋 정책이 다른 파이프라인 변형입니다.

### `int kb_run_model_with_cascade_features(SENSOR_DATA_T *pSample, int nsensors, int model_index);`
- **의도**: `kb_run_model`과 유사하되, cascade feature generation이 있고 **슬라이딩 방식**으로 동작.
- **반환(주석 일부)**
  - `-1`: 분류를 만들기 위해 더 많은 데이터 대기
  - `-2`: feature bank에 대해 features가 생성됨(분류 전 단계 이벤트처럼 사용될 수 있음)
  - 그 외 `>=0`: 클래스 ID
- **비고**: 주석에 “`nsensors` unused currently” 언급이 있어, 실제 사용 여부는 구현/모델 설정에 따라 달라질 수 있음.

### `int kb_segment_with_cascade_features(SENSOR_DATA_T *pSample, int nsensors, int model_index);`
- **의도**: cascade feature generation이 있고 **슬라이딩하지 않는 방식**.
- **반환 의미**: 위와 유사하게 `-1`, `-2`, `>=0` 패턴을 따름(주석 기준).

### `int kb_run_model_with_cascade_reset(SENSOR_DATA_T *pSample, int nsensors, int model_index);`
- **의도**: cascade + reset 정책. “전체 새 세트 features가 준비된 뒤” 분류하고, 이후 feature banks를 비우는 동작을 강조.
- **반환 의미**: `-1`(대기), `-2`(features 생성 이벤트), `>=0`(클래스 ID) 형태.

### `int kb_run_segment_with_cascade_reset(int model_index);`
- **의도**: `kb_run_segment`의 cascade reset 버전. feature banks가 모두 채워진 뒤에만 분류하고, 분류 후 banks를 비움.

### 프로파일링/성능 조회
- `void kb_get_feature_gen_times(int model_index, float *time_arr);`
- `void kb_get_feature_gen_cycles(int model_index, unsigned int *cycle_arr);`
- `float kb_get_classifier_time(int model_index);`
- `unsigned int kb_get_classifier_cycles(int model_index);`
- `bool kb_is_profiling_enabled(int model_index);`

---

## 실전에서 가장 자주 쓰는 패턴

### 스트리밍(가장 일반적)
- 매 측정 주기마다 `kb_run_model(sample, nsensors, model_index)` 호출
- 반환값이 `>=0`인 경우만 “분류 발생”으로 처리

### 분류가 “항상 0(Unknown)”으로만 나오는 경우 체크리스트
- `nsensors`가 학습 시 컬럼 수와 일치하는지
- 입력이 학습 때 사용한 값(`signal`/`delta`/정규화 여부)과 정합인지
- 반환이 실제로 `-1`(대기)인데 출력 로직에서 누락/오해하고 있지 않은지

