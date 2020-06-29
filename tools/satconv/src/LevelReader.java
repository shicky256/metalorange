import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import java.io.File;
import java.io.PrintWriter;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.Scanner;

public class LevelReader {
    //palettes used for each tile
    private int[] palettes = new int[]{0x100, 0x100};
    private ArrayList<Level> levels;
    private int bpp;

    public LevelReader(int bpp) {
        this.bpp = bpp;
        levels = new ArrayList<>();
    }

    public void addLevel(File levelFile) {
        DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
        DocumentBuilder db = null;
        Document mapDocument = null;
        try {
            db = dbf.newDocumentBuilder();
            mapDocument = db.parse(levelFile);
        } catch (Exception e) {
            e.printStackTrace();
            return;
        }
        mapDocument.getDocumentElement().normalize();
        Element map = mapDocument.getDocumentElement();
        System.out.println(map.getNodeName());
        int width = Integer.parseInt(map.getAttribute("width"));
        int height = Integer.parseInt(map.getAttribute("height"));
        System.out.println("width: " + width);
        System.out.println("height: " + height);
        Element layer = (Element) map.getElementsByTagName("layer").item(0);
        Element data = (Element) layer.getElementsByTagName("data").item(0);
        String mapStr = data.getTextContent();
        Level level = new Level(height, width);
        Scanner scanner = new Scanner(mapStr);
        int rowCount = 0;
        while (scanner.hasNext()) {
            String line = scanner.nextLine();
            while (line.equals("") && scanner.hasNext()) {
                line = scanner.nextLine();
            }
            String[] elements = line.split(",");
            for (int i = 0; i < elements.length; i++) {
                if (!elements[i].equals("")) {
                    //tiled stores map elements 1 indexed
                    level.set(rowCount, i,Integer.parseUnsignedInt(elements[i]) - 1);
                }
            }
            rowCount++;
            if (rowCount == height) {
                break;
            }
        }
//        for (int i = 0; i < height; i++) {
//            for (int j = 0; j < width; j++) {
//                System.out.print(level.get(i, j) + " ");
//            }
//            System.out.println();
//        }
        scanner.close();
        levels.add(level);
    }

    public void outputLevel(String filename) {
//        System.out.println("---output---");
        try {
            Path path = Paths.get(filename);
            int fileSize = 0;
            for (int i = 0; i < levels.size(); i++) {
                Level level = levels.get(i);
                fileSize += (level.getNumRows() * level.getNumCols() * 2);
            }
            int counter = 0;
            byte[] byteArr = new byte[fileSize];
            for (int i = 0; i < levels.size(); i++) {
                Level level = levels.get(i);
//                System.out.println();
                for (int j = 0; j < level.getNumCols(); j++) {
                    for (int k = 0; k < level.getNumRows(); k++) {
                        short mapVal = 0;
                        if (bpp == 8) {
                            mapVal = (short) (((level.get(k, j) & 0x3ff) * 2) & 0xffff);
                        } else if (bpp == 4) {
                            mapVal = (short) ((level.get(k, j) & 0x3ff) & 0xffff);
                        }
                        //is tile horizontally flipped?
                        if ((level.get(k, j) & 0x80000000) == 0x80000000) {
                            mapVal |= 0x400;
                        }
                        //is tile vertically flipped?
                        if ((level.get(k, j) & 0x40000000) == 0x40000000) {
                            mapVal |= 0x800;
                        }
//                        System.out.print(mapVal + " ");
                        byteArr[counter++] = (byte) (((mapVal & 0xFF00) >> 8) & 0xff);
                        byteArr[counter++] = (byte) (mapVal & 0xff);
                    }
//                    System.out.println();
                }
                Files.write(path, byteArr);
            }
        }
        catch (Exception e) {
            e.printStackTrace();
            return;
        }
    }

    public void writeInfo(String filename) {
        PrintWriter writer = null;
        try {
            writer = new PrintWriter(filename);
        }
        catch (Exception e) {
            e.printStackTrace();
            return;
        }
        String varName = filename.substring(filename.lastIndexOf('/') + 1, filename.indexOf('.'));
        String widthArr = "Uint16 " + varName + "_widths[] = {";
        String heightArr = "Uint16 " + varName + "_heights[] = {";
        for (int i = 0; i < levels.size(); i++) {
            Level level = levels.get(i);
            widthArr += level.getNumCols() + ",";
            heightArr += level.getNumRows() + ",";
        }
        widthArr += "};";
        heightArr += "};";
        writer.println(widthArr);
        writer.println(heightArr);
        int fileSize = 0;
        String offsetArr = "Uint16 " + varName + "_offsets[] = {";
        for (int i = 0; i < levels.size(); i++) {
            Level level = levels.get(i);
            offsetArr += fileSize + ",";
            fileSize += (level.getNumRows() * level.getNumCols() * 2);
        }
        offsetArr += "};";
        writer.println("Uint16 " + varName + "_size = " + fileSize + ";");
        writer.println(offsetArr);
        writer.println("char " + varName + "_name[] = \"" + varName.toUpperCase().substring(0, Math.min(varName.length(), 8)) + ".MAP\";");
        writer.close();
    }
}