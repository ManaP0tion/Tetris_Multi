-make 가능

s_testsdh/s_roomsdh/s_queue 중 원하는서버를 실행 후,  
s_chk_performance을 통해서 성능을 채크합니다.

s_chk_performance는 s_client를 fork, exec로 실행합니다  
s_client는 상대방 cli에게 10*20의 배열을 1초 단위로 100번 전달 후 자동으로 종료됩니다.  
  
s_chk_performance의 fork 횟수는
#define MAXFORK 100의 개수를 조절해서 조정 가능합니다.  
  
fork의 개수에 따라서 그 숫자만큼 성능이 저장된txt파일이 생성됩니다
***
만약 개별 실행을 원한다면 ./s_client 1과 같이 숫자 인수를 넣어야 됩니다.  
숫자 인수에 따라서 결과 성능이 (숫자).txt, (숫자)_go.txt로 저장됩니다.
***
p2p server는 m_server, m_client를 실행하면 됩니다
***
(숫자).txt의 첫번째 라인은 룸이 생성되는 시간.  
나머지 라인은 응답시간  
  
(숫자)_go.txt는 서버에 접속시간이 저장됩니다.
***
s_client.c, m_client.c 모두 127.0.0.1(내부망)에 접속, 필요시 적절히 변경
  
***
저희가 했던 ppt에 했던 테스트 기준으로 listen port 1000,  wait_cli 1000으로 설정했습니다.  
fork를 1000회 이상하면 오류 가능성이 있습니다
