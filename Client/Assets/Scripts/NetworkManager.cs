using System.Collections;
using System.Collections.Generic;
using System.Net.Sockets;
using System.Text;
using UnityEngine;

public class NetworkManager : MonoBehaviour
{
    private TcpClient client;
    private NetworkStream stream;
    public bool IsGameReady { get; private set; }
    public Board board;
    public P2Board p2board;
    private string receiveBuffer = "";

    private void Start()
    {
        board = FindObjectOfType<Board>();
        p2board = FindObjectOfType<P2Board>();
        ConnectToServer();
    }

    private void ConnectToServer()
    {
        try
        {
            //client = new TcpClient("211.188.49.52", 27015);   // naver
            client = new TcpClient("172.27.123.18", 27015);     // wsl
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
        if (stream != null)
        {
            string message = $"CLEAR:{linesCleared}\n";
            byte[] data = Encoding.UTF8.GetBytes(message);
            stream.Write(data, 0, data.Length);
        }
    }
    private void Update() {
        if (client != null && stream != null)
        {
            ReceiveData();
        }
    }
    
    private void ReceiveData()
    {
        if (stream.DataAvailable)
        {
            byte[] buffer = new byte[1024];
            int bytesRead = stream.Read(buffer, 0, buffer.Length);
            if (bytesRead > 0)
            {
                string receivedData = Encoding.UTF8.GetString(buffer, 0, bytesRead);
                receiveBuffer += receivedData;

                string[] messages = receiveBuffer.Split('\n');
                for (int i = 0; i < messages.Length - 1; i++)
                {
                    HandleServerMessage(messages[i].Trim());
                }
                receiveBuffer = messages[messages.Length - 1];
            }
        }
    }
    
    private void HandleServerMessage(string message)
    {
        if (string.IsNullOrEmpty(message)) return;

        if (message.StartsWith("CLEAR:"))
        {
            if (int.TryParse(message.Split(':')[1], out int linesToAdd))
            {
                board.AddLines(linesToAdd);
                Debug.Log($"Adding {linesToAdd} lines to the board.");
            }
        }
        else if (message.Contains("GAMESTART"))
        {
            Debug.Log("Game Start received from server.");
            IsGameReady = true;
        }

        else if (message.StartsWith("CHANGES:"))
        {
            string[] changes = message.Substring(8).Split(';');
            List<(int x, int y, bool hasTile)> tileChanges = new List<(int x, int y, bool hasTile)>();

            foreach (string change in changes)
            {
                if (!string.IsNullOrWhiteSpace(change))
                {
                    string[] parts = change.Split(',');
                    if (parts.Length == 3)
                    {
                        int x = int.Parse(parts[0]);
                        int y = int.Parse(parts[1]);
                        bool hasTile = parts[2] == "1";
                        tileChanges.Add((x, y, hasTile));
                    }
                }
            }
            
            // Apply the changes to the P2Board
            p2board.ApplyChanges(tileChanges);
            Debug.Log("Applied changes to P2Board.");
        }
    }


    public void SendGameOver()
    {
        if (client != null && stream != null)
        {
            string message = "GAME_OVER\n";
            byte[] data = Encoding.UTF8.GetBytes(message);
            stream.Write(data, 0, data.Length);
            Debug.Log("Sent game over message to server.");
        }
    }
    public void SendChangedTiles(List<(int x, int y, bool hasTile)> changes)
    {
        if (stream != null)
        {
            StringBuilder messageBuilder = new StringBuilder("CHANGES:");

            foreach (var change in changes)
            {
                messageBuilder.Append($"{change.x},{change.y},{(change.hasTile ? 1 : 0)};");
            }

            messageBuilder.Append("\n");

            string message = messageBuilder.ToString();
            byte[] data = Encoding.UTF8.GetBytes(message);
            stream.Write(data, 0, data.Length);
            Debug.Log($"Sent changed tiles as string: {message.Trim()}");
        }
    }


    private void OnApplicationQuit()
    {
        stream?.Close();
        client?.Close();
    }



}
