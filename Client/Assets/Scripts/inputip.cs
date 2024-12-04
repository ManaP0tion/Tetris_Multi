using System.Collections;
using UnityEngine;
using UnityEngine.SceneManagement;
using UnityEngine.UI;

public class inputip : MonoBehaviour
{
    public InputField ipInput;

    // Update is called once per frame
    void Update()
    {
        // IP 입력 필드에 텍스트가 있고 Enter 키를 눌렀을 때
        if (!string.IsNullOrEmpty(ipInput.text) && Input.GetKeyDown(KeyCode.Return))
        {
            f_ipInput();
            SceneManager.LoadScene("Tetris"); // Tetris 씬 로드
        }
    }

    public void f_ipInput()
    {
        // GameManager의 Singleton을 통해 IP 설정
        GameManager.Instance.ip = ipInput.text;
    }
}
