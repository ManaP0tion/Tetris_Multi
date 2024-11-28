using System.Net.Sockets;
using System.Text;
using UnityEngine;

public class NetworkManager : MonoBehaviour
{
    private TcpClient client;
    private NetworkStream stream;

    private void Start()
    {
        ConnectToServer();
    }

    private void ConnectToServer()
    {
        try
        {
            client = new TcpClient("127.0.0.1", 27015); // 서버 IP와 포트
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

    public void ReceiveData()
    {
        if (client != null && stream != null && stream.DataAvailable)
        {
            byte[] buffer = new byte[1024];
            int bytesRead = stream.Read(buffer, 0, buffer.Length);
            string message = Encoding.UTF8.GetString(buffer, 0, bytesRead);

            // 처리 로직
            HandleServerMessage(message);
        }
    }

    private void HandleServerMessage(string message)
    {
        if (message.StartsWith("ADD:"))
        {
            int linesToAdd = int.Parse(message.Split(':')[1]);
            FindObjectOfType<Board>().AddLines(linesToAdd);
        }
    }

    private void OnApplicationQuit()
    {
        stream?.Close();
        client?.Close();
    }
}
