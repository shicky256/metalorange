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
import java.util.Scanner;

//saturn map format: YXPC00000CCCCCCC0AAAAAAAAAAAAAAA

public class MapReader {
    //palettes used for each tile
    private int[] palettes = new int[]{0x100, 0x100};
    private int[][] mapArr;
    private int bpp;
    private int size;

    public MapReader(String filename, int bpp, int size) {
        this.bpp = bpp;
        this.size = size;
        File mapFile = new File(filename);
        DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
        DocumentBuilder db = null;
        Document mapDocument = null;
        try {
            db = dbf.newDocumentBuilder();
            mapDocument = db.parse(mapFile);
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
        mapArr = new int[height][width];
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
                    mapArr[rowCount][i] = Integer.parseUnsignedInt(elements[i]) - 1;
                }
            }
            rowCount++;
            if (rowCount == height) {
                break;
            }
        }
//        for (int i = 0; i < height; i++) {
//            for (int j = 0; j < width; j++) {
//                System.out.print(mapArr[i][j] + " ");
//            }
//            System.out.println();
//        }
        scanner.close();
    }

    //outputs 1 word maps
    private void outputMap1(String filename) {
        try {
            Path path = Paths.get(filename);
            byte[] byteArr = new byte[mapArr[0].length * mapArr.length * 2];
            for (int i = 0; i < mapArr.length; i++) {
                for (int j = 0; j < mapArr[0].length; j++) {
                    short mapVal = 0;
                    if (bpp == 8) {
                        mapVal = (short)(((mapArr[i][j] & 0x3ff) * 2) & 0xffff);
                    }
                    else if (bpp == 4) {
                        mapVal = (short)((mapArr[i][j] & 0x3ff) & 0xffff);
                    }
                    //is tile horizontally flipped?
                    if ((mapArr[i][j] & 0x80000000) == 0x80000000) {
                        mapVal |= 0x400;
                    }
                    //is tile vertically flipped?
                    if ((mapArr[i][j] & 0x40000000) == 0x40000000) {
                        mapVal |= 0x800;
                    }
                    byteArr[(i * mapArr[0].length * 2) + (j * 2)] = (byte)(((mapVal & 0xFF00) >> 8) & 0xff);
                    byteArr[(i * mapArr[0].length * 2) + (j * 2 + 1)] = (byte)(mapVal & 0xff);
                }
            }
            Files.write(path, byteArr);
        }
        catch (Exception e) {
            e.printStackTrace();
            return;
        }
    }

    //outputs 2 word maps
    private void outputMap2(String filename) {
        try {
            Path path = Paths.get(filename);
            byte[] byteArr = new byte[mapArr[0].length * mapArr.length * 4];
            for (int i = 0; i < mapArr.length; i++) {
                for (int j = 0; j < mapArr[0].length; j++) {
                    int mapVal = (((mapArr[i][j] & 0x7fff) * 2) & 0x7fff);
                    //is tile horizontally flipped?
                    if ((mapArr[i][j] & 0x80000000) == 0x80000000) {
                        mapVal |= 0x40000000;
                    }
                    //is tile vertically flipped?
                    if ((mapArr[i][j] & 0x40000000) == 0x40000000) {
                        mapVal |= 0x80000000;
                    }
                    //reverse endianness
                    mapVal = Integer.reverseBytes(mapVal);
                    System.out.print(mapVal + " ");
                    //write to byte array
                    byteArr[(i * mapArr[0].length * 4) + (j * 4)] = (byte)(mapVal & 0xff);
                    byteArr[(i * mapArr[0].length * 4) + (j * 4 + 1)] = (byte)(((mapVal & 0xff00) >> 8) & 0xff);
                    byteArr[(i * mapArr[0].length * 4) + (j * 4 + 2)] = (byte)(((mapVal & 0xff0000) >> 16) & 0xff);
                    byteArr[(i * mapArr[0].length * 4) + (j * 4 + 3)] = (byte)(((mapVal & 0xff000000) >> 24) & 0xff);
                }
            }
            Files.write(path, byteArr);
        }
        catch (Exception e) {
            e.printStackTrace();
            return;
        }
    }

    public void outputMap(String filename) {
        if (size == 1) {
            outputMap1(filename);
        }
        else {
            outputMap2(filename);
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
        writer.println("Uint16 " + varName + "_width = " + mapArr[0].length + ";");
        writer.println("Uint16 " + varName + "_height = " + mapArr.length + ";");
        writer.println("char " + varName + "_name[] = \"" + varName.toUpperCase().substring(0, Math.min(varName.length(), 8)) + ".MAP\";");
        writer.close();
    }
}