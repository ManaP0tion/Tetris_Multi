using System;
using System.Collections.Generic;
using TMPro;
using UnityEngine;
using UnityEngine.PlayerLoop;
using UnityEngine.UIElements;

public class Piece : MonoBehaviour
{
    public Board board{ get; private set; }
    public TetrominoData data { get; private set; }
    public Vector3Int[] cells { get; private set; }
    public Vector3Int position{ get; private set; }
    public int rotationIndex  { get; private set;}
    public float stepDelay = 1f;
    public float lockDelay = 0.5f;
    private float stepTime;
    private float lockTime;
    
    public void Initialize(Board board, Vector3Int position, TetrominoData data)
    {
        this.board = board;
        this.position = position;
        this.data = data;
        this.rotationIndex = 0;
        this.stepTime = Time.time + this.stepDelay;
        this.lockTime = 0f;

        if(this.cells == null){
            this.cells = new Vector3Int[data.cells.Length];
        }

        for(int i = 0; i< data.cells.Length; i++){
            this.cells[i] = (Vector3Int)data.cells[i];
        }
    }

    private void Update() {
        this.board.Clear(this);
        this.lockTime += Time.deltaTime;
        if(this.board.EnableInput == true){
            if(Input.GetKeyDown(KeyCode.LeftArrow )){
                Move(Vector2Int.left);
                //Debug.Log("left arrow");
            }
            else if (Input.GetKeyDown(KeyCode.RightArrow )){
                Move(Vector2Int.right);
                //Debug.Log("right arrow");
            }

            if(Input.GetKeyDown(KeyCode.DownArrow)){
                Move(Vector2Int.down);
            }

            if(Input.GetKeyDown(KeyCode.Space)){
                HardDrop();
            }

            if(Input.GetKeyDown(KeyCode.Z)){
                Rotate(-1);
            }

            if(Input.GetKeyDown(KeyCode.X)){
                Rotate(1);
            }

            if(Time.time >= this.stepTime){
                Step();
            }

            if(Input.GetKeyDown(KeyCode.T)){
                board.AddLines(1);
            }

            this.board.Set(this);
        }
    }

    private void Lock(){
        this.board.Set(this);

         // 변경된 셀 감지 및 전송
        List<(int x, int y, bool hasTile)> changes = this.board.GetChangedTiles();
        if (changes.Count > 0)
        {
            NetworkManager networkManager = FindObjectOfType<NetworkManager>();
            if (networkManager != null)
            {
                networkManager.SendChangedTiles(changes);
            }
        }

        this.board.ClearLines();
        this.board.SpawnPiece();
    }

    private void Step()
    {
        this.stepTime = Time.time + this.stepDelay;
        Move(Vector2Int.down);

        if(this.lockTime >= this.lockDelay)
            Lock();
    }

    public void HardDrop(){
        while(Move(Vector2Int.down)){
            continue;
        }
        Lock();
    }

    private bool Move(Vector2Int translation){
        Vector3Int newPos = this.position;
        newPos.x += translation.x;
        newPos.y += translation.y;

        bool valid = this.board.IsValidPosition(this, newPos);

        if(valid){
            this.position = newPos;
            this.lockTime = 0f;
        }
        return valid;
    }

private void Rotate(int direction)
    {
        // Store the current rotation in case the rotation fails
        // and we need to revert
        int originalRotation = rotationIndex;

        // Rotate all of the cells using a rotation matrix
        rotationIndex = Wrap(rotationIndex + direction, 0, 4);
        ApplyRotationMatrix(direction);

        // Revert the rotation if the wall kick tests fail
        if (!TestWallKicks(rotationIndex, direction))
        {
            rotationIndex = originalRotation;
            ApplyRotationMatrix(-direction);
        }
    }

    private void ApplyRotationMatrix(int direction)
    {
        float[] matrix = Data.RotationMatrix;

        // Rotate all of the cells using the rotation matrix
        for (int i = 0; i < cells.Length; i++)
        {
            Vector3 cell = cells[i];

            int x, y;

            switch (data.tetromino)
            {
                case Tetromino.I:
                case Tetromino.O:
                    // "I" and "O" are rotated from an offset center point
                    cell.x -= 0.5f;
                    cell.y -= 0.5f;
                    x = Mathf.CeilToInt((cell.x * matrix[0] * direction) + (cell.y * matrix[1] * direction));
                    y = Mathf.CeilToInt((cell.x * matrix[2] * direction) + (cell.y * matrix[3] * direction));
                    break;

                default:
                    x = Mathf.RoundToInt((cell.x * matrix[0] * direction) + (cell.y * matrix[1] * direction));
                    y = Mathf.RoundToInt((cell.x * matrix[2] * direction) + (cell.y * matrix[3] * direction));
                    break;
            }

            cells[i] = new Vector3Int(x, y, 0);
        }
    }

    private int Wrap(int input, int min, int max){
        if(input < min)
            return max - (min - input) % (max - min);
        
        else
            return min + (input - min ) % (max - min);
    }

    private bool TestWallKicks(int rotationIndex, int rotationDirection)
    {
        int wallKickIndex = GetWallKickIndex(rotationIndex, rotationDirection);
        for(int i = 0; i<this.data.wallKicks.GetLength(1); i++){
            Vector2Int translation = this.data.wallKicks[wallKickIndex, i];
            if(Move(translation))
                return true;
        }
        return false;
    }

    private int GetWallKickIndex(int rotationIndex, int rotationDirection){
        int wallKickIndex = rotationIndex * 2;
        if(rotationIndex < 0)
            wallKickIndex--;

            return Wrap(wallKickIndex, 0, this.data.wallKicks.GetLength(0));
    }
}