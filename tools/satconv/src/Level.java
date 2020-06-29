public class Level {
    private int[][] level;

    public Level(int rows, int cols) {
        level = new int[cols][rows];
    }

    public Level(int[][] level) {
        this.level = level;
    }

    public int getNumCols() {
        return level.length;
    }

    public int getNumRows() {
        return level[0].length;
    }

    public void set(int row, int col, int val) {
        level[col][row] = val;
    }

    public int get(int row, int col) {
        return level[col][row];
    }
}
