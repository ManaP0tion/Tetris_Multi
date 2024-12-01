using UnityEngine;
using UnityEngine.Tilemaps;
using UnityEngine.Networking;
using System.Collections;
using System.Text;
public class Board : MonoBehaviour
{
    public Tilemap tilemap{get; private set;}
    public Piece activePiece{ get; private set; }
    public TetrominoData[] tetrominoes;
    public Vector3Int spawnPosition;
    public Vector2Int boardSize = new Vector2Int(10, 20);
    public TileBase testTile;
    public bool EnableInput;

    private NetworkManager networkManager;

    
    public RectInt Bounds{
        get{
            Vector2Int position = new Vector2Int(-this.boardSize.x/2, -this.boardSize.y/2);
            return new RectInt(position, this.boardSize);
        }
    }

    private void Awake(){
        this.tilemap = GetComponentInChildren<Tilemap>();
        this.activePiece = GetComponentInChildren<Piece>();

        for(int i = 0; i<this.tetrominoes.Length; i++){
            this.tetrominoes[i].Initialize();
        }
    }

    private void Start(){
        networkManager = FindObjectOfType<NetworkManager>();
        SpawnPiece();
        EnableInput = false;                    // 인풋 못하게 하려고 설정한변수임
        Time.timeScale = 0;                     // 게임 일시정지
        //StartCoroutine(WaitForGameStart());     // 클라 2개 연결될때 까지 대기함
    }

    private void Update() {
        if(networkManager.IsGameReady == true){
            EnableInput = true;
            Time.timeScale = 1;
        }
    }
    
    IEnumerator WaitForGameStart()
    {
        yield return new WaitUntil(() => networkManager.IsGameReady);
        EnableInput = true;
        Time.timeScale = 1;
    }

    public void SpawnPiece(){
        int random = Random.Range(0, this.tetrominoes.Length);
        TetrominoData data = this.tetrominoes[random];

        this.activePiece.Initialize(this, this.spawnPosition, data);

        if(IsValidPosition(this.activePiece, this.spawnPosition)){
            Set(this.activePiece);
        }
        else{
            GameOver();
        } 
    }

    private void GameOver(){
        this.tilemap.ClearAllTiles();
        Debug.Log("Game over");

        // 서버에게 게임 오버 메시지 전송
        if (networkManager != null)
        {
            networkManager.SendGameOver();
        }

        // 게임을 5초 후에 재시작
        Invoke(nameof(RestartGame), 5f);
    }
    public void Set(Piece piece)
    {
        for(int i = 0; i< piece.cells.Length; i++){
            Vector3Int tilePosition = piece.cells[i] + piece.position;
            this.tilemap.SetTile(tilePosition, piece.data.tile);
        }
        
    }

    private void RestartGame()
    {
        // Clear existing pieces and reset the board
        this.tilemap.ClearAllTiles();
        Debug.Log("Restarting game...");

        // Optionally reset any other necessary state here

        // Start a new game
        SpawnPiece();
    }

    public void Clear(Piece piece)
    {
        for(int i = 0; i< piece.cells.Length; i++){
            Vector3Int tilePosition = piece.cells[i] + piece.position;
            this.tilemap.SetTile(tilePosition, null);
        }
        
    }
    
    public bool IsValidPosition(Piece piece, Vector3Int position)
    {
        RectInt bounds = this.Bounds;

        for(int i = 0; i<piece.cells.Length; i++){
            Vector3Int tilePosition = piece.cells[i] + position;
            
            if(!bounds.Contains((Vector2Int)tilePosition))
                return false;

            if(this.tilemap.HasTile(tilePosition))
                return false;

        }

        return true;
    }

    public void ClearLines(){
        RectInt bound = this.Bounds;
        int row = Bounds.yMin;
        int linesCleared = 0;

        while(row < Bounds.yMax)
        {
            if(IsLineFull(row)){
                LineClear(row);
                linesCleared++;
                //Debug.Log(linesCleared);
            }
            else{
                row++;
            }
        }
        if(linesCleared > 0){
            networkManager.SendLineClear(linesCleared);
        }
    }

    private void LineClear(int row)
    {
        RectInt bounds = this.Bounds;
        for(int col = bounds.xMin; col < bounds.xMax; col++){
            Vector3Int position = new Vector3Int(col, row, 0);
            this.tilemap.SetTile(position, null);
        }

        while(row < bounds.yMax){
            for(int col = bounds.xMin; col < bounds.xMax; col++)
            {
                Vector3Int position = new Vector3Int(col, row + 1, 0);
                TileBase above = this.tilemap.GetTile(position);

                position = new Vector3Int(col , row, 0);
                this.tilemap.SetTile(position, above);
            }
            row++;
        }
    }

    private bool IsLineFull(int row){
        RectInt bounds = this.Bounds;
        for(int col = bounds.xMin; col < bounds.xMax; col++){
            Vector3Int position = new Vector3Int(col, row, 0);

            if(!this.tilemap.HasTile(position)){
                return false;
            }
        }
        return true;
    }

    public void AddLines(int lineCount)
    {
        RectInt bounds = this.Bounds;

        // 기존 라인을 위로 이동
        for (int row = bounds.yMax - 1; row >= bounds.yMin + lineCount; row--)
        {
            for (int col = bounds.xMin; col < bounds.xMax; col++)
            {
                Vector3Int currentPosition = new Vector3Int(col, row, 0);
                Vector3Int newPosition = new Vector3Int(col, row - lineCount, 0);

                TileBase tile = this.tilemap.GetTile(newPosition); // 아래 라인의 타일 가져오기
                this.tilemap.SetTile(currentPosition, tile); // 현재 위치에 설정
            }
        }

        // 새로운 줄을 추가 (랜덤 배치)
        System.Random random = new System.Random();
        for (int row = bounds.yMin; row < bounds.yMin + lineCount; row++)
        {
            for (int col = bounds.xMin; col < bounds.xMax; col++)
            {
                Vector3Int position = new Vector3Int(col, row, 0);

                // 일정 확률로 빈 칸 또는 블록 추가
                if (random.Next(0, 10) < 7) // 70% 확률로 블록 추가
                {
                    this.tilemap.SetTile(position, testTile);
                }
                else
                {
                    this.tilemap.SetTile(position, null); // 빈 칸
                }
            }
        }

        Debug.Log($"Added {lineCount} lines to the bottom of the board.");
    }

    /*
    public string GetBoardStateAsString()
    {
        StringBuilder boardData = new StringBuilder();
        RectInt bounds = this.Bounds;
        for (int y = bounds.yMin; y < bounds.yMax; y++)
        {
            for (int x = bounds.xMin; x < bounds.xMax; x++)
            {
                Vector3Int pos = new Vector3Int(x, y, 0);
                boardData.Append(tilemap.HasTile(pos) ? "1" : "0");
            }
        }
        string boardState = boardData.ToString();
        //Debug.Log("Current board state: " + boardState);    


        return boardState;
    }
    */
}
