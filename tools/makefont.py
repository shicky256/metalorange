import sys
from PIL import Image

FONT_X = 32
FONT_Y = 32
letters = [
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'
]
    

font_img = Image.open(sys.argv[1])
font_width, font_height = font_img.size
print(font_width)
print(font_height)
letter = 0
count = 0

for y in range(0, font_height, FONT_Y):
    for x in range(0, font_width, FONT_X):
        box = (x, y, x + FONT_X, y + FONT_Y)
        character = font_img.crop(box)
        
        for cy in range(0, FONT_Y):
            for cx in range(0, FONT_X):
                col = character.getpixel((cx, cy))
                if col != (0, 0, 0):
                    col_val = ((cy + 4) & 0xfc) * 8
                    character.putpixel((cx, cy), (col_val, col_val, col_val))
        
        character.save("out\\" + str(count) + letters[letter] + ".png")
        letter += 1
        if letter >= len(letters):
            letter = 0
            count += 1