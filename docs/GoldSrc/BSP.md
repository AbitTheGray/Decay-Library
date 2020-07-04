# BSP v30

BSP is file format used for maps in GoldSrc (and its original engine, Quake).
BSP means Binary Space Partitioning and describes how is the map optimized - separated into smaller segments, iterated as a tree structure.

## File Format

### Header

File starts with 32-bit number identifying its version.

```C++
int32_t Version = 30; // 0x1E
```

Then is followed by offsets for all data (lumps) in the file.

```C++
struct Lump
{
    int32_t Offset; // Relative to start of the file
    int32_t Length; // In bytes
};
```
Both `Offset` and `Length` are referred to as `int` but must not contain negative numbers.

```C++
enum class LumpType : uint8_t
{
    Entities = 0,
    Planes = 1,
    Textures = 2,
    Vertices = 3,
    Visibility = 4,
    Nodes = 5,
    TextureMapping = 6,
    Faces = 7,
    Lighting = 8,
    ClipNodes = 9,
    Leaves = 10,
    MarkSurface = 11,
    Edges = 12,
    SurfaceEdges = 13,
    Models = 14
};
static const std::size_t LumpType_Size = static_cast<uint8_t>(LumpType::Models) + 1; // 15
```

Primary lump is `Models`, others are referred from it.


### Entities

Entities lump is text-only data (followed by `'\0'`).
Closest commonly-used format is JSON (except this lump does not use `,` and is 1 record per line).

```
{
"wad" "\sierra\half-life\valve\halflife.wad;"
"skyname" "des"
"MaxRange" "8192"
"classname" "worldspawn"
"classname" "worldspawn"
}
{
"origin" "-1 -20 3"
"angles" "0 0 0"
"angle" "42"
"pitch" "-60"
"_light" "255 255 128 70"
"classname" "light_environment"
}
```

As you can see, the key may no be unique (but it will have same values, just use any of those).

