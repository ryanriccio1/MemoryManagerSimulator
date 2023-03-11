from PIL import Image


input_file = input("Input file: ")
output_file = input("Output file: ")

im = Image.open(input_file, 'r')
r, g, b, a = im.split()
w, h = im.size

with open(output_file, "w") as file:
    file.write("static unsigned char icon[] = {\n")
    for row in range(h):
        for col in range(w):
            string_r = "0x" + hex(r.getpixel((col,row)))[2:].zfill(2) + ", "
            string_g = "0x" + hex(g.getpixel((col,row)))[2:].zfill(2) + ", "
            string_b = "0x" + hex(b.getpixel((col,row)))[2:].zfill(2) + ", "
            string_a = "0x" + hex(a.getpixel((col,row)))[2:].zfill(2) + ", "

            file.write(f"\t{string_r}{string_g}{string_b}{string_a}\n")
    file.write("};\n")
