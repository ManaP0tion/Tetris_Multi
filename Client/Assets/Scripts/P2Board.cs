using UnityEngine;
using UnityEngine.Tilemaps;
using System.Collections.Generic;

public class P2Board : MonoBehaviour
{
    public Tilemap tilemap; // 상대방 보드를 표시할 Tilemap
    public TileBase blockTile; // 블록이 있는 셀에 사용할 타일
    public Vector2Int boardSize = new Vector2Int(10, 20); // 보드 크기

    private bool[,] boardState; // 상대방 보드의 현재 상태

    private void Awake()
    {
        if (tilemap == null)
        {
            tilemap = GetComponentInChildren<Tilemap>();
        }

        boardState = new bool[boardSize.x, boardSize.y];
        ClearBoard();
    }

    /// <summary>
    /// 상대방 보드의 데이터를 수신하여 그린다.
    /// </summary>
    /// <param name="boardData">보드 상태를 나타내는 2차원 bool 배열 (true: 블록 있음, false: 블록 없음)</param>
    public void UpdateBoard(bool[,] boardData)
    {
        if (boardData.GetLength(0) != boardSize.x || boardData.GetLength(1) != boardSize.y)
        {
            Debug.LogError("Received board data size does not match the expected board size.");
            return;
        }

        // Clear the previous state
        ClearBoard();

        // Update the board state
        for (int y = 0; y < boardSize.y; y++)
        {
            for (int x = 0; x < boardSize.x; x++)
            {
                boardState[x, y] = boardData[x, y];

                // If there is a block, set the tile
                if (boardState[x, y])
                {
                    Vector3Int position = new Vector3Int(x - boardSize.x / 2, y - boardSize.y / 2, 0);
                    tilemap.SetTile(position, blockTile);
                }
            }
        }
    }


    public void ApplyChanges(List<(int x, int y, bool hasTile)> changes)
    {
        foreach (var change in changes)
        {
            Vector3Int position = new Vector3Int(change.x, change.y , 0);

            if (change.hasTile)
            {
                // 블록이 추가된 경우
                tilemap.SetTile(position, blockTile);
            }
            else
            {
                // 블록이 제거된 경우
                tilemap.SetTile(position, null);
            }
        }
    }

    /// <summary>
    /// 보드를 초기화하여 모든 타일을 제거한다.
    /// </summary>
    public void ClearBoard()
    {
        tilemap.ClearAllTiles();
    }
}
