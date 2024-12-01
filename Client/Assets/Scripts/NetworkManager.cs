using System.Collections;
using System.Net.Sockets;
using System.Text;
using UnityEngine;

public class NetworkManager : MonoBehaviour
{
    private TcpClient client;
    private NetworkStream stream;
    public bool IsGameReady { get; private set; }
    public Board board;

    public float updateInterval = 1f;

    private void Start()
    {
        if (board == null)
        {
            board = FindObjectOfType<Board>();
        }
        ConnectToServer();
    }

    private void ConnectToServer()
    {
        try
        {
            client = new TcpClient("211.188.49.52", 27015);   // naver
            //client = new TcpClient("172.27.123.18", 27015);     // wsl
            stream = client.GetStream();
            Debug.Log("Connected to server.");
        }
        catch (SocketException e)
        {
            Debug.LogError("Failed to connect to server: " + e.Message);
        }
    }

    public void SendLineClear(int linesCleared)
    {
        if (client != null && stream != null)
        {
            string message = $"CLEAR:{linesCleared}";
            byte[] data = Encoding.UTF8.GetBytes(message);
            stream.Write(data, 0, data.Length);
            Debug.Log($"Sent line clear data: {message}");
        }
    }
    private void Update() {
        ReceiveData();
        /*
        if (IsGameReady && board != null)
        {
            StartCoroutine(BoardUpdate());
        }*/
    }
    
    public void ReceiveData()
    {
        if (client != null && stream != null && stream.DataAvailable)
        {
            byte[] buffer = new byte[1024];
            int bytesRead = stream.Read(buffer, 0, buffer.Length);
            string message = Encoding.UTF8.GetString(buffer, 0, bytesRead);
            Debug.Log($"Received from server: {message}"); // 모든 메시지 로깅
            // 처리 로직
            HandleServerMessage(message);
        }
    }

    private void HandleServerMessage(string message)
    {
        if (message.StartsWith("CLEAR:"))
        {
            int linesToAdd = int.Parse(message.Split(':')[1]); // 숫자 추출
            FindObjectOfType<Board>().AddLines(linesToAdd);
            Debug.Log($"Adding {linesToAdd} lines to the board.");
        }
        if (message.Contains("GAMESTART"))
        {
            Debug.Log("Game Start received from server.");
            // 게임 시작 로직 트리거
            IsGameReady = true;
        }
    }

    public void SendGameOver()
    {
        if (client != null && stream != null)
        {
            string message = "GAME_OVER";
            byte[] data = Encoding.UTF8.GetBytes(message);
            stream.Write(data, 0, data.Length);
            Debug.Log("Sent game over message to server.");
        }
    }

    private void OnApplicationQuit()
    {
        stream?.Close();
        client?.Close();
    }

        /*
    public void SendBoardState(string boardState)
    {
        if (client != null && stream != null)
        {
            string message = "BOARD_STATE:" + boardState;
            byte[] data = Encoding.UTF8.GetBytes(message);
            stream.Write(data, 0, data.Length);
            //Debug.Log("Board state sent to server.");
        }
    }

    IEnumerator BoardUpdate(){
        while(true){
            SendBoardState(board.GetBoardStateAsString());
            yield return new WaitForSeconds(updateInterval);
        }
    }
    */

}