Common keys are:
- `wad` = paths to `.wad` files used by the BSP file (relative to `Half-Life` directory, prefixed by `\sierra\half-life\`)
- `skyname` = name of skybox
- `MaxRange` = render distance
- `classname` = type of the entity
- `origin` = shift of the entity (position)
- `angles` = euler angles rotation
- `_light` = RGB + intensity of light
- `model` = index into [Models](#models) lump, always prefixed by `*` (`*1` = 1), index `0` is never valid

Other info (and `classname` values) can be found in `.fgd` file ([Valve Developer Community - FGD](https://developer.valvesoftware.com/wiki/FGD)) of equivalent gamemode.

Limits:
- `32` = max key length
- `1024` = max value length
- `1024` = max total entities
- `128*1024` = max total length


### Planes

```C++
enum class PlaneAxis : uint32_t
{
    // Perpendicular plane
    X = 0,
    Y = 1,
    Z = 2,

    // Non-axial plane
    AnyX = 3,
    AnyY = 4,
    AnyZ = 5
};

struct Plane
{
    glm::vec3 Normal;
    float Distance;
    PlaneAxis Axis;
};
```
Plane definitions use [Hesse normal form](https://en.wikipedia.org/wiki/Hesse_normal_form): `Normal * Point - Distance = 0`

Those data are used to speed-up rendering.

Limits:
- `65535` = maximum planes


### Textures

Unlike other lumps, this lump is prefixed by number of elements.

```C++
uint32_t TextureCount;
```

Then there are `TextureCount` offsets to texture definitions.

```C++
int32_t Offsets;
```

```C++
struct Texture
{
    char Name[16]; // Name of the texture, includes `'\0'` character 
    uint32_t Width, Height; // Texture dimensions
    Offsets[4]; // Offsets to all mip-map levels
};
```

If `Offsets[0] != 0` then all 4 offsets point to start of texture data (`Offsets[1]` have halved dimensions), relative to start of the `Texture` structure.
All data are `uint8_t` indexes into palette.
After last data, there are 2 bytes (`uint16_t PaletteLength`) followed by RGB palette.
Transparency is supported only at palette index `255` (last item of palette) if its RGB value is `0 0 255`.

Limits:
- `16` = name length
- `4` = mip-map levels
- `8` = minimum `Width` and `Height`
- `256` = maximum palette length 
- `512` = maximum texture entries


### Vertices

Array of 3D coordinates.

```
glm::vec3 Vertex;
```

Limits:
- `65535` = maximum vertices


### Visibility

//TODO


### Nodes

Array of structs.

```C++
struct Node
{
    uint32_t PlaneIndex;
    int16_t Children[2]; // Positive to Nodes, Negative to Leaves (bitwise inverted)
    
    glm::i16vec3 BBox_Min, BBox_Max; // Bounding box
    
    uint16_t FaceIndex, FaceCount;
};
```
Nodes in [BSP Tree](https://en.wikipedia.org/wiki/Binary_space_partitioning).

Limits:
- `32767` = maximum nodes


### Texture Mapping

Information how is texture mapped.
May be shared between multiple textures.

```C++
struct TextureMapping
{
    glm::vec3 S;
    float SShift;
    
    glm::vec3 T;
    float TShift;
    
    uint32_t TextureIndex;
    uint32_t Flags; // Seem to always be 0
};
```

To convert `S + SShift` and `T + TShift` into `UV` coordinates, use:
```C++
float GetTexelU(glm::vec3 position, glm::u32vec2 textureSize)
{
    return (S.x * position.x + S.y * position.y + S.z * position.z + SShift) / textureSize.x;
}
float GetTexelV(glm::vec3 position, glm::u32vec2 textureSize)
{
    return (T.x * position.x + T.y * position.y + T.z * position.z + TShift) / textureSize.y;
}
```
If you rather use texture's `0 - Width` and `0 - Height` instead of `0 - 1` range, don't use `textureSize` (or use `{1, 1}`).

Limits:
- `8192` = maximum texture mappings


### Faces

Convex polygon faces.

```C++
struct Face
{
    uint16_t PlaneIndex; // Plane the face is parallel to
    uint16_t PlaneSide; // Used as boolean, if `true`, plane's normal si multiplied by -1

    uint32_t SurfaceEdgeIndex; // Index of the first Surface Edge
    uint16_t SurfaceEdgeCount; // Number of consecutive Surface Edges

    uint16_t TextureMappingIndex; // Index of the Texture Info structure

    uint8_t LightingStyles[4];
    uint32_t LightmapOffset; // Offsets into the raw LightMap data
};
```

Limits:
- `65535` = maximum faces


### Lightmap

//TODO


### Clip Nodes

Array of structures.

```C++
struct ClipNode
{
    int32_t PlaneIndex;
    int16_t ChildNodes[2];
};
```
Secondary BSP tree used for collisions.
See `CollisionNode` in [Model](#models).

Positive `ChildNodes` point to `ClipNode`, negative to [Leaf](#leaves) (bitwise or numeric inverse?). 

Limits:
- `32767` = maximum clip nodes


### Leaves

Array of Leaf structures.

```C++
enum class LeafContent : int32_t
{
    Empty = -1,
    Solid = -2,
    Water = -3,
    Slime = -4,
    Lava = -5,
    Sky = -6,
    Origin = -7,
    Clip = -8,
    Current_0 = -9,
    Current_90 = -10,
    Current_180 = -11,
    Current_270 = -12,
    Current_Up = -13,
    Current_Down = -14,
    Translucent = -15,
};
struct Leaf
{
    LeafContent Content; // Info about the content
    int32_t VisOffset; // Offset into the visibility lump or -1 if not exist

    glm::vec3 BBox_Min, BBox_Max; // Bounding box

    uint16_t FirstMarkSurface, MarkSurfaceCount;
    uint8_t AmbientSoundLevels[4];
};
```
End nodes of BSP tree.

To get face, iterate [Mark Surface](#mark-surfaces) (`FirstMarkSurface` and `MarkSurfaceCount`).

Limits:
- `8192` = maximum leaves


### Mark Surfaces

Simple 16-bit integer array.

```C++
int16_t MarkSurface;
```

Points to [Face](#faces).

Limits:
- `65535` = maximum mark surfaces


### Edges

```C++
struct Edge
{
    uint16_t First;
    uint16_t Second;
};
```

Limits:
- `256000` = maximum edges


### Surface Edges

Simple 32-bit integer array.

```C++
int32_t SurfaceEdge;
```
Index into [Edges](#edges) lump.

Uses `First` index from `Edge` if `SurfaceEdge` value is positive, otherwise uses `Second` index of `-SurfaceEdge`
```C++
= SurfaceEdge >= 0 ? Edges[SurfaceEdge].First : Edges[-SurfaceEdge].Second;
```

Limits:
- `512000` = maximum surface edges


### Models

Array of structure.

First Model (index `0`) is main world, other are for brush entities

```C++
struct Model
{
    glm::vec3 BBox_Min, BBox_Max; // Bounding box
    glm::vec3 Origin;
    
    int32_t RenderNode; // Root node of Model's BSP tree (used for rendering)
    int32_t CollisionNode; // BSP trees used for collision
    int32_t CollisionNode2; // Not very sure about this
    int32_t DummyNode; // = 0

    int32_t VisLeafs;
    
    int32_t FaceIndex, FaceCount; // Render faces
};
```

Limits:
- `400` = maximum models
