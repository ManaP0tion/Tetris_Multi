using UnityEngine;

public class GameManager : MonoBehaviour
{
    // Singleton 인스턴스
    public static GameManager Instance { get; private set; }

    // 공유할 IP 변수
    public string ip = "172.27.123.18";

    private void Awake()
    {
        // Singleton 유지
        if (Instance != null && Instance != this)
        {
            Destroy(gameObject); // 기존 인스턴스가 있으면 현재 오브젝트 제거
            return;
        }

        Instance = this;
        DontDestroyOnLoad(gameObject); // 씬 전환 시에도 GameManager 유지
    }
}
