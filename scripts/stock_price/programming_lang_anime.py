#!/usr/bin/python3
# coding: utf-8
"""Makes images for a programming language roulette"""

from PIL import Image, ImageDraw, ImageFont

def draw_image(bgcolor, main_x, main_y, lang_font_size, lang_x, lang_y, lang_text,
               desc_font_size, desc_x, desc_y, desc_text, out_filename):
    """Makes an image for a programming language roulette"""

    width = 384
    height = 384
    img = Image.new(mode="RGB", size=(width, height), color=bgcolor)
    draw = ImageDraw.Draw(img)

    font_name = "C:\Windows\Fonts\YuGothB.ttc"
    font_size = 72
    font = ImageFont.truetype(font=font_name, size=font_size)
    draw.text(xy=(main_x, main_y), text="大吉", font=font, fill="tomato")

    lang_font_name = "C:\Windows\Fonts\segoeui.ttf"
    lang_font = ImageFont.truetype(font=lang_font_name, size=lang_font_size)
    draw.text(xy=(lang_x, lang_y), text=lang_text, font=lang_font, fill="Black")

    desc_font_name = "C:\Windows\Fonts\YuGothR.ttc"
    desc_font = ImageFont.truetype(font=desc_font_name, size=desc_font_size)
    draw.text(xy=(desc_x, desc_y), text=desc_text, font=desc_font, fill="Black")
    img.save(out_filename)

def main():
    """Makes all images for a programming language roulette"""

    draw_image(bgcolor="ghostwhite", main_x=10, main_y=10,
               lang_font_size=160, lang_x=35, lang_y=80, lang_text="C++",
               desc_font_size=20, desc_x=30, desc_y=320,
               desc_text="システムプログラミングへようこそ",
               out_filename="images/img01.png")

    draw_image(bgcolor="floralwhite", main_x=230, main_y=30,
               lang_font_size=200, lang_x=130, lang_y=60, lang_text="R",
               desc_font_size=20, desc_x=40, desc_y=320,
               desc_text="統計学のレポートを美しく描こう",
               out_filename="images/img02.png")

    draw_image(bgcolor="azure", main_x=40, main_y=280,
               lang_font_size=144, lang_x=30, lang_y=0, lang_text="Rust",
               desc_font_size=16, desc_x=20, desc_y=200,
               desc_text="ようこそガベージコレクションの無い世界へ",
               out_filename="images/img03.png")

    draw_image(bgcolor="honeydew", main_x=220, main_y=270,
               lang_font_size=108, lang_x=30, lang_y=40, lang_text="Python",
               desc_font_size=24, desc_x=100, desc_y=210,
               desc_text="これであなたもAI人材",
               out_filename="images/img04.png")

    draw_image(bgcolor="ghostwhite", main_x=30, main_y=30,
               lang_font_size=72, lang_x=30, lang_y=140, lang_text="Assembly",
               desc_font_size=20, desc_x=30, desc_y=260,
               desc_text="SIMD命令で最適化に勝とう!",
               out_filename="images/img05.png")

    draw_image(bgcolor="floralwhite", main_x=220, main_y=20,
               lang_font_size=144, lang_x=30, lang_y=100, lang_text="Java",
               desc_font_size=24, desc_x=50, desc_y=310,
               desc_text="やっぱり一番人気でしょ",
               out_filename="images/img06.png")

    draw_image(bgcolor="azure", main_x=30, main_y=290,
               lang_font_size=144, lang_x=20, lang_y=40, lang_text="Julia",
               desc_font_size=20, desc_x=30, desc_y=220,
               desc_text="202x年のプログラミングを先取り",
               out_filename="images/img07.png")

    draw_image(bgcolor="honeydew", main_x=230, main_y=270,
               lang_font_size=72, lang_x=30, lang_y=100, lang_text="TypeScript",
               desc_font_size=20, desc_x=40, desc_y=220,
               desc_text="今年からあなたもWeb人材",
               out_filename="images/img08.png")

if __name__ == '__main__':
    main()
