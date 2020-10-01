import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;

public class SatConv {
    public static void main(String[] args) {
        if (args.length == 0) {
            System.out.println("Usage: neoconv [list].txt");
            return;
        }
        System.out.println(args[0]);
        File listFile = new File(args[0]);
        BufferedReader reader;
        try {
            reader = new BufferedReader(new FileReader(listFile));
            String line = reader.readLine();
            while (line != null) {
                System.out.println(line);
                //sprite folder
                if (line.charAt(0) == 's') {
                    File folder = new File(line.substring(2));
                    File[] fileList = folder.listFiles();
                    SpriteConverter spriteConverter = new SpriteConverter();
                    for (int i = 0; i < fileList.length; i++) {
//                        System.out.println(i);
                        spriteConverter.addImage(fileList[i]);
                    }
                    spriteConverter.writeInfo(fileList[0], line.substring(2) + ".c");
                    spriteConverter.writeImages(line.substring(2, Math.min(line.length(), 10)) + ".spr");
                }
                //tiles
                if (line.charAt(0) == 't') {
                    char bppChar = line.charAt(1);
                    int bpp = Character.getNumericValue(bppChar);
                    char sizeChar = line.charAt(2);
                    int size = Character.getNumericValue(sizeChar);
                    //allow size to be 1 character
                    if (size == 6) {
                        size = 16;
                    }
                    File tiles = new File(line.substring(4));
                    TileConverter tileConverter = new TileConverter(tiles, bpp, size);
                    tileConverter.writeInfo(line.substring(4, line.indexOf('.')) + ".c");
                    tileConverter.writeTiles(line.substring(4, Math.min(line.indexOf('.'), 12)) + ".tle");
                }
                //tiled level (scrolls horizontally, so format is a bunch of columns)
                else if (line.charAt(0) == 'l') {
                    LevelReader levelReader = new LevelReader(8);
                    String levelFilename = line.substring(3, Math.min(line.indexOf('.'), 11)) + ".map";
                    String infoFilename = line.substring(3, line.indexOf('.')) + ".c";
                    File levelFile = new File(line.substring(3));
                    levelReader.addLevel(levelFile);
                    levelReader.outputLevel(levelFilename);
                    levelReader.writeInfo(infoFilename);
                }
                //tiled map (standard saturn tilemap format)
                else if (line.charAt(0) == 'm') {
                    if (line.charAt(1) == '4') {
                        MapReader mapReader = new MapReader(line.substring(3), 4);
                        mapReader.outputMap(line.substring(3, Math.min(line.indexOf('.'), 11)) + ".map");
                        mapReader.writeInfo(line.substring(3, line.indexOf('.')) + ".c");
                    }
                    else if (line.charAt(1) == '8') {
                        MapReader mapReader = new MapReader(line.substring(3), 8);
                        mapReader.outputMap(line.substring(3, Math.min(line.indexOf('.'), 11)) + ".map");
                        mapReader.writeInfo(line.substring(3, line.indexOf('.')) + ".c");
                    }
                }
                line = reader.readLine();
            }
        } catch (Exception e) { e.printStackTrace(); }
    }
}
