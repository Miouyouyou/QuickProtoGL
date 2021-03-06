cd tools &&
cmake . &&
cmake --build . &&
cd .. &&

./tools/font-packer "data/Blogger Sans-Bold.otf" /usr/share/fonts/noto/NotoSans-Regular.ttf /usr/share/fonts/noto-cjk/NotoSansCJK-Regular.ttc shaders/chars.txt &&
mv font_pack_meta.dat data/ &&
mv fonts_bitmap.myyraw textures/ &&

ruby tools/ShadersPacker.rb shaders/metadata.ini &&
mv shaders/shaders.h ./ &&
mv shaders/shaders.pack data/

