-make 가능

s_testsdh
s_roomsdh
s_queue
중 원하는서버를 실행 후, 
s_chk_performance을 통해서 성능을 채크합니다.

s_chk_perfomanche의 fork 횟수는

#define MAXFORK 100의 개수를 조절해서 조정 가능합니다.
fork의 개수에 따라서 그 숫자만큼 성능이 저장된txt파일이 생성됩니다
---
만약 개별 실행을 원한다면 ./s_client 1과 같이 숫자 인수를 넣어야 됩니다
숫자 인수에 따라서 결과 성능이 (숫자).txt, (숫자)_go.txt로 저장됩니다
---
(숫자).txt의 첫번째 라인은 룸이 생성되는 시간
나머지 라인은 응답시간
(숫자)_go.txt는 서버에 접속시간이 저장됩니다.
