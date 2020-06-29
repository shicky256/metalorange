import javax.imageio.ImageIO;
import java.awt.image.BufferedImage;
import java.awt.image.IndexColorModel;
import java.io.*;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;

public class SpriteConverter {
    private ArrayList<Byte> imgData;
    private int numSprites;

    public SpriteConverter() {
        imgData = new ArrayList<>();
        numSprites = 0;
    }

    public void addImage(File bmpFile) {
        try {
            BufferedImage image = ImageIO.read(bmpFile);
            IndexColorModel colorModel;
            if (image.getColorModel() instanceof IndexColorModel) {
                colorModel = (IndexColorModel) image.getColorModel();
            }
            else {
                System.out.println("ERROR: Image not indexed color");
                return;
            }
            int imageWidth = image.getWidth();
            int imageHeight = image.getHeight();
            int[] imageData = new int[imageWidth * imageHeight];
            imageData = image.getData().getPixels(0, 0, imageWidth, imageHeight, imageData);
//            System.out.println(colorModel.getMapSize());
            if (colorModel.getMapSize() == 256) {
                for (int i = 0; i < imageData.length; i++) {
//                    System.out.println(imageData[i]);
                    imgData.add((byte) (imageData[i] & 0xFF));
                }
            }
            else if (colorModel.getMapSize() == 16) {
                for (int i = 0; i < imageData.length; i+=2) {
                    byte pixels = (byte) (((imageData[i] & 0xF) << 4) | (imageData[i+1] & 0xF));
                    imgData.add(pixels);
                }
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
        numSprites++;
    }

    public void writeImages(String filename) {
        if (imgData.size() == 0) {
            return;
        }
        Path path = Paths.get(filename);
        byte[] tilesArr = new byte[imgData.size()];
        for (int i = 0; i < tilesArr.length; i++) {
            tilesArr[i] = imgData.get(i);
        }
        try {
            Files.write(path, tilesArr);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
    public void writeInfo(File bmpFile, String outFile) {
        ArrayList<Integer> palettes = new ArrayList<>();
        try {
            BufferedImage image = ImageIO.read(bmpFile);
            IndexColorModel colorModel;
            if (image.getColorModel() instanceof IndexColorModel) {
                colorModel = (IndexColorModel) image.getColorModel();
            }
            else {
                System.out.println("ERROR: Image not indexed color");
                return;
            }
            //---get palette from image---
            int size = colorModel.getMapSize();
            byte[] reds = new byte[size];
            byte[] greens = new byte[size];
            byte[] blues = new byte[size];
            colorModel.getReds(reds);
            colorModel.getGreens(greens);
            colorModel.getBlues(blues);
            for (int i = 0; i < size; i++) {
                //Saturn palette format:
                //0123456789ABCDEF0123456789ABCDEF
                //RRRRRRRRGGGGGGGGBBBBBBBB00000000
                int paletteEntry = (reds[i] & 0xff) | ((greens[i] & 0xff) << 8) | ((blues[i] & 0xff) << 16);
                System.out.print(paletteEntry + " ");
                palettes.add(paletteEntry);
            }
            PrintWriter writer = new PrintWriter(outFile, "UTF-8");
            String substring = outFile.substring(outFile.lastIndexOf('/') + 1, outFile.indexOf('.'));
            writer.println("Uint16 " + substring + "_num = " + numSprites + ";");
            int imageLength;
            if (size == 256) {
                imageLength = image.getWidth() * image.getHeight();
            }
            else {
                imageLength = (image.getWidth() * image.getHeight()) / 2;
            }
            writer.println("Uint16 " + substring + "_size = " + imageLength + ";");
            writer.println("Uint16 " + substring + "_width = " + image.getWidth() + ";");
            writer.println("Uint16 " + substring + "_height = " + image.getHeight() + ";");
            writer.println("char " + substring + "_name[] = \"" + substring.toUpperCase().substring(0, Math.min(substring.length(), 8)) + ".SPR\";");
            writer.print("Uint32 " + substring + "_pal[] = {");
            for (int i = 0; i < palettes.size(); i += 8) {
    //            //if a row is entirely zeroes, assume we've reached the end of a palette definition
    //            boolean allZeroes = true;
    //            for (int j = 0; j < 8; j++) {
    //                if (palettes.get(i + j) != 0) {
    //                    allZeroes = false;
    //                    break;
    //                }
    //            }
    //            if (allZeroes) {
    //                writer.close();
    //                return;
    //            }
                writer.print("\n\t");
                for (int j = 0; j < 8; j++) {
                    writer.print(String.format("0x%04X,", palettes.get(i + j)));
                }
            }
            writer.println("\n};");
            writer.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
