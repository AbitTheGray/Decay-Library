# WAD2 + WAD3

WAD is file format used for maps in GoldSrc (and its original engine, Quake).
WAD means Where's All Data and is equivalent to ZIP files.

## File Format

### Header

File starts with 32-bit number (4 characters) identifying its version.

```C++
char Version_wad2[4] = {'W', 'A', 'D', '2'};
char Version_wad3[4] = {'W', 'A', 'D', '3'};
```
There is no difference between WAD2 and WAD3.
In [Half Life 1](https://github.com/ValveSoftware/halflife), there is switch to toggle between WAD2 and WAD3 versions in header but does nothing else.

WAD2 was used by Quake 1 while GoldSrc uses WAD3.
Those formats are compatible.


Version is followed by offset to entry information.
```C++
struct EntryHeader
{
    int32_t EntryCount;
    int32_t EntryOffset; // Offset from start of file to start of entry table
};
```

Entry itself holds all data required to find and process its content.
```C++
struct Entry
{
    int32_t DataOffset; // Offset to data (from start of the file)
    int32_t DataSize_Compressed;
    int32_t DataSize_Uncompressed;
    
    int8_t Type;
    uint8_t Compression = 0; // Not implemented
    
    int16_t Padding; // Referred to as `pad1` and `pad2`
    char Name[16]; // Entry name
};
```
Compression is not supported by official implementation.
It seems they intended to use LZSS ([Lempel–Ziv–Storer–Szymanski](https://en.wikipedia.org/wiki/Lempel%E2%80%93Ziv%E2%80%93Storer%E2%80%93Szymanski)) as compression algoright.

GoldSrc supports only 3 types: [Texture](#texture) (`0x43`), [Font](#font) (`0x46`) and [Image](#image) (`0x42`)


### Texture
Type: `0x43` / `67`

```C++
struct Texture
{
    char Name[16]; // Name of the texture, includes `'\0'` character 
    uint32_t Width, Height; // Texture dimensions
    Offsets[4]; // Offsets to all mip-map levels
};
```
This structure is same as [Textures in BSP](BSP.md#textures).
Will always contain texture data.

Be warned that `Name` matches `Name` in `Entry` not its case.


```C++
struct Texture
{
    char Name[16]; // Name of the texture, includes `'\0'` character 
    uint32_t Width, Height; // Texture dimensions
    uint32_t Offsets[4]; // Offsets to all mip-map levels
};
```

If `Offsets[0] != 0` then all 4 offsets point to start of texture data (`Offsets[1]` have halved dimensions), relative to start of the `Texture` structure.
All data are `uint8_t` indexes into palette.
After last data, there are 2 bytes (`uint16_t PaletteLength`) followed by RGB palette.
Transparency is supported only at palette index `255` (last item of palette) if its RGB value is `0 0 255`.

Followed by 1 dummy byte.
I found no reason but `Wally` will have wrong palette otherwise


### Font
Type: `0x46` / `70`

```C++
struct FontChar
{
    uint16_t Offset;
    uint16_t Width;
};

struct Font
{
    uint32_t Width = 256, Height;
    uint32_t RowCount, RowHeight;
    
    FontChar Characters[256];
    
    uint8_t Data[Width * Height];
    
    uint16_t PaletteSize = 256;
    glm::u8vec3 PaletteRGB[PaletteSize];
};
```
*This is not valid C++ structure but best description of the content.*

`FontChar.Width` can be `0` (for non-printable characters).

`Font.Width` is always `256` but the binary data does not match.

Some fonts (`fonts.wad`, not `gfx.wad`) have palette size above 256 when processed correctly.
In the binary data there are all 256 characters repeated 3 times (like `!!!"""###$$$` but starting with `0x00`).
Last color of the palette (index `255`) should be transparent (for `fonts.wad` to work).


### Image
Type: `0x42` / `66`

```C++
struct Image
{
    uint32_t Width, Height;
    uint8_t Data[Width * Height];
    
    uint16_t PaletteSize;
    glm::u8vec3 PaletteRGB[PaletteSize];
};
```
*This is not valid C++ structure but best description of the content.*
