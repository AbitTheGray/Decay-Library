# Rich Map Format

Binary alternative to [MAP](MAP.md) with some added functionality.

The format itself can be found [at the official website](https://developer.valvesoftware.com/wiki/Rich_Map_Format).

## Main

```cpp
struct RMF
{
    uint8 Version = { 0xCD, 0xCC, 0x0C, 0x40 }; // float = 2.2
    char Magic[4] = "RMF";
    
    int32 VisGroupCount;
    VisGroup VisGroups[VisGroupCount];
    
    World WorldEntity;
    
    char DocInfo[8] = "DOCINFO";
    float CameraDataVersion = 0.2;
    int32 ActiveCamera = -1 /* None */;
    
    int CameraCount;
    Camera Cameras[CameraCount];
};
```

## Basic Structures

### Vector

```cpp
struct Vector
{
    float X, Y, Z;
};
```

- Fixed size of 12 bytes

### Color

```cpp
struct Color
{
    uint8_t Red, Green, Blue;
};
```

- Fixed size of 3 bytes

### Length-prefixed string
`n_string`

Null-terminated string with length specified before it.

Length is stored as 1 byte (`int8` or `uint8` ?) so maximum number of characters is 126 or 254 (because of `\0` character at the end).

```cpp
struct n_string
{
    uint8 Length;
    /// Null-terminated
    char String[Length];
};
```

### KeyValue

```cpp
struct KeyValue
{
    n_string Key;
    n_string Value;
};
```

### Camera

```cpp
struct Camera
{
    Vector EyePosition;
    Vector LookPosition;
};
```

## Objects

```cpp
struct Object {};
```

### Brush
Sometime also called `Solid`.

```cpp
struct Brush : Object
{
    n_string Type = "CMapSolid";
    
    int32 VisGroup;
    Color DisplayColor;
        
    // 4 bytes - unknown
    
    int32 FaceCount;
    Face Faces[FaceCount];
};
```

- Requires at least 4 `Faces` to form a 3D object

### Entity

```cpp
struct Entity : Object
{
    n_string Type = "CMapEntity";
    
    int32 VisGroup;
    Color DisplayColor;
        
    int32 BrushCount;
    Brush Brushes[BrushCount];
    
    n_string Classname;
    
    // 4 bytes - unknown
    
    /// `spawnflags` from FGD
    int32 EntityFlags;
    
    int32 ValueCount;
    KeyValue Values[ValueCount];
    
    // 14 bytes - unknown
    
    Vector Position;
    
    // 4 bytes - unknown
};
```

- `Entity` cannot contain other entities or groups

### Group

```cpp
struct Group : Object
{
    n_string Type = "CMapGroup";
    
    int32 VisGroup;
    Color DisplayColor;
        
    int32 ObjectCount;
    Object Objects[ObjectCount];
};
```

### World

```cpp
struct World : Object
{
    n_string Type = "CMapWorld";
    
    int32 VisGroup;
    Color DisplayColor;
    
    int32 ObjectCount;
    Object Objects[ObjectCount];
    
    /// Codename of the entity
    n_string Classname;
    
    // 4 bytes - unknown
    
    /// `spawnflags` from FGD
    int32 EntityFlags;
    
    int32 ValueCount;
    KeyValue Values[ValueCount];
    
    // 12 bytes - unknown
    
    int32 PathCount;
    Path Paths[PathCount];
};
```

- `Classname` is always `worldspawn` for World
- `EntityFlags` cannot be changed in Valve Hammer Editor
- There can be only 1 `World` in the RMF file at its designated position, having it as an `Object` has no use
- `Map properties...` in VHE

## Special Objects

### Face

Part of the [`Brush`](#brush).

```cpp
struct Face
{
    char TextureName[256];
    float ???;
    
    Vector UAxis;
    float UShift;
    
    Vector VAxis;
    float VShift;
    
    /// Degrees
    float Rotation; // Is this only for user as in MAP file?
    
    float ScaleX, ScaleY;
    
    // 16 bytes
    
    int32 VertexCount;
    /// Pre-processed face polygon
    Vector Vertices[VertexCount];
    
    Vector PlaneVertices[3];
};
```

- VHE uses first 3 `Vertices`
- There must be at least 3 items in `Vertices` to make a face

### Corner

One point of the [`Path`](#path).

```cpp
struct Corner
{
    Vector Position;
    /// Used to generate targetnames like `corner01`, `corner02`...
    int32 Index;
    /// "Name Override"
    /// Classname override?
    char Name[128];
    
    int32 ValueCount;
    KeyValue Values[ValueCount]; // Not saved correctly by VHE
};
```

### Path

```cpp
enum PathType : int32
{
    OneWay = 0,
    Circular = 1,
    PingPong = 2
};

struct Path
{
    char Name[128];
    char Classname[128];
    PathType Type;
    
    int32 CornerCount;
    Corner Corners[CornerCount];
};
```

- `Classname` should be either `path_corner` or `path_track`

### Vis Group

```cpp
struct VisGroup
{
    char Name[128];
    Color DisplayColor;
    
    // 1 byte - unknown
    
    int32 Index;
    
    uint8 Visible; // 0 = false, 1 = true
    
    // 3 bytes - unknown
};
```

- Fixed size of 140 bytes
