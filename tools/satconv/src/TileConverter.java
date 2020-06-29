import javax.imageio.ImageIO;
import java.awt.image.BufferedImage;
import java.awt.image.IndexColorModel;
import java.io.File;
import java.io.IOException;
import java.io.PrintWriter;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;

public class TileConverter {
    private ArrayList<Byte> tileData;
    private ArrayList<Integer> palettes;
    private int width;
    private int height;

    public TileConverter(File bmpFile, int bpp) {
        tileData = new ArrayList<>();
        palettes = new ArrayList<>();
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
            width = image.getWidth();
            height = image.getHeight();
            //split image up into 16x16 tiles
            int[] imageData = new int[8 * 8];
            for (int i = 0; i < height; i += 16) {
                for (int j = 0; j < width; j += 16) {
                    //top left
                    imageData = image.getData().getPixels(j, i, 8, 8, imageData);
                    if (bpp == 8) {
                        for (int k = 0; k < imageData.length; k++) {
//                            System.out.println(imageData[k] + " " + k);
                            tileData.add((byte) (imageData[k] & 0xFF));
                        }
                    }
                    else if (bpp == 4) {
                        for (int k = 0; k < imageData.length; k += 2) {
                            byte pixels = (byte) (((imageData[k] & 0xF) << 4) | (imageData[k+1] & 0xF));
                            tileData.add(pixels);
                        }
                    }
                    //top right
                    imageData = image.getData().getPixels(j + 8, i, 8, 8, imageData);
                    if (bpp == 8) {
                        for (int k = 0; k < imageData.length; k++) {
//                            System.out.println(imageData[k] + " " + k);
                            tileData.add((byte) (imageData[k] & 0xFF));
                        }
                    }
                    else if (bpp == 4) {
                        for (int k = 0; k < imageData.length; k += 2) {
                            byte pixels = (byte) (((imageData[k] & 0xF) << 4) | (imageData[k+1] & 0xF));
                            tileData.add(pixels);
                        }
                    }
                    //bottom left
                    imageData = image.getData().getPixels(j, i + 8, 8, 8, imageData);
                    if (bpp == 8) {
                        for (int k = 0; k < imageData.length; k++) {
//                            System.out.println(imageData[k] + " " + k);
                            tileData.add((byte) (imageData[k] & 0xFF));
                        }
                    }
                    else if (bpp == 4) {
                        for (int k = 0; k < imageData.length; k += 2) {
                            byte pixels = (byte) (((imageData[k] & 0xF) << 4) | (imageData[k+1] & 0xF));
                            tileData.add(pixels);
                        }
                    }
                    //bottom right
                    imageData = image.getData().getPixels(j + 8, i + 8, 8, 8, imageData);
                    if (bpp == 8) {
                        for (int k = 0; k < imageData.length; k++) {
//                            System.out.println(imageData[k] + " " + k);
                            tileData.add((byte) (imageData[k] & 0xFF));
                        }
                    }
                    else if (bpp == 4) {
                        for (int k = 0; k < imageData.length; k += 2) {
                            byte pixels = (byte) (((imageData[k] & 0xF) << 4) | (imageData[k+1] & 0xF));
                            tileData.add(pixels);
                        }
                    }
                }
            }
            int size = colorModel.getMapSize();
            byte[] reds = new byte[size];
            byte[] greens = new byte[size];
            byte[] blues = new byte[size];
            colorModel.getReds(reds);
            colorModel.getGreens(greens);
            colorModel.getBlues(blues);
            for (int i = 0; i < (bpp == 4 ? 16 : size); i++) {
                //Saturn palette format:
                //0123456789ABCDEF0123456789ABCDEF
                //RRRRRRRRGGGGGGGGBBBBBBBB00000000
                int paletteEntry = (reds[i] & 0xff) | ((greens[i] & 0xff) << 8) | ((blues[i] & 0xff) << 16);
                System.out.print(paletteEntry + " ");
                palettes.add(paletteEntry);
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public void writeTiles(String filename) {
        if (tileData.size() == 0) {
            return;
        }
        Path path = Paths.get(filename);
        byte[] tilesArr = new byte[tileData.size()];
        for (int i = 0; i < tilesArr.length; i++) {
            tilesArr[i] = tileData.get(i);
        }
        try {
            Files.write(path, tilesArr);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public void writeInfo(String outFile) {
        try {
            PrintWriter writer = new PrintWriter(outFile, "UTF-8");
            String substring = outFile.substring(outFile.lastIndexOf('/') + 1, outFile.indexOf('.'));
            writer.println("Uint16 " + substring + "_num = " + (width / 16) * (height / 16) + ";");
            writer.println("char " + substring + "_name[] = \"" + substring.toUpperCase().substring(0, Math.min(substring.length(), 8)) + ".TLE\";");
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
